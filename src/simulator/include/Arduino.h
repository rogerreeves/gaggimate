#ifndef ARDUINO_H
#define ARDUINO_H

#ifdef __cplusplus
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#else
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef F
#define F(x) x
#endif

#ifdef __cplusplus
using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using int8_t = std::int8_t;
using int16_t = std::int16_t;
using int32_t = std::int32_t;
#endif

#ifdef __cplusplus
class String {
  public:
    String() = default;
    String(const char *value) : data_(value ? value : "") {}
    String(const std::string &value) : data_(value) {}
    String(char value) : data_(1, value) {}
    String(int value) : data_(std::to_string(value)) {}
    String(unsigned int value) : data_(std::to_string(value)) {}
    String(long value) : data_(std::to_string(value)) {}
    String(unsigned long value) : data_(std::to_string(value)) {}
    String(float value, int decimals = 2) { format_decimal(value, decimals); }
    String(double value, int decimals = 2) { format_decimal(value, decimals); }

    const char *c_str() const { return data_.c_str(); }
    size_t length() const { return data_.size(); }
    bool isEmpty() const { return data_.empty(); }

    int indexOf(char c, size_t from = 0) const {
        const auto pos = data_.find(c, from);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    int lastIndexOf(char c) const {
        const auto pos = data_.rfind(c);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    String substring(size_t start) const {
        if (start >= data_.size()) {
            return String("");
        }
        return String(data_.substr(start));
    }

    String substring(size_t start, size_t end) const {
        if (start >= data_.size() || end <= start) {
            return String("");
        }
        return String(data_.substr(start, end - start));
    }

    float toFloat() const {
        try {
            return std::stof(data_);
        } catch (...) {
            return 0.0f;
        }
    }

    int toInt() const {
        try {
            return std::stoi(data_);
        } catch (...) {
            return 0;
        }
    }

    double toDouble() const {
        try {
            return std::stod(data_);
        } catch (...) {
            return 0.0;
        }
    }

    String &operator=(const char *value) {
        data_ = value ? value : "";
        return *this;
    }

    String &operator=(const std::string &value) {
        data_ = value;
        return *this;
    }

    String &operator+=(const String &other) {
        data_ += other.data_;
        return *this;
    }

    friend String operator+(const String &lhs, const String &rhs) { return String(lhs.data_ + rhs.data_); }

    friend bool operator==(const String &lhs, const String &rhs) { return lhs.data_ == rhs.data_; }
    friend bool operator!=(const String &lhs, const String &rhs) { return lhs.data_ != rhs.data_; }
    friend bool operator<(const String &lhs, const String &rhs) { return lhs.data_ < rhs.data_; }

    char operator[](size_t index) const { return data_[index]; }

  private:
    std::string data_;

    void format_decimal(double value, int decimals) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(decimals) << value;
        data_ = oss.str();
    }
};

inline unsigned long millis() {
    static const auto start = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count());
}

template <typename T>
inline T max(T a, T b) {
    return std::max(a, b);
}

template <typename T>
inline T min(T a, T b) {
    return std::min(a, b);
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void vTaskDelay(unsigned long ms) {
    delay(ms);
}

#ifndef portTICK_PERIOD_MS
#define portTICK_PERIOD_MS 1
#endif

using xTaskHandle = void *;

#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE 1024
#endif

inline void xTaskCreatePinnedToCore(void (*)(void *), const char *, size_t, void *, int, xTaskHandle *, int) {}

#else
static inline unsigned long millis(void) {
    static unsigned long start_ms = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long now_ms = (unsigned long)(ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL);
    if (start_ms == 0) {
        start_ms = now_ms;
    }
    return now_ms - start_ms;
}

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

static inline void delay(unsigned long ms) {
    struct timespec req;
    req.tv_sec = (time_t)(ms / 1000UL);
    req.tv_nsec = (long)((ms % 1000UL) * 1000000UL);
    nanosleep(&req, NULL);
}

static inline void vTaskDelay(unsigned long ms) {
    delay(ms);
}

#endif

#endif // ARDUINO_H
