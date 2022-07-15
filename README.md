# Oxen Logging - C++ spdlog helpers

This repository contains code for interacting with spdlog for logging from Oxen C++ program
(oxen-core, oxen-storage-server, lokinet).  It is mostly a small amount of wrappers/functionality
around spdlog that aids how we apply logging in Oxen core programs.

Most notably what this adds around vanilla spdlog is more robust handling of log categories, and
automatic file/line-number handling *without* needing to resort to macros.

# Usage

## Building

Add as a subdirectory from an existing cmake project, and then link the oxen::logging cmake target
into your code.  This also makes available `fmt::fmt`, `fmt::fmt-header-only`, and `spdlog::spdlog`
targets from those projects that you can link to if you want to use fmt or spdlog somewhere that
doesn't depend on oxen::logging.

There are a few cmake settings you may wish to set; see below for details.

## Coding

### Simple example

Logging from code is intended to follow this pattern:

```C++
#include <oxen/log.hpp>

namespace somewhere {

namespace log = oxen::log;

static auto log_cat = log::Cat("xyz");

void f() {
    log::info(log_cat, "{} - {} = {}", 66, 24, 42);
    log::critical(log_cat, "Oh no!");
}

}
```

In addition to `info` and `critical`, you can also log at `trace`, `debug`, `info`, `warning`, and
`error` levels in the same way.  There are also upper-case aliases (`Trace`, `Debug`, etc.) if that
better fits the coding style.

### Initializing

Before any logging actually appears the logger needs to be told where to log, typically as early as
possible in the code, via a call to `oxen::log::add_sink`.  This is also a good place to set the
initial, default log level for early log statements as well:

```C++
#include <oxen/log.hpp>

int main() {
    oxen::log::add_sink(oxen::log::Type::Print, "stdout");
    oxen::log::reset_level(oxen::log::Level::warn);
    // ...
}
```

`add_sink` takes other argument values to log to a file, system logger (e.g. syslog, or equivalent
on other platforms), stderr, etc.  See the header documentation.  You can also call `add_sink`
multiple times if you want copies of the logs to go to multiple places, and `clear_sinks()` if you
want to reset the output location (for example, to clear an initial print logger and set up file
logging after loading a config file).

### Log categories

Oxen logger is fundamentally designed around using logging categories, which different categories
both provide some detail in the log output, and allow configuring logging to have different levels
at different categories.

Categorized loggers can be created on the fly via a call to `log::Cat("name")`; this returns a proxy
object that delays creation of the logger until it is first used to log, and as such is suitable for
(and intended for) static initialization.  It is perfectly acceptable to have multiple `log::Cat()`
calls with the same name: when each proxy instance is actually initialized there will only be one
underlying logger per name.

The `log::Cat("name")` lookup is, however, moderately expensive as it requires a mutex and an
`std::unordered_map` lookup whenever it is called.  This is perfectly fine when a logging statement
is encountered rarely, but could be a noticeable penalty in a hot loop.  As such it is generally
preferred to use a static variable with appropriate scope, for instance by adding:

```C++
static auto log_cat = log::Cat("flowers");
```

at the top of a `.cpp` file.  This `log_cat` variable is then passed as the first argument to a
log::info, log::debug, etc. statement to specify which named logging category the output goes to.

### Log levels

Each category has its own log level, which you can alter by calling `log::set_level`, which takes
either a category logger or category name as first argument:

```C++
log::set_level(log_cat, log::Level::debug);
log::set_level("p2p", log::Level::warning);
```

You can set the log level of all current categories and future category loggers with:

```C++
log::reset_level(log::Level::debug);
log::set_level_default(log::Level::warning);
```

The first affects all current categories and will be used for any new categories (i.e. categories
that haven't been initialized yet); the latter is only used for new categories but leaves existing
category logger log levels untouched.

## CMake Settings

Generally you should set these using `set(OXEN_LOGGING_WHATEVER somevalue CACHE INTERNAL "")` before
adding the oxen-logging subdirectory in your cmake parent project.

### `OXEN_LOGGING_SOURCE_ROOT`

If set to the root path of your source files then that path will be stripped from the filename
source locations that get logged.

### fmt::fmt, spdlog::spdlog

If these targets already exist then they are used rather than loading fmt/spdlog from the bundled
submodules.

### `OXEN_LOGGING_FORCE_SUBMODULES`

If this is set to ON then we always load fmt/spdlog from submodules (assuming the targets are not
already present, see above).  If off then we first look for a suitable system version to link
against, loading from submodules if we don't find one.

### `BUILD_SHARED_LIBS`

This is not directly used by oxen-logger (which always builds a static library), but *is* recognized
by the fmt/spdlog submodules.

### `OXEN_LOGGING_RELEASE_TRACE`

By default all oxen::log::trace() statements become no-ops when building in release mode (i.e. with
NDEBUG defined).  If you want Trace statements to be usable in a release build then you must set
this to ON.

### `OXEN_LOGGING_FMT_HEADER_ONLY`, `OXEN_LOGGING_SPDLOG_HEADER_ONLY`

If enabled (default is off) then these use fmt and spdlog, respectively, in header-only mode rather
than compiled mode (by using the `fmt::fmt-header-only` target instead of `fmt::fmt` for fmt, and
`spdlog::spdlog_header_only` instead of `spdlog::spdlog` for spdlog).  This fmt version also sets
the spdlog option to also use fmt in header-only mode, when using the spdlog submodule.

Turning this on is a good idea for project with a small number of compilation units, but will likely
make large projects build considerably slower.
