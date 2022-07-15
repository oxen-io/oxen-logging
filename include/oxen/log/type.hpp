#pragma once

#include <string_view>

namespace oxen::log {

enum class Type {
    Unknown = 0,
    File,
    System,
    Print,
};

/// Returns the logging type from a string; string values are the same as the enum names, but
/// lower-cased.  Also supports "syslog" as an alias for System.  Returns Unknown if unknown.
Type type_from_string(std::string_view type);

/// Returns the string representation of a logging type, e.g. "print", "system"
std::string_view to_string(Type t);

}  // namespace oxen::log
