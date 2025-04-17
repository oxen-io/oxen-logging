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
struct text_style_wrapper {
    const fmt::text_style& sty;
    fmt::string_view fmt;
    fmt::format_args args;

    text_style_wrapper(const fmt::text_style& sty, fmt::string_view fmt, fmt::format_args args) :
            sty{sty}, fmt{fmt}, args{std::move(args)} {}
};

}  // namespace oxen::log::detail

template <>
struct fmt::formatter<oxen::log::detail::text_style_wrapper> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const oxen::log::detail::text_style_wrapper& f, fmt::format_context& ctx) const {
        auto out = ctx.out();
        return fmt::vformat_to(out, f.sty, f.fmt, f.args);
    }
};
