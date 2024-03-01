#pragma once

#include <fmt/color.h>
#include <tuple>

namespace oxen::log::detail {

// Wraps text_style, fmt, and arguments and outputs them via fmt when formatted.  This is here so
// that we can use styled text that *won't* go through formatting when the logging level isn't
// active while still lets us use fmt's text_style to color/emphasize/etc. the text, and avoids
// double-formatting (if we use fmt::format ourself in the log argument, and the output happens to
// have {} in it).
//
// This object should not be called directly; instead call log::info, etc.  with a text style as
// first argument.
template <typename... T>
struct text_style_wrapper {
    const fmt::text_style& sty;
    fmt::basic_string_view<char> fmt;
    const std::tuple<const T&...> args;

    text_style_wrapper(const fmt::text_style& sty, fmt::format_string<T...> fmt, const T&... args) :
            sty{sty}, fmt{fmt}, args{std::tie(args...)} {}
};

}  // namespace oxen::log::detail

template <typename... T>
struct fmt::formatter<oxen::log::detail::text_style_wrapper<T...>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const oxen::log::detail::text_style_wrapper<T...>& f, FormatContext& ctx) {
        auto out = ctx.out();
        return std::apply(
                [&](const auto&... args) { return fmt::format_to(out, f.sty, f.fmt, args...); },
                f.args);
    }
};
