#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace workbench {

// High-resolution wall-clock timer
class Timer {
public:
    Timer() : start_time_(std::chrono::high_resolution_clock::now()) {}

    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    double elapsed_ms() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_time_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};

// Custom exception types for actionable, descriptive error messages
class WorkbenchException : public std::runtime_error {
public:
    explicit WorkbenchException(const std::string& msg) : std::runtime_error(msg) {}
};

class ParseException : public WorkbenchException {
public:
    explicit ParseException(const std::string& msg) : WorkbenchException("Parse Error: " + msg) {}
};

class ValidationException : public WorkbenchException {
public:
    explicit ValidationException(const std::string& msg) : WorkbenchException("Validation Error: " + msg) {}
};

class HardwareException : public WorkbenchException {
public:
    explicit HardwareException(const std::string& msg) : WorkbenchException("Hardware Interface Error: " + msg) {}
};

class OperationCancelledException : public WorkbenchException {
public:
    explicit OperationCancelledException(const std::string& msg = "Operation cancelled by user.")
        : WorkbenchException(msg) {}
};

// Minimal lightweight JSON builder helper for C++17 with zero third-party dependencies
class JsonHelper {
public:
    static std::string escape(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
                case '"': o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\b': o << "\\b"; break;
                case '\f': o << "\\f"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) <= 0x1f) {
                        o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        o << c;
                    }
            }
        }
        return o.str();
    }

    static std::string error_response(const std::string& message) {
        return "{\"status\":\"error\",\"message\":\"" + escape(message) + "\"}";
    }

    static std::string cancelled_response(const std::string& message = "Operation cancelled by user.") {
        return "{\"status\":\"cancelled\",\"message\":\"" + escape(message) + "\"}";
    }
};

} // namespace workbench
