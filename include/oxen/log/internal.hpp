#pragma once

#include <array>
#include <memory>
#include <spdlog/spdlog.h>
#include "type.hpp"
#include "level.hpp"

#if __has_include(<version>)
#include <version>
#endif

#ifdef __cpp_lib_source_location

#include <source_location>
namespace oxen::log {
using source_location = std::source_location;
}

#elif __has_include(<experimental/source_location>)

#include <experimental/source_location>
#ifdef __cpp_lib_experimental_source_location
namespace oxen::log {
using source_location = std::experimental::source_location;
}
#else
#error "Unable to find a working <source_location> or <experimental/source_location>"
#endif

#else
#error "This compiler and/or stdlib does not support <source_location>"
#endif

namespace oxen::log {
using logger_ptr = std::shared_ptr<spdlog::logger>;
}

namespace oxen::log::detail {

/** internal */

spdlog::sink_ptr make_sink(Type type, const std::string& file);

bool is_ansicolor_sink(const spdlog::sink_ptr& sink);

inline auto spdlog_sloc(const source_location& loc) {
    std::string_view filename{loc.file_name()};
    // We try to keep the last two path components, e.g. "bar/x.cpp" from "/home/me/src/bar/x.cpp",
    // unless we end up with src/x.cpp in which case we drop the src/ and just keep "x.cpp":
    if (auto pos = filename.rfind('/'); pos != 0 && pos != std::string_view::npos) {
        if (pos = filename.rfind('/', pos - 1); pos != std::string_view::npos)
            filename.remove_prefix(pos + 1);
        if (filename.starts_with("src/"))
            filename.remove_prefix(4);
    }
    return spdlog::source_loc{filename.data(), static_cast<int>(loc.line()), loc.function_name()};
}

inline void make_lc(std::string& s) {
    for (char& c : s)
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
}

}  // namespace oxen::log::detail
