#include <oxen/log/type.hpp>
#include <oxen/log/internal.hpp>
#include <oxen/log/format.hpp>
#include <fmt/core.h>

namespace oxen::log {

Type type_from_string(std::string type) {
    detail::make_lc(type);

    if (type == "file")
        return Type::File;
    if (type == "print")
        return Type::Print;
    if (type == "system" || type == "syslog")
        return Type::System;

    throw std::invalid_argument{"Invalid log type '{}'"_format(type)};
}

std::string_view to_string(Type type) {
    switch (type) {
        case Type::File: return "file";
        case Type::Print: return "print";
        case Type::System: return "system";
    }
    return "unknown";
}

}  // namespace oxen::log
