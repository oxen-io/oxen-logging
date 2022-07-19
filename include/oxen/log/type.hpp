#pragma once

#include <string>

namespace oxen::log {

enum class Type {
    File,
    System,
    Print,
};

/// Returns the logging type from a string; string values are the same as the enum names
/// (case-insensitive).  Also supports "syslog" as an alias for System.  Throws
/// std::invalid_argument on unknown values.
Type type_from_string(std::string type);

/// Returns the string representation of a logging type, i.e. "file", "print", or "system"
std::string_view to_string(Type t);

}  // namespace oxen::log
