#include <oxen/log.hpp>
#include <oxen/log/type.hpp>
#include <oxen/log/catlogger.hpp>
#include <oxen/log/format.hpp>

#include <chrono>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#if defined(_WIN32)
#include <spdlog/sinks/win_eventlog_sink.h>
#elif defined(ANDROID)
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/syslog_sink.h>
#endif

namespace oxen::log {

namespace {

    const auto started_at = std::chrono::steady_clock::now();

    // Custom log formatting flag that prints the elapsed time since startup
    class startup_elapsed_flag : public spdlog::custom_flag_formatter {
      public:
        void format(const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest)
                override {
            using namespace std::literals;
            auto elapsed = std::chrono::steady_clock::now() - started_at;

            dest.append(fmt::format(
                    elapsed >= 1h     ? "+{0:d}h{1:02d}m{2:02d}.{3:03d}s"
                    : elapsed >= 1min ? "+{1:d}m{2:02d}.{3:03d}s"
                                      : "+{2:d}.{3:03d}s",
                    std::chrono::duration_cast<std::chrono::hours>(elapsed).count(),
                    (std::chrono::duration_cast<std::chrono::minutes>(elapsed) % 1h).count(),
                    (std::chrono::duration_cast<std::chrono::seconds>(elapsed) % 1min).count(),
                    (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed) % 1s).count()));
        }

        std::unique_ptr<custom_flag_formatter> clone() const override {
            return std::make_unique<startup_elapsed_flag>();
        }
    };

    template <typename T, typename U>
    bool is_instance(const U* ptr) {
        return dynamic_cast<const T*>(ptr) != nullptr;
    }

    bool is_ansicolor_sink(const spdlog::sink_ptr& sink) {
#ifdef _WIN32
        (void) sink;
        return false;
#else
        auto* s = sink.get();
        return is_instance<spdlog::sinks::ansicolor_stdout_sink_mt>(s) ||
               is_instance<spdlog::sinks::ansicolor_stderr_sink_mt>(s);
#endif
    }

    void set_sink_format(const spdlog::sink_ptr& sink, std::optional<std::string> pattern) {
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<startup_elapsed_flag>('*');
        if (pattern)
            formatter->set_pattern(*std::move(pattern));
        else if (is_ansicolor_sink(sink))
            formatter->set_pattern(DEFAULT_PATTERN_COLOR);
        else
            formatter->set_pattern(DEFAULT_PATTERN_MONO);
        sink->set_formatter(std::move(formatter));
    }

    spdlog::sink_ptr make_sink(Type type, std::string_view target) {

        spdlog::sink_ptr sink;

        switch (type) {
            case Type::Print:
                if (target.empty() || target == "stdout" || target == "-")
                    sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(
                            spdlog::color_mode::always);
                else if (target == "nocolor" || target == "stdout-nocolor")
                    sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
                else if (target == "stderr")
                    sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>(
                            spdlog::color_mode::always);
                else if (target == "stderr-nocolor")
                    sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
                else
                    throw std::invalid_argument{
                            "{} is not a valid target for type=Print logging"_format(target)};
                break;

            case Type::File:
                // throws on error
                sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{target});
                break;

            case Type::System:
#ifdef _WIN32
                sink = std::make_shared<spdlog::sinks::win_eventlog_sink_mt>(std::string{target});
#elif defined(ANDROID)
                sink = std::make_shared<spdlog::sinks::android_sink_mt>(std::string{target});
#else
                sink = std::make_shared<spdlog::sinks::syslog_sink_mt>(
                        std::string{target}, 0, LOG_DAEMON, true);
#endif
                break;
        }
        return sink;
    }

}  // namespace

void reset_level(Level level) {
    for_each_cat_logger(
            [level](const std::string&, spdlog::logger& logger) { logger.set_level(level); },
            [level]() { detail::set_default_catlogger_level(level); });
}

void set_level_default(Level level) {
    for_each_cat_logger(nullptr, [level]() { detail::set_default_catlogger_level(level); });
}

Level get_level_default() {
    Level lvl;
    for_each_cat_logger(nullptr, [&lvl]() { lvl = detail::get_default_catlogger_level(); });
    return lvl;
}

void set_level(std::string cat_name, Level level) {
    Cat(std::move(cat_name))->set_level(level);
}

Level get_level(std::string cat_name) {
    return Cat(std::move(cat_name))->level();
}

void flush() {
    master_sink->flush();
}

void add_sink(spdlog::sink_ptr sink, std::optional<std::string> pattern) {
    set_sink_format(sink, std::move(pattern));
    master_sink->add_sink(std::move(sink));
}

void add_sink(Type type, std::string_view target, std::optional<std::string> pattern) {
    add_sink(make_sink(type, target), std::move(pattern));
}

void clear_sinks() {
    master_sink->set_sinks({});
}

}  // namespace oxen::log
