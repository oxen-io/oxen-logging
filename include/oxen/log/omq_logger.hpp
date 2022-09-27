#pragma once

#include "ring_buffer_sink.hpp"

#include <spdlog/spdlog.h>
#include <oxenmq/pubsub.h>
#include <oxenmq/oxenmq.h>

namespace oxen::log {

using namespace std::literals;


/**
 * A class for sending logs via RPC subscription
 *
 * Construct with a RingBufferSink which is registered with oxen::logging
 * and a reference to the OxenMQ object used for RPC.
 *
 * *** The OxenMQ reference must remain valid for the lifetime of this class! ***
 */
class PubsubLogger
{
    oxenmq::OxenMQ& omq;
    const std::shared_ptr<RingBufferSink> buffer;
    oxenmq::Subscription<std::string> subs;

public:
    PubsubLogger() = delete;
    PubsubLogger(oxenmq::OxenMQ& _omq,
            std::shared_ptr<RingBufferSink> _buffer,
            std::chrono::milliseconds sub_duration = 30min)
        : omq{_omq},
        buffer{std::move(_buffer)},
        subs{"omq rpc logger"s, sub_duration}
    {
        if (!buffer)
            throw std::runtime_error{"PubsubLogger must be supplied a RingBufferSink"};
        buffer->set_log_callback([this](const auto& message) {
                subs.publish([this,message](const auto& conn, const auto& endpoint) {
                        omq.send(conn, endpoint, message);
                });
        });
    }

    ~PubsubLogger() {
        buffer->set_log_callback(nullptr);
    }

    bool subscribe(const oxenmq::ConnectionID& conn, std::string peer_rpc_endpoint) {
        return subs.subscribe(conn, std::move(peer_rpc_endpoint));
    }

    bool unsubscribe(const oxenmq::ConnectionID& conn) {
        return subs.unsubscribe(conn).has_value();
    }

    void remove_expired() {
        subs.remove_expired();
    }

    void send_all(const oxenmq::ConnectionID& conn, const std::string& endpoint)
    {
        omq.send(conn, endpoint, oxenmq::send_option::data_parts(buffer->get_all()));
    }
};
 
} // namespace oxen::log

// vim:sw=4:et
