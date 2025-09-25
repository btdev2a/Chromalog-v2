# chromalog

a modern c++ logging library with colored output and simple formatting

## features

- color coded log levels for better readability
- precise timestamps with millisecond precision
- easy message formatting with `{}` placeholders
- single header file implementation
- thread safe logging with mutex protection
- file logging support
- logger registry system
- auto-detection of terminal color support
- thread id tracking in log output
- support for linux, windows, and mac
- lightweight and fast

## what's new

### thread safety
- full thread safe implementation with mutexes
- safe concurrent logging from multiple threads

### file logging
- log to files with `set_log_file(path)`
- automatic color stripping for file output
- thread safe file operations with proper cleanup

### logger registry
- named logger management system
- `get_logger(name)` to retrieve loggers by name
- `get_default_logger()` for global logging
- unknown logger fallback with random suffixes
- `get_logger_names()` to list all registered loggers

### enhanced output
- thread id included in log format
- better aligned output formatting
- cross-platform terminal color detection
- improved timestamp precision

### memory safety
- proper raii with destructor cleanup
- move semantics support
- smart pointer usage throughout
- disabled copy operations to prevent issues

### additional features
- `shutdown_all_loggers()` for clean shutdown
- `set_global_level()` to change all loggers at once
- auto-detect colors based on terminal capability
- better cross-platform compatibility

## installation

download `chromalog.h` and include it in your project:

```cpp
#include "chromalog.h"
```

## getting started

```cpp
#include <iostream>
#include "chromalog.h"

int main() {
    // basic; create logger
    auto logger = create_logger("veryproniceapplication", chromalog::debug, true);
    logger->log_debug("Bananas are berries, but strawberries are not.");
    logger->log_info("The Eiffel Tower can grow about 15 cm taller in summer.");
    logger->log_warning("Sharks existed before trees.");
    logger->log_error("Octopuses have three hearts.");
    logger->log_critical("Honey never spoils — 3000-year-old honey is still edible.");
    auto db = create_logger("db", chromalog::info, true);
    auto test = create_logger("test", chromalog::warning, true);

    db->log_info("Sloths can hold their breath longer than dolphins.");
    test->log_warning("A day on Venus is longer than a year on Venus.");

    logger->set_log_file("app.log");
    logger->log_info("Random fact: Wombat poop is cube-shaped.");

    return 0;
}
```

## logging levels

chromalog supports the following log levels in order of severity:

- `trace` - very detailed diagnostic info
- `debug` - detailed diagnostic info for debugging
- `info` - general informational messages
- `warning` - warning about potential issues
- `error` - error conditions that do not stop execution
- `critical` - critical errors that may stop execution
- `off` - disable all logging

## configuration

```cpp
auto logger = create_logger("app", chromalog::debug, true);
logger->set_level(chromalog::warning);
logger->set_colors(false);
logger->set_auto_detect_colors(true);  // terminal support
logger->set_log_file("app.log");  // file log
```

## basic usage

### create logger

```cpp
// logger with default settings
auto logger = create_logger("name");

// custom level w colors
auto logger = create_logger("name", chromalog::debug, true);

// default logger
auto default_logger = chromalog::get_default_logger();

// gets logger by its name
auto existing = chromalog::get_logger("existing_name");
```

### log messages

```cpp
logger->log_trace("detailed info: {}", value);
logger->log_debug("debug info: {}", value);
logger->log_info("information: {}", value);
logger->log_warning("warning: {}", message);
logger->log_error("error: {}", message);
logger->log_critical("critical: {}", message);
```

### settings

```cpp
logger->set_level(level);              // set minimum log level
logger->set_colors(enable);            // enable/disable colors
logger->set_auto_detect_colors(auto);  // auto-detect terminal support
logger->set_log_file("path.log");      // enable file logging
logger->close_log_file();              // close file logging
logger->get_level();                   // get current log level
logger->get_name();                    // get logger name
```

### global operations

```cpp
chromalog::set_global_level(level);    // set level for all loggers
chromalog::shutdown_all_loggers();     // clean shutdown
chromalog::get_logger_names();         // get list of logger names
```

## log levels

log levels follow this hierarchy:
`trace < debug < info < warning < error < critical < off`

setting a log level will display that level and all levels above it....

## quick logging

for quick logging without creating a logger:

```cpp
get_logger().log_info("message: {}", value);
```

## file logging

enable file logging for any logger:

```cpp
auto logger = create_logger("app");
logger->set_log_file("application.log");
logger->log_info("this appears in both console and file");
```

files automatically strip color codes and include full formatting.

## thread safety

all logging operations are thread safe. multiple threads can safely:
- log to the same logger simultaneously
- create and manage different loggers
- access the logger registry

## platform support

- **linux/macos**: colors work in all standard terminals
- **windows**: colors work in windows 10+ cmd, powershell
- **auto detection**: automatically disables colors when output is redirected

## credits

created by bitflags

## license

mit license - see license file for details.

if you encounter any issues, feel free to add me on discord - oi2qq

thanks for using chromalog!
