#include <oxen/log/catlogger.hpp>

#include <spdlog/sinks/dist_sink.h>

#if defined(_WIN32) || defined(WIN32)
#define OXEN_LOGGING_EXPORT __declspec(dllexport)
#else
#define OXEN_LOGGING_EXPORT __attribute__((visibility("default")))
#endif

namespace oxen::log {

inline auto get_master_sink() {
    static auto ms = std::make_shared<spdlog::sinks::dist_sink_mt>();
    return ms;
}

std::shared_ptr<spdlog::sinks::dist_sink_mt> master_sink = get_master_sink();

namespace detail {

    OXEN_LOGGING_EXPORT std::unordered_map<std::string, logger_ptr>& loggers() {
        static std::unordered_map<std::string, logger_ptr> loggers_impl;
        return loggers_impl;
    }
    OXEN_LOGGING_EXPORT std::mutex& loggers_mutex() {
        static std::mutex loggers_mutex_impl;
        return loggers_mutex_impl;
    }
    OXEN_LOGGING_EXPORT Level& loggers_default_level() {
        static Level default_level_impl = Level::info;
        return default_level_impl;
    }

    void set_default_catlogger_level(Level level) {
        loggers_default_level() = level;
    }

    Level get_default_catlogger_level() {
        return loggers_default_level();
    }

}  // namespace detail

CategoryLogger::CategoryLogger(std::string name_) : name{std::move(name_)} {
    std::lock_guard lock{detail::loggers_mutex()};
    // Insert an empty shared_ptr here because we don't want to create the underlying logger until
    // first use, but we at least want to know the category exists.
    detail::loggers()[name];
}

void CategoryLogger::find_or_make_logger() {
    if (have_logger)
        return;

    std::lock_guard lock{detail::loggers_mutex()};
    auto& known_logger = detail::loggers()[name];
    if (!known_logger) {
        known_logger = std::make_shared<spdlog::logger>(name, master_sink);
        known_logger->set_level(detail::loggers_default_level());
    }

    logger = known_logger;
    have_logger = true;
}

void for_each_cat_logger(
        std::function<void(const std::string& name, spdlog::logger&)> f,
        std::function<void()> and_then) {
    std::lock_guard lock{detail::loggers_mutex()};
    if (f)
        for (auto& [name, logger] : detail::loggers())
            if (logger)
                f(name, *logger);
    if (and_then)
        and_then();
}

void for_each_cat_name(
        std::function<void(const std::string& name)> f, std::function<void()> and_then) {
    std::lock_guard lock{detail::loggers_mutex()};
    if (f)
        for (auto& [name, logger] : detail::loggers())
            f(name);
    if (and_then)
        and_then();
}

}  // namespace oxen::log
