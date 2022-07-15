#include <oxen/log/level.hpp>
#include <unordered_map>
#include <algorithm>
#include <spdlog/common.h>
#include <string_view>

namespace oxen::log {

std::string_view to_string(Level lvl) {
    auto l = spdlog::level::to_string_view(lvl);
    return {l.data(), l.size()};
}

std::optional<Level> level_from_string(std::string level) {
    // Convert to lower case:
    for (char& ch : level)
        if (ch >= 'A' && ch <= 'Z')
            ch += 'a' - 'A';

    // Special case "off" because the spdlog call below returns off for unknown inputs
    if (level == "off" || level == "none")
        return spdlog::level::off;
    auto l = spdlog::level::from_str(level);
    if (l == spdlog::level::off)
        return std::nullopt;
    return l;
}

}  // namespace oxen::log
