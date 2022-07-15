#include <oxen/log/type.hpp>

namespace oxen::log {

Type type_from_string(std::string_view type) {
    if (type == "unknown")
        return Type::Unknown;
    if (type == "file")
        return Type::File;
    if (type == "print")
        return Type::Print;
    if (type == "system" || type == "syslog")
        return Type::System;

    return Type::Unknown;
}

std::string_view to_string(Type type) {
    switch (type) {
        case Type::Unknown: return "unknown";
        case Type::File: return "file";
        case Type::Print: return "print";
        case Type::System: return "system";
    }
    return "";
}

}  // namespace oxen::log
