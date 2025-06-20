#include <oxen/log.hpp>
#include <oxen/log/format.hpp>

#include <shared_mutex>
#include <span>
#include <stdexcept>
#include <string_view>

#include <fmt/ranges.h>

#if defined(_WIN32) || defined(WIN32)
#define OXEN_LOGGING_EXPORT __declspec(dllexport)
#else
#define OXEN_LOGGING_EXPORT __attribute__((visibility("default")))
#endif

namespace oxen::log {

static auto logcat = Cat("logging");

using namespace std::literals;

namespace detail {
    OXEN_LOGGING_EXPORT std::unordered_map<std::string, Level>& levels() {
        static std::unordered_map<std::string, Level> levels_impl{
                {"off"s, Level::off},
                {"none"s, Level::off},
                {"disable"s, Level::off},
                {"disabled"s, Level::off},
                {"critical"s, Level::critical},
                {"crit"s, Level::critical},
                {"error"s, Level::err},
                {"err"s, Level::err},
                {"warning"s, Level::warn},
                {"warn"s, Level::warn},
                {"info"s, Level::info},
                {"debug"s, Level::debug},
                {"trace"s, Level::trace},
        };
        return levels_impl;
    }
    OXEN_LOGGING_EXPORT std::shared_mutex& levels_mutex() {
        static std::shared_mutex levels_mutex_impl;
        return levels_mutex_impl;
    }
}  // namespace detail

std::string_view to_string(Level lvl) {
    auto l = spdlog::level::to_string_view(lvl);
    return {l.data(), l.size()};
}

Level level_from_string(std::string level) {
    detail::make_lc(level);

    std::shared_lock lock{detail::levels_mutex()};
    auto& levels = detail::levels();
    if (auto it = levels.find(level); it != levels.end())
        return it->second;

    throw std::invalid_argument{"Invalid log level '{}'"_format(level)};
}

void add_level_compat_string(std::string key, Level level) {
    std::unique_lock lock{detail::levels_mutex()};
    detail::levels()[std::move(key)] = level;
}

static std::vector<std::string_view> split_any(
        std::string_view str, const std::string_view delims, bool trim) {
    std::vector<std::string_view> results;
    for (size_t pos = str.find_first_of(delims); pos != std::string_view::npos;
         pos = str.find_first_of(delims)) {
        if (!trim || !results.empty() || pos > 0)
            results.push_back(str.substr(0, pos));
        size_t until = str.find_first_not_of(delims, pos + 1);
        if (until == std::string_view::npos)
            str.remove_prefix(str.size());
        else
            str.remove_prefix(until);
    }
    if (!trim || str.size())
        results.push_back(str);
    else
        while (!results.empty() && results.back().empty())
            results.pop_back();
    return results;
}

LogCats extract_categories(std::string_view categories) {
    LogCats result;
    for (auto cat : split_any(categories, " ,;", true)) {
        auto pieces = split_any(cat, ":=", true);
        if (pieces.size() < 1 || pieces.size() > 2) {
            error(logcat,
                  "Invalid or unparseable log category/level '{}'; expected 'level' or "
                  "'category=level'",
                  cat);
            continue;
        }
        Level lvl;
        try {
            lvl = level_from_string(std::string{pieces.back()});
        } catch (const std::invalid_argument&) {
            error(logcat, "Invalid log level '{}' in log input '{}'", pieces.back(), cat);
            continue;
        }
        auto cat_name = pieces.size() == 1 ? "*"sv : pieces.front();

        if (cat_name == "*"sv) {
            result.default_level = lvl;
            // Any categories up to this point just get wiped out by the default resetting
            // everything:
            result.cat_levels.clear();
        } else if (auto pos = cat_name.find('*'); pos != std::string_view::npos) {
            auto matches = split_any(cat_name, "*"sv, false);
            for_each_cat_name([&result,
                               lvl,
                               &prefix = matches.front(),
                               suffix = matches.back(),
                               middle = std::span{matches}.subspan(1, matches.size() - 2)](
                                      const std::string& name) {
                std::string_view remaining{name};
                if (!prefix.empty()) {
                    if (!remaining.starts_with(prefix))
                        return;
                    remaining.remove_prefix(prefix.size());
                }
                if (!suffix.empty()) {
                    if (!remaining.ends_with(suffix))
                        return;
                    remaining.remove_suffix(suffix.size());
                }
                for (const auto& m : middle) {
                    if (auto p = remaining.find(m); p != std::string_view::npos)
                        remaining.remove_prefix(p + m.size());
                    else
                        return;
                }

                result.cat_levels[name] = lvl;
            });
        } else {
            result.cat_levels[std::string{pieces.front()}] = lvl;
        }
    }

    return result;
}

std::list<std::string> LogCats::apply(std::function<void(Level)> amend_default) {
    std::list<std::string> applied;
    if (default_level) {
        reset_level(*default_level);
        if (amend_default)
            amend_default(*default_level);
        applied.push_back(fmt::format("*={}", to_string(*default_level)));
    }

    for (const auto& [cat, lvl] : cat_levels) {
        set_level(cat, lvl);
        applied.push_back(fmt::format("{}={}", cat, to_string(lvl)));
    }

    if (!applied.empty())
        info(logcat, "Applied log categories: {}", fmt::join(applied, ", "));

    return applied;
}

}  // namespace oxen::log
