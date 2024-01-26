#pragma once

/// This header defines a ""_format user-defined literal that works similarly to fmt::format, but
/// with a more clever syntax:
///
///     fmt::format("xyz {}", 42);
///
/// becomes:
///
///     "xyz {}"_format(42);
///
/// There is also a `_format_to` that allows in-place appending to an existing string (or
/// something string-like):
///
///     "xyz {}"_format_to(somestr, 42);
///
/// is a shortcut for:
///
///     fmt::format_to(std::back_inserter(somestr), "{}", 42);
///
/// which is equivalent to (but more efficient than):
///
///     somestr += "xyz {}"_format(42);
///
/// The functions live in the `oxen::log::literals` namespace; you should use them via:
///
///     #include <oxen/log/format.hpp>
///     // ...
///     using namespace oxen::log::literals;
///
/// to make them available (the header/namespace is not included by default from oxen-logging
/// headers).

#include <array>
#include <iterator>
#include <string_view>

#include <fmt/core.h>

#ifdef _MSVC_LANG
#define OXEN_LOGGING_CPLUSPLUS _MSVC_LANG
#else
#define OXEN_LOGGING_CPLUSPLUS __cplusplus
#endif

namespace oxen::log {

namespace detail {

#if OXEN_LOGGING_CPLUSPLUS >= 202002L

    template <size_t N>
    struct string_literal {
        std::array<char, N> str;

        consteval string_literal(const char (&s)[N]) { std::copy(s, s + N, str.begin()); }

        consteval std::string_view sv() const { return {str.data(), N}; }
    };

    // Internal implementation of _format that holds the format as a compile-time string in the type
    // itself; when the (...) operator gets called we give that off to fmt::format (and so just like
    // using fmt::format directly, you get compiler errors if the arguments do not match).
    template <string_literal Format>
    struct fmt_wrapper {
        consteval fmt_wrapper() = default;

        /// Calling on this object forwards all the values to fmt::format, using the format string
        /// as provided during type definition (via the "..."_format user-defined function).
        template <typename... T>
        constexpr auto operator()(T&&... args) && {
            return fmt::format(Format.sv(), std::forward<T>(args)...);
        }
    };

    template <string_literal Format>
    struct fmt_append_wrapper : fmt_wrapper<Format> {
        consteval fmt_append_wrapper() = default;

        template <typename String, typename... T>
        constexpr auto operator()(String& s, T&&... args) && {
            return fmt::format_to(std::back_inserter(s), Format.sv(), std::forward<T>(args)...);
        }
    };

#else  // Not C++20:

    // Internal implementation of _format that holds the format temporarily until the (...) operator
    // is invoked on it.  This object cannot be moved, copied but only used ephemerally in-place.
    struct fmt_wrapper17 {
      protected:
        std::string_view format;

        // Non-copyable and non-movable:
        fmt_wrapper17(const fmt_wrapper17&) = delete;
        fmt_wrapper17& operator=(const fmt_wrapper17&) = delete;
        fmt_wrapper17(fmt_wrapper17&&) = delete;
        fmt_wrapper17& operator=(fmt_wrapper17&&) = delete;

      public:
        constexpr explicit fmt_wrapper17(const char* str, const std::size_t len) :
                format{str, len} {}

        /// Calling on this object forwards all the values to fmt::format, using the format string
        /// as provided during construction (via the "..."_format user-defined function).
        template <typename... T>
        auto operator()(T&&... args) && {
            return fmt::format(format, std::forward<T>(args)...);
        }
    };

    struct fmt_append_wrapper17 : fmt_wrapper17 {
        using fmt_wrapper17::fmt_wrapper17;

        template <typename String, typename... T>
        auto operator()(String& s, T&&... args) && {
            return fmt::format_to(std::back_inserter(s), format, std::forward<T>(args)...);
        }
    };

#endif

}  // namespace detail

inline namespace literals {

#if OXEN_LOGGING_CPLUSPLUS >= 202002L
    template <detail::string_literal Format>
    inline consteval auto operator""_format() {
        return detail::fmt_wrapper<Format>{};
    }

    template <detail::string_literal Format>
    inline consteval auto operator""_format_to() {
        return detail::fmt_append_wrapper<Format>{};
    }
#else
    inline detail::fmt_wrapper17 operator""_format(const char* str, size_t len) {
        return detail::fmt_wrapper17{str, len};
    }

    inline detail::fmt_append_wrapper17 operator""_format_to(const char* str, size_t len) {
        return detail::fmt_append_wrapper17{str, len};
    }
#endif

}  // namespace literals

}  // namespace oxen::log
