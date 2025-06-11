#pragma once

#include <list>
#include <optional>
#include <string_view>

#include <spdlog/common.h>

namespace oxen::log {

using Level = spdlog::level::level_enum;

/// Returns a string representation of the log level (e.g. "trace", "warning")
std::string_view to_string(Level lvl);

/// Takes a log level string (case-insensitive) and converts it to a Level value.  Throws
/// std::invalid_argument if the string isn't a valid log level.  Valid strings are:
/// - "trace"
/// - "debug"
/// - "info"
/// - "warn" or "warning"
/// - "error" or "err"
/// - "critical" or "crit"
/// - "none", "off", or "disabled"
///
/// Additional level strings can be added by an application using the add_level_compat_string(), but
/// this is discouraged except where needed to maintain backwards compatibility.
Level level_from_string(std::string level);

/// Adds a string to parse as a given log level for backwards compatibility.  This affects both
/// `level_from_string` and `extract_categories`.  This is not recommended for any new code, but
/// rather for older code (such as oxen-core) where existing config files may specify log levels as
/// "1" or "wrn".
void add_level_compat_string(std::string key, Level level);

// Return type of extract_categories.
struct LogCats {
    // Contains the default level, if one was given.
    std::optional<Level> default_level;
    // Contains the levels to apply to all given (or matched) categories.
    std::unordered_map<std::string, Level> cat_levels;
    // Returns true if neither a defaul nor any category levels were parsed or matched at all.
    bool empty() const { return !default_level && cat_levels.empty(); }

    // Applies the settings in the object to the global logger.  This string is reconstructed
    // according to the parsed and matched results and so will not necessarily exactly match the
    // input.
    //
    // If the optional callback is set then it will be called immediately after applying the default
    // level (it is not called at all if the category string does not specify a default).  This is
    // intended for code to have a global log level imply tweaks levels for some categories: for
    // instance, oxen-core at a global (and default) "WARNING" level puts some unnecessarily verbose
    // categories into ERR level, and some others into "INFO" level, and makes similar changes at a
    // global INFO level.  The callback is given the global Level that has just been applied to all
    // categories.
    std::list<std::string> apply(std::function<void(Level)> amend_default = nullptr);
};

/// Parses a user-provided logging string of levels and/or categories and returns an object
/// containing the parsed results (which can be applied by invoking `.apply()` on it).  The input
/// string consists of any number of the following, separated by whitespace, commas (,), or
/// semicolons (;).
///
/// - *=LEVEL, where LEVEL is warning, debug, and so on.  This resets *all* log categories to the
///   given log level, and so typically should be the first (or only) argument given.
/// - A single level string by itself, such as `critical` or `off`.  This is an alias for `*=LEVEL`.
/// - CAT=LEVEL where CAT is a log category ("quic") and LEVEL is a level such as `trace` or `info`.
/// - Category globbing, such as `db.*=debug` or `*test=off` sets the level of all matching, known
///   log categories at the time the levels are parsed.  Note that log categories are dynamic and so
///   a category that is not yet created (such as an "on the fly" `log::Cat("xyz")` in a log
///   statement, or from a library that is dynamically loaded after this call) may not be matched if
///   the code path that creates it has never been called.
/// - A `:` can also be given instead of `=` (for example: `MyCat:warning`) for backwards
///   compatibility, but its use is discouraged.
///
/// Categories are case-sensitive; log levels are not.  Allowed level strings are: "trace",
/// "debug"/"dbg", "info", "warning"/"warn", "error"/"err", "critical"/"crit", and
/// "off"/"disabled"/"none".  (Additional level strings for backwards compatibility can be added by
/// calling `add_level_compat_string()`).
///
/// Levels are applied in the order given, and so generally the more general values should be first.
/// For example `*=ERROR, test*=WARN, test42=INFO` sets the level to info for `test42`, warning for
/// `test` and `test123`, and error for everything else.  However, `test42=INFO, test*=WARN,
/// *=ERROR` is likely a mistake: it would apply error log levels to all categories.
///
/// This call itself makes error logs to a `logging` category on unparseable input values, and an
/// info log statement is also emitted when applying the categories.
LogCats extract_categories(std::string_view categories);

/// Shortcut for `extract_categories(string).apply()`.
inline void apply_categories(std::string_view categories) {
    extract_categories(categories).apply();
}

}  // namespace oxen::log
