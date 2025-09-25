#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <random>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

class chromalog {
public:
    enum level {
        trace = 0,
        debug = 1,
        info = 2,
        warning = 3,
        error = 4,
        critical = 5,
        off = 6
    };

private:
    level log_level;
    std::string logger_name;
    bool enable_colors;
    std::string log_file_path;
    mutable std::mutex log_mutex;
    mutable std::ofstream log_file;
    bool auto_detect_colors;
    static std::unordered_map<std::string, std::shared_ptr<chromalog>> loggers;
    static std::mutex registry_mutex;
    static std::shared_ptr<chromalog> default_logger;
    static const std::string reset_color;
    static const std::string black_color;
    static const std::string red_color;
    static const std::string green_color;
    static const std::string yellow_color;
    static const std::string blue_color;
    static const std::string magenta_color;
    static const std::string cyan_color;
    static const std::string white_color;
    static const std::string bold_style;

    std::string level_to_string(level lvl) const {
        switch (lvl) {
        case trace:    return "TRACE";
        case debug:    return "DEBUG";
        case info:     return "INFO";
        case warning:  return "WARN";
        case error:    return "ERROR";
        case critical: return "CRIT";
        default:       return "UNKNWN";
        }
    }

    std::string level_to_color(level lvl) const {
        if (!should_use_colors()) return "";

        switch (lvl) {
        case trace:    return blue_color;
        case debug:    return cyan_color;
        case info:     return green_color;
        case warning:  return yellow_color;
        case error:    return red_color;
        case critical: return bold_style + red_color;
        default:       return "";
        }
    }

    bool should_use_colors() const {
        if (!enable_colors) return false;
        if (!auto_detect_colors) return true;
        // detect if outputs to terminal..
#ifdef _WIN32
        return isatty(fileno(stdout));
#else
        return isatty(STDOUT_FILENO);
#endif
    }

    std::string get_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm local_time;
#ifdef _WIN32
        localtime_s(&local_time, &time_t);
#else
        localtime_r(&time_t, &local_time);
#endif

        std::stringstream ss;
        ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string get_thread_id() const {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return ss.str();
    }

    void output_log(level lvl, const std::string& msg) {
        if (lvl < log_level) return;

        std::lock_guard<std::mutex> lock(log_mutex);

        std::string color = level_to_color(lvl);
        std::string reset = should_use_colors() ? reset_color : "";
        std::string timestamp = get_timestamp();
        std::string thread_id = get_thread_id();
        std::stringstream formatted_msg;
        formatted_msg << "[" << timestamp << "] "
            << "[" << thread_id.substr(0, 6) << "] "
            << color << "[" << level_to_string(lvl) << "]" << reset << " "
            << "[" << logger_name << "] "
            << color << msg << reset;
        std::string final_msg = formatted_msg.str();
        std::cout << final_msg << std::endl;
        if (log_file.is_open()) {
            std::stringstream file_msg;
            file_msg << "[" << timestamp << "] "
                << "[" << thread_id.substr(0, 6) << "] "
                << "[" << level_to_string(lvl) << "] "
                << "[" << logger_name << "] "
                << msg;
            log_file << file_msg.str() << std::endl;
            log_file.flush();
        }
    }

    template<typename T>
    std::string to_string_safe(const T& value) const {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    std::string format_message(const std::string& format_str) const {
        return format_str;
    }

    template<typename T, typename... Args>
    std::string format_message(const std::string& format_str, const T& t, const Args&... args) const {
        std::string result = format_str;
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, to_string_safe(t));
            return format_message(result, args...);
        }
        return result;
    }

    static std::string generate_random_suffix() {
        static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

        std::string result;
        for (int i = 0; i < 8; ++i) {
            result += charset[dis(gen)];
        }
        return result;
    }

