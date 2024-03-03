#pragma once

#if (defined(__GNUC__) && __GNUC__ >= 11) || (defined(__clang__) && __clang_major__ >= 15)
#include <source_location>
#else
#include <experimental/source_location>
#define USING_EXPERIMENTAL_SRCLOC
#endif

#include <array>
#include <spdlog/spdlog.h>
#include "type.hpp"
#include "level.hpp"

using source_location =
#ifdef USING_EXPERIMENTAL_SRCLOC
        std::experimental::source_location;
#else
        std::source_location;
#endif

namespace oxen::log {

using logger_ptr = std::shared_ptr<spdlog::logger>;
}

namespace oxen::log::detail {

/** internal */

spdlog::sink_ptr make_sink(Type type, const std::string& file);

bool is_ansicolor_sink(const spdlog::sink_ptr& sink);

#ifndef OXEN_LOGGING_SOURCE_ROOTS_LEN
#define OXEN_LOGGING_SOURCE_ROOTS_LEN 0
#endif

inline constexpr std::array<std::string_view, OXEN_LOGGING_SOURCE_ROOTS_LEN> source_prefixes = {
#ifdef OXEN_LOGGING_SOURCE_ROOTS
        OXEN_LOGGING_SOURCE_ROOTS,
#endif
};

inline auto spdlog_sloc(const source_location& loc) {
    std::string_view filename{loc.file_name()};
    for (const auto& prefix : source_prefixes) {
        if (filename.substr(0, prefix.size()) == prefix) {
            filename.remove_prefix(prefix.size());
            if (!filename.empty() && filename[0] == '/')
                filename.remove_prefix(1);
        }
    }
    while (filename.substr(0, 3) == "../")
        filename.remove_prefix(3);

    return spdlog::source_loc{filename.data(), static_cast<int>(loc.line()), loc.function_name()};
}

inline void make_lc(std::string& s) {
    for (char& c : s)
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
}

}  // namespace oxen::log::detail
