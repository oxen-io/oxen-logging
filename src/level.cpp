#include <oxen/log/level.hpp>
#include <oxen/log/internal.hpp>
#include <oxen/log/format.hpp>
#include <stdexcept>
#include <spdlog/common.h>
#include <string_view>

namespace oxen::log {

std::string_view to_string(Level lvl) {
    auto l = spdlog::level::to_string_view(lvl);
    return {l.data(), l.size()};
}

Level level_from_string(std::string level) {
    detail::make_lc(level);

    // Special case "off" because the spdlog call below returns off for unknown inputs
    if (level == "off" || level == "none")
        return spdlog::level::off;
    if (auto l = spdlog::level::from_str(level); l != spdlog::level::off)
        return l;
    throw std::invalid_argument{"Invalid log level '{}'"_format(level)};
}

}  // namespace oxen::log