public:
    chromalog(const std::string& name, level lvl = info, bool colors = true)
        : logger_name(name), log_level(lvl), enable_colors(colors), auto_detect_colors(true) {
    }

    ~chromalog() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    chromalog(const chromalog&) = delete;
    chromalog& operator=(const chromalog&) = delete;
    chromalog(chromalog&& other) noexcept {
        std::lock_guard<std::mutex> lock1(log_mutex);
        std::lock_guard<std::mutex> lock2(other.log_mutex);

        logger_name = std::move(other.logger_name);
        log_level = other.log_level;
        enable_colors = other.enable_colors;
        auto_detect_colors = other.auto_detect_colors;
        log_file_path = std::move(other.log_file_path);
        log_file = std::move(other.log_file);
    }

    chromalog& operator=(chromalog&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock1(log_mutex);
            std::lock_guard<std::mutex> lock2(other.log_mutex);

            logger_name = std::move(other.logger_name);
            log_level = other.log_level;
            enable_colors = other.enable_colors;
            auto_detect_colors = other.auto_detect_colors;
            log_file_path = std::move(other.log_file_path);
            log_file = std::move(other.log_file);
        }
        return *this;
    }

    void set_level(level lvl) {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_level = lvl;
    }

    level get_level() const {
        std::lock_guard<std::mutex> lock(log_mutex);
        return log_level;
    }

    void set_colors(bool colors) {
        std::lock_guard<std::mutex> lock(log_mutex);
        enable_colors = colors;
    }

    void set_auto_detect_colors(bool auto_detect) {
        std::lock_guard<std::mutex> lock(log_mutex);
        auto_detect_colors = auto_detect;
    }

    bool set_log_file(const std::string& file_path) {
        std::lock_guard<std::mutex> lock(log_mutex);

        if (log_file.is_open()) {
            log_file.close();
        }

        log_file.open(file_path, std::ios::app);
        if (log_file.is_open()) {
            log_file_path = file_path;
            return true;
        }
        return false;
    }

    void close_log_file() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) {
            log_file.close();
        }
        log_file_path.clear();
    }

    std::string get_name() const {
        std::lock_guard<std::mutex> lock(log_mutex);
        return logger_name;
    }

    template<typename... Args>
    void log_trace(const std::string& format_str, const Args&... args) {
        output_log(trace, format_message(format_str, args...));
    }

    template<typename... Args>
    void log_debug(const std::string& format_str, const Args&... args) {
        output_log(debug, format_message(format_str, args...));
    }

    template<typename... Args>
    void log_info(const std::string& format_str, const Args&... args) {
        output_log(info, format_message(format_str, args...));
    }

    template<typename... Args>
    void log_warning(const std::string& format_str, const Args&... args) {
        output_log(warning, format_message(format_str, args...));
    }

    template<typename... Args>
    void log_error(const std::string& format_str, const Args&... args) {
        output_log(error, format_message(format_str, args...));
    }

    template<typename... Args>
    void log_critical(const std::string& format_str, const Args&... args) {
        output_log(critical, format_message(format_str, args...));
    }
    static std::shared_ptr<chromalog> get_default_logger() {
        std::lock_guard<std::mutex> lock(registry_mutex);
        if (!default_logger) {
            default_logger = std::make_shared<chromalog>("default", info, true);
        }
        return default_logger;
    }

    static std::shared_ptr<chromalog> get_logger(const std::string& name) {
        std::lock_guard<std::mutex> lock(registry_mutex);

        auto it = loggers.find(name);
        if (it != loggers.end()) {
            return it->second;
        }
        std::string unknown_name = "UNKNOWN_" + name + "_" + generate_random_suffix();
        auto unknown_logger = std::make_shared<chromalog>(unknown_name, info, true);
        // not addd unknown loggers to the registry uhm
        return unknown_logger;
    }

    static std::shared_ptr<chromalog> create_logger(const std::string& name,
        level lvl = info,
        bool colors = true) {
        std::lock_guard<std::mutex> lock(registry_mutex);

        auto logger = std::make_shared<chromalog>(name, lvl, colors);
        loggers[name] = logger;
        return logger;
    }

    static void register_logger(std::shared_ptr<chromalog> logger) {
        if (!logger) return;

        std::lock_guard<std::mutex> lock(registry_mutex);
        loggers[logger->get_name()] = logger;
    }

    static void unregister_logger(const std::string& name) {
        std::lock_guard<std::mutex> lock(registry_mutex);
        loggers.erase(name);
    }

    static std::vector<std::string> get_logger_names() {
        std::lock_guard<std::mutex> lock(registry_mutex);
        std::vector<std::string> names;
        names.reserve(loggers.size());
        for (const auto& pair : loggers) {
            names.push_back(pair.first);
        }
        return names;
    }

    static void set_global_level(level lvl) {
        std::lock_guard<std::mutex> lock(registry_mutex);
        for (auto& pair : loggers) {
            if (pair.second) {
                pair.second->set_level(lvl);
            }
        }
        if (default_logger) {
            default_logger->set_level(lvl);
        }
    }

    static void shutdown_all_loggers() {
        std::lock_guard<std::mutex> lock(registry_mutex);
        for (auto& pair : loggers) {
            if (pair.second) {
                pair.second->close_log_file();
            }
        }
        if (default_logger) {
            default_logger->close_log_file();
        }
        loggers.clear();
        default_logger.reset();
    }
};
const std::string chromalog::reset_color = "\033[0m";
const std::string chromalog::black_color = "\033[30m";
const std::string chromalog::red_color = "\033[31m";
const std::string chromalog::green_color = "\033[32m";
const std::string chromalog::yellow_color = "\033[33m";
const std::string chromalog::blue_color = "\033[34m";
const std::string chromalog::magenta_color = "\033[35m";
const std::string chromalog::cyan_color = "\033[36m";
const std::string chromalog::white_color = "\033[37m";
const std::string chromalog::bold_style = "\033[1m";

std::unordered_map<std::string, std::shared_ptr<chromalog>> chromalog::loggers;
std::mutex chromalog::registry_mutex;
std::shared_ptr<chromalog> chromalog::default_logger;
inline chromalog& get_logger() {
    static auto logger = chromalog::get_default_logger();
    return *logger;
}
inline std::shared_ptr<chromalog> create_logger(const std::string& name,
    chromalog::level lvl = chromalog::info,
    bool colors = true) {
    return chromalog::create_logger(name, lvl, colors);
}                       
