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

#include <fmt/core.h>
#include <string_view>
#include <iterator>

namespace oxen::log {

namespace detail {

    // Internal implementation of _format that holds the format temporarily until the (...) operator
    // is invoked on it.  This object cannot be moved, copied but only used ephemerally in-place.
    struct fmt_wrapper {
      protected:
        std::string_view format;

        // Non-copyable and non-movable:
        fmt_wrapper(const fmt_wrapper&) = delete;
        fmt_wrapper& operator=(const fmt_wrapper&) = delete;
        fmt_wrapper(fmt_wrapper&&) = delete;
        fmt_wrapper& operator=(fmt_wrapper&&) = delete;

      public:
        constexpr explicit fmt_wrapper(const char* str, const std::size_t len) : format{str, len} {}

        /// Calling on this object forwards all the values to fmt::format, using the format string
        /// as provided during construction (via the "..."_format user-defined function).
        template <typename... T>
        auto operator()(T&&... args) && {
            return fmt::format(format, std::forward<T>(args)...);
        }
    };

    struct fmt_append_wrapper : fmt_wrapper {
        using fmt_wrapper::fmt_wrapper;

        template <typename String, typename... T>
        auto operator()(String& s, T&&... args) && {
            return fmt::format_to(std::back_inserter(s), format, std::forward<T>(args)...);
        }
    };

}  // namespace detail

inline namespace literals {

    inline detail::fmt_wrapper operator""_format(const char* str, size_t len) {
        return detail::fmt_wrapper{str, len};
    }

    inline detail::fmt_append_wrapper operator""_format_to(const char* str, size_t len) {
        return detail::fmt_append_wrapper{str, len};
    }

}  // namespace literals

}  // namespace oxen::log
