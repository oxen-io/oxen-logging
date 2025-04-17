# This provides the fmt::fmt and spdlog::spdlog targets.  It's used by oxen-logging itself, but can
# also be used by a parent project that skip add_directory(oxen-logging) (typically because the
# parent is loading oxen-logging itself via system library instead of via submodule).

if(NOT OXEN_LOGGING_FORCE_SUBMODULES)
    if(NOT TARGET fmt::fmt)
        find_package(fmt 9.0.0 CONFIG QUIET)
        if(fmt_FOUND)
            message(STATUS "Found fmt ${fmt_VERSION}")
            # Make the target available to the parent project (which is the case if we go by
            # subproject, but isn't for packages we find via find_package).  cmake 3.24+ has a
            # `GLOBAL` flag in the find_package, but we need this to work on older cmakes as well.
            set_target_properties(fmt::fmt PROPERTIES IMPORTED_GLOBAL TRUE)
        else()
            message(STATUS "Did not find suitable fmt; using submodule")
        endif()
    endif()
    set(min_spdlog 1.9.1)
    if(NOT TARGET fmt::fmt OR fmt_VERSION VERSION_GREATER_EQUAL 10.0.0)
        set(min_spdlog 1.12.0)
    endif()

    if(NOT TARGET spdlog::spdlog)
        find_package(spdlog ${min_spdlog} CONFIG QUIET)
        if(spdlog_FOUND)
            message(STATUS "Found spdlog ${spdlog_VERSION}")
            # Make available in parent; see above.
            set_target_properties(spdlog::spdlog PROPERTIES IMPORTED_GLOBAL TRUE)
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
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../fmt)
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
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../spdlog)
endif()
