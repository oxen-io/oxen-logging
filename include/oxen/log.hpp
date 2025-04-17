#pragma once

// Header for actual log statements such as oxen::log::info(...) and so on.

#include <memory>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/dist_sink.h>

#include "log/level.hpp"
#include "log/type.hpp"
#include "log/color.hpp"
#include "log/internal.hpp"
#include "log/catlogger.hpp"

namespace oxen::log {

using logger_ptr = std::shared_ptr<spdlog::logger>;

// Our master sink where all log output goes; we add sub-sinks into this as desired, but this
// master sink stays around forever.
extern std::shared_ptr<spdlog::sinks::dist_sink_mt> master_sink;

// Function-like logging statements.  These are structs for technical reasons, but are meant to be
// used as if functions: all of the logging involved happens in the constructor.

/// Log a "trace" log statement.  Use this as if a function, where the first argument is (typically)
/// a CategoryLogger, the second argument is an fmt pattern, and the rest of the arguments are
/// arguments for the formatted string.
template <typename... T>
struct trace {
    trace(const logger_ptr& cat_logger,
          [[maybe_unused]] fmt::format_string<T...> fmt,
          [[maybe_unused]] T&&... args,
          [[maybe_unused]] const source_location& location = source_location::current()) {
#if defined(NDEBUG) && !defined(OXEN_LOGGING_RELEASE_TRACE)
        // Using [[maybe_unused]] on the *first* ctor argument breaks gcc 8/9
        (void)cat_logger;
#else
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::trace, fmt, std::forward<T>(args)...);
#endif
    }
    trace(const logger_ptr& cat_logger,
          [[maybe_unused]] const fmt::text_style& sty,
          [[maybe_unused]] fmt::format_string<T...> fmt,
          [[maybe_unused]] T&&... args,
          [[maybe_unused]] const source_location& location = source_location::current()) {
#if defined(NDEBUG) && !defined(OXEN_LOGGING_RELEASE_TRACE)
        // Using [[maybe_unused]] on the *first* ctor argument breaks gcc 8/9
        (void)cat_logger;
#else
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::trace,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
#endif
    }
};
/// Log a "debug" log statement.  Use this as if a function, where the first argument is (typically)
/// a CategoryLogger, the second argument is an fmt pattern, and the rest of the arguments are
/// arguments for the formatted string.
template <typename... T>
struct debug {
    debug(const logger_ptr& cat_logger,
          fmt::format_string<T...> fmt,
          T&&... args,
          const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::debug, fmt, std::forward<T>(args)...);
    }
    debug(const logger_ptr& cat_logger,
          const fmt::text_style& sty,
          fmt::format_string<T...> fmt,
          T&&... args,
          const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::debug,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};
/// Log a "info" log statement.  Use this as if a function, where the first argument is (typically)
/// a CategoryLogger, the second argument is an fmt pattern, and the rest of the arguments are
/// arguments for the formatted string.
template <typename... T>
struct info {
    info(const logger_ptr& cat_logger,
         fmt::format_string<T...> fmt,
         T&&... args,
         const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::info, fmt, std::forward<T>(args)...);
    }
    info(const logger_ptr& cat_logger,
         const fmt::text_style& sty,
         fmt::format_string<T...> fmt,
         T&&... args,
         const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::info,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};
/// Log a "warning" log statement.  Use this as if a function, where the first argument is
/// (typically) a CategoryLogger, the second argument is an fmt pattern, and the rest of the
/// arguments are arguments for the formatted string.
template <typename... T>
struct warning {
    warning(const logger_ptr& cat_logger,
            fmt::format_string<T...> fmt,
            T&&... args,
            const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::warn, fmt, std::forward<T>(args)...);
    }
    warning(const logger_ptr& cat_logger,
            const fmt::text_style& sty,
            fmt::format_string<T...> fmt,
            T&&... args,
            const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::warn,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};
/// Log a "error" log statement.  Use this as if a function, where the first argument is (typically)
/// a CategoryLogger, the second argument is an fmt pattern, and the rest of the arguments are
/// arguments for the formatted string.
template <typename... T>
struct error {
    error(const logger_ptr& cat_logger,
          fmt::format_string<T...> fmt,
          T&&... args,
          const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::err, fmt, std::forward<T>(args)...);
    }
    error(const logger_ptr& cat_logger,
          const fmt::text_style& sty,
          fmt::format_string<T...> fmt,
          T&&... args,
          const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::err,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};
/// Log a "critical" log statement.  Use this as if a function, where the first argument is
/// (typically) a CategoryLogger, the second argument is an fmt pattern, and the rest of the
/// arguments are arguments for the formatted string.
template <typename... T>
struct critical {
    critical(
            const logger_ptr& cat_logger,
            fmt::format_string<T...> fmt,
            T&&... args,
            const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location), Level::critical, fmt, std::forward<T>(args)...);
    }
    critical(
            const logger_ptr& cat_logger,
            const fmt::text_style& sty,
            fmt::format_string<T...> fmt,
            T&&... args,
            const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    Level::critical,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};

