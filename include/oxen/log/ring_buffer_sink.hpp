#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <list>
#include <string>
#include <functional>

namespace oxen::log {

namespace detail {
    struct MessageList
    {
    private:
        const size_t max_size;
        std::list<std::string> messages;

    public:
        MessageList(size_t size = 100)
            : max_size(size) {}

        void add(std::string msg)
        {
            messages.push_back(std::move(msg));
            if (messages.size() > max_size)
                messages.pop_front();
        }

        std::list<std::string> get_all() {
            return messages;
        }
    };

} // namespace oxen::log::detail

using sink_type = spdlog::sinks::base_sink<std::mutex>;

class RingBufferSink : public sink_type
{
public:
    using LogCallback = std::function<void(const std::string&)>;

private:
    detail::MessageList logs;
    LogCallback onLog = nullptr;

public:
    RingBufferSink(size_t max_size = 100, LogCallback callback = nullptr)
        : logs{max_size}, onLog{std::move(callback)} {}

    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t buf;
        formatter_->format(msg, buf);
        std::string as_str = to_string(buf);
        if (onLog) onLog(as_str);
        logs.add(std::move(as_str));
    }

    void set_log_callback(LogCallback callback = nullptr)
    {
        std::lock_guard lock{mutex_};
        onLog = std::move(callback);
    }

    std::list<std::string> get_all() {
        std::lock_guard lock{mutex_};
        return logs.get_all();
    }

    void flush_() override {};
};
 
} // namespace oxen::log

// vim:sw=4:et

