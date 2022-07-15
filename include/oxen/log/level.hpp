#pragma once

#include <optional>
#include <string_view>

#include <spdlog/common.h>

namespace oxen::log {

using Level = spdlog::level::level_enum;

/// Returns a string representation of the log level (e.g. "trace", "warning")
std::string_view to_string(Level lvl);

/// Takes a log level string and converts it to a Level value.  Returns nullopt if the string
/// isn't a valid log level.
std::optional<Level> level_from_string(std::string level);

}  // namespace oxen::log