template <typename... T>
struct log {
    log(const logger_ptr& cat_logger,
        Level level,
        fmt::format_string<T...> fmt,
        T&&... args,
        const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(detail::spdlog_sloc(location), level, fmt, std::forward<T>(args)...);
    }
    log(const logger_ptr& cat_logger,
        Level level,
        const fmt::text_style& sty,
        fmt::format_string<T...> fmt,
        T&&... args,
        const source_location& location = source_location::current()) {
        if (cat_logger)
            cat_logger->log(
                    detail::spdlog_sloc(location),
                    level,
                    "{}",
                    detail::text_style_wrapper{sty, fmt, fmt::make_format_args(args...)});
    }
};

// Deduction guides for our logging function-like structs; these force all arguments given in the
// constructor to become explicit `T` constructor arguments, which in turn forces the
// source_location constructor argument to always get defaulted, which is what we want.  (This
// little deduction guide trick is why we need classes: automatic deduction of generic types won't
// work with the trailing defaulted value).
template <typename... T>
trace(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> trace<T...>;
template <typename... T>
trace(const logger_ptr& cat, const fmt::text_style& sty, fmt::format_string<T...> fmt, T&&... args)
        -> trace<T...>;

template <typename... T>
debug(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> debug<T...>;
template <typename... T>
debug(const logger_ptr& cat, const fmt::text_style& sty, fmt::format_string<T...> fmt, T&&... args)
        -> debug<T...>;

template <typename... T>
info(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> info<T...>;
template <typename... T>
info(const logger_ptr& cat, const fmt::text_style& sty, fmt::format_string<T...> fmt, T&&... args)
        -> info<T...>;

template <typename... T>
warning(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> warning<T...>;
template <typename... T>
warning(const logger_ptr& cat,
        const fmt::text_style& sty,
        fmt::format_string<T...> fmt,
        T&&... args) -> warning<T...>;

template <typename... T>
error(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> error<T...>;
template <typename... T>
error(const logger_ptr& cat, const fmt::text_style& sty, fmt::format_string<T...> fmt, T&&... args)
        -> error<T...>;

template <typename... T>
critical(const logger_ptr& cat, fmt::format_string<T...> fmt, T&&... args) -> critical<T...>;
template <typename... T>
critical(
        const logger_ptr& cat,
        const fmt::text_style& sty,
        fmt::format_string<T...> fmt,
        T&&... args) -> critical<T...>;

template <typename... T>
log(const logger_ptr& cat, Level level, fmt::format_string<T...> fmt, T&&... args) -> log<T...>;
template <typename... T>
log(const logger_ptr& cat,
    Level level,
    const fmt::text_style& sty,
    fmt::format_string<T...> fmt,
    T&&... args) -> log<T...>;

/// Resets the log level of all existing category loggers, and sets a new default for any created
/// after this call.  If this has not been called, the default log level of category loggers is
/// info.
void reset_level(Level level);

/// Sets the log level of new category loggers initialized after this call, but does not change the
/// log level of already-initialized category loggers.
void set_level_default(Level level);

/// Gets the default log level of new loggers (since the last reset_level or set_level_default
/// call).
Level get_level_default();

/// Set the log level of a logger
inline void set_level(const logger_ptr& cat, Level level) {
    cat->set_level(level);
}
/// Set the log level of a logger, by logger category name.
void set_level(std::string cat_name, Level level);

/// Gets the log level of a logger
inline Level get_level(const logger_ptr& cat) {
    return cat->level();
}
/// Gets the log level of a logger by, logger category name.
Level get_level(std::string cat_name);

/// Flushes the logging sink(s) immediately.
void flush();

/// The default pattern when no explicit pattern is given and you are using an ansi-color-supporting
/// log sink.
const std::string DEFAULT_PATTERN_COLOR =
        "[%Y-%m-%d %T] [%*] [\x1b[1m%n\x1b[0m:%^%l%$|\x1b[3m%g:%#\x1b[0m] %v";

/// The default pattern when no explicit pattern is given and not using an ansi-color-supporting log
/// sink.
const std::string DEFAULT_PATTERN_MONO = "[%Y-%m-%d %T] [%*] [%n:%^%l%$|%g:%#] %v";

/// Adds a logging sink to the list of logging sinks where output goes; existing sinks are not
/// affected.  You *must* call this at least once before log output will go anywhere.
///
/// • type defines the type of sink (file, print, syslog)
/// • target is the type-dependent "target" of the sink:
///   - for file sinks, target is the output filename
///   - for print sinks, target can be "", "-", "stdout" for coloured stdout; "stderr" for coloured
///     stderr; "nocolor" or "stdout-nocolor" for monochrome stdout; or "stderr-nocolor" for
///     monochrome stderr.
///   - for syslog sinks, target is an application identifier (e.g. "lokinet")
/// • pattern is an log output format pattern to use instead of the default.  This is a standard
///   spdlog formatting string with custom format '%*' added to print a time-elapsed-since-startup
///   value.
void add_sink(
        Type type, std::string_view target, std::optional<std::string> pattern = std::nullopt);

/// Adds a manually constructed spdlog sink to the logging sinks.  This is for advanced cases where
/// the above add_sink won't work.
void add_sink(spdlog::sink_ptr, std::optional<std::string> pattern = std::nullopt);

/// Removes all existing log sinks, typically to replace the current log sink.  Note that until
/// `add_sink` is called after this, logging output will not go anywhere.
void clear_sinks();

}  // namespace oxen::log
