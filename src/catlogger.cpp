#include <oxen/log/catlogger.hpp>

#include <spdlog/sinks/dist_sink.h>

namespace oxen::log {

std::shared_ptr<spdlog::sinks::dist_sink_mt> master_sink =
        std::make_shared<spdlog::sinks::dist_sink_mt>();

static std::unordered_map<std::string, logger_ptr> loggers_;
static std::mutex loggers_mutex_;
static Level loggers_default_level_ = Level::info;  // Default log level for new CategoryLoggers

void CategoryLogger::find_or_make_logger() {
    std::lock_guard lock{loggers_mutex_};
    if (have_logger)
        return;

    auto& known_logger = loggers_[name];
    if (!known_logger) {
        known_logger = std::make_shared<spdlog::logger>(name, master_sink);
        known_logger->set_level(loggers_default_level_);
    }

    logger = known_logger;
    have_logger = true;
}

void for_each_cat_logger(
        std::function<void(const std::string& name, spdlog::logger&)> f,
        std::function<void()> and_then) {
    std::lock_guard lock{loggers_mutex_};
    if (f)
        for (auto& [name, logger] : loggers_)
            f(name, *logger);
    if (and_then)
        and_then();
}

namespace detail {

    void set_default_catlogger_level(Level level) { loggers_default_level_ = level; }

    Level get_default_catlogger_level() { return loggers_default_level_; }

}  // namespace detail

}  // namespace oxen::log
