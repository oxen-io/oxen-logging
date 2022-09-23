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
/// The function lives in the `oxen::log::literals` namespace; you should use it via:
///
///     #include <oxen/log/format.hpp>
///     // ...
///     using namespace oxen::log::literals;
///
/// to make it available (it is not included by default from oxen-logging headers).

#include <fmt/core.h>
#include <string_view>

namespace oxen::log {

namespace detail {

    // Internal implementation of _format that holds the format temporarily until the (...) operator
    // is invoked on it.  This object cannot be moved, copied but only used ephemerally in-place.
    struct fmt_wrapper {
      private:
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

}  // namespace detail

inline namespace literals {

    inline detail::fmt_wrapper operator""_format(const char* str, size_t len) {
        return detail::fmt_wrapper{str, len};
    }

}  // namespace literals

}  // namespace oxen::log
