cmake_minimum_required(VERSION 3.10...3.23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(oxen-logging VERSION 1.0.0 LANGUAGES CXX)

set(OXEN_LOGGING_SOURCE_ROOT "" CACHE PATH "Base path to strip from log message filenames")
option(OXEN_LOGGING_FORCE_SUBMODULES "Force use of the bundled fmt/spdlog rather than looking for system packages" OFF)
option(OXEN_LOGGING_RELEASE_TRACE "Enable trace logging in release builds" OFF)
option(OXEN_LOGGING_FMT_HEADER_ONLY "Use fmt in header-only mode" OFF)
option(OXEN_LOGGING_SPDLOG_HEADER_ONLY "Use spdlog in header-only mode" OFF)

if(NOT OXEN_LOGGING_FORCE_SUBMODULES)
    if(NOT TARGET fmt::fmt)
        find_package(fmt 9.0.0 CONFIG QUIET)
        if(fmt_FOUND)
            message(STATUS "Found fmt ${fmt_VERSION}")
        else()
            message(STATUS "Did not find suitable fmt; using submodule")
        endif()
    endif()
    if(NOT TARGET spdlog::spdlog)
        find_package(spdlog 1.9.1 CONFIG QUIET)
        if(spdlog_FOUND)
            message(STATUS "Found spdlog ${spdlog_VERSION}")
        else()
            message(STATUS "Did not find suitable spdlog; using submodule")
        endif()
    endif()
endif()

set(OXEN_LOGGING_FMT_TARGET fmt::fmt)
set(OXEN_LOGGING_SPDLOG_TARGET spdlog::spdlog)

if(NOT TARGET fmt::fmt)
    if(OXEN_LOGGING_FMT_HEADER_ONLY)
        set(OXEN_LOGGING_FMT_TARGET fmt::fmt-header-only)
    endif()
    add_subdirectory(fmt)
endif()

if(NOT TARGET spdlog::spdlog)
    if(OXEN_LOGGING_FMT_HEADER_ONLY)
        set(SPDLOG_FMT_EXTERNAL_HO ON CACHE INTERNAL "")
    else()
        set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")
    endif()
    if(OXEN_LOGGING_SPDLOG_HEADER_ONLY)
        set(OXEN_LOGGING_SPDLOG_TARGET spdlog::spdlog_header_only)
    endif()
    add_subdirectory(spdlog)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(oxen-logging STATIC
    src/catlogger.cpp
    src/level.cpp
    src/log.cpp
    src/type.cpp
)
target_include_directories(oxen-logging PUBLIC include)
target_link_libraries(oxen-logging PUBLIC ${OXEN_LOGGING_FMT_TARGET} ${OXEN_LOGGING_SPDLOG_TARGET})
target_compile_features(oxen-logging PUBLIC cxx_std_17)

if(OXEN_LOGGING_SOURCE_ROOT)
    message(STATUS "Stripping source root ${OXEN_LOGGING_SOURCE_ROOT} from log paths")
    target_compile_definitions(oxen-logging PUBLIC "OXEN_LOGGING_SOURCE_ROOT=\"${OXEN_LOGGING_SOURCE_ROOT}\"")
else()
    message(STATUS "Source root log path stripping disabled")
endif()

if (OXEN_LOGGING_RELEASE_TRACE)
    target_compile_definitions(oxen-logging PUBLIC OXEN_LOGGING_RELEASE_TRACE)
endif()

add_library(oxen::logging ALIAS oxen-logging)
