// Ouzel by Elviss Strazdins

#ifndef OUZEL_UTILS_LOG_HPP
#define OUZEL_UTILS_LOG_HPP

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <type_traits>
#include "../math/Matrix.hpp"
#include "../math/Quaternion.hpp"
#include "../math/Size.hpp"
#include "../math/Vector.hpp"
#include "../storage/Path.hpp"
#include "Bit.hpp"

namespace ouzel
{
    inline std::string toString(const bool val)
    {
        return val ? "true" : "false";
    }

    inline std::string toString(const char val)
    {
        return std::to_string(val);
    }

    inline std::string toString(const std::uint8_t val)
    {
        constexpr char digits[] = "0123456789abcdef";
        std::string s;
        s.push_back(digits[(val >> 4) & 0x0F]);
        s.push_back(digits[(val >> 0) & 0x0F]);
        return s;
    }

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T> &&
        !std::is_same_v<T, bool> &&
        !std::is_same_v<T, std::uint8_t>>* = nullptr>
    std::string toString(const T val)
    {
        return std::to_string(val);
    }

    inline std::string toString(const std::string& val)
    {
        return val;
    }

    inline std::string toString(const char* val)
    {
        return val;
    }

    template <typename T, std::enable_if_t<!std::is_same_v<T, char>>* = nullptr>
    std::string toString(const T* val)
    {
        constexpr char digits[] = "0123456789abcdef";

        const auto ptrValue = bitCast<std::uintptr_t>(val);

        std::string s;
        for (std::size_t i = 0; i < sizeof(val) * 2; ++i)
            s.push_back(digits[(ptrValue >> (sizeof(ptrValue) * 2 - i - 1) * 4) & 0x0F]);
        return s;
    }

    template<typename T, typename = void>
    struct IsContainer: std::false_type {};

    template<typename T>
    struct IsContainer<T,
        std::void_t<
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
        >>: std::true_type {};

    template<typename T>
    inline constexpr bool isContainer = IsContainer<T>::value;

    template <typename T, std::enable_if_t<isContainer<T> || std::is_array_v<T>>* = nullptr>
    std::string toString(const T& val)
    {
        std::string s;
        bool first = true;
        for (const auto& i : val)
        {
            if (!first) s += ", ";
            first = false;
            s += toString(i);
        }
        return s;
    }

    class Logger;

    class Log final
    {
    public:
        enum class Level
        {
            off,
            error,
            warning,
            info,
            all
        };

        explicit Log(const Logger& initLogger, Level initLevel = Level::info):
            logger{initLogger}, level{initLevel}
        {
        }

        Log(const Log& other):
            logger{other.logger},
            level{other.level},
            s{other.s}
        {
        }

        Log(Log&& other) noexcept:
            logger{other.logger},
            level{other.level},
            s{std::move(other.s)}
        {
            other.level = Level::info;
        }

        Log& operator=(const Log& other)
        {
            if (&other == this) return *this;

            level = other.level;
            s = other.s;

            return *this;
        }

        Log& operator=(Log&& other) noexcept
        {
            if (&other == this) return *this;

            level = other.level;
            other.level = Level::info;
            s = std::move(other.s);

            return *this;
        }

        ~Log();

        template <class T>
        Log& operator<<(T&& val)
        {
            s += toString(val);
            return *this;
        }

    private:
        const Logger& logger;
        Level level = Level::info;
        std::string s;
    };

    class Logger final
    {
    public:
        explicit Logger(Log::Level initThreshold = Log::Level::all):
            threshold{initThreshold}
        {
        }

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        Log operator()(const Log::Level level = Log::Level::info) const
        {
            return Log{*this, level};
        }

        void log(const std::string& str, const Log::Level level = Log::Level::info) const
        {
            if (level <= threshold)
            {
#ifndef __EMSCRIPTEN__
                std::scoped_lock lock{logMutex};
#endif
                logString(str, level);
            }
        }

    private:
        static void logString(const std::string& str, const Log::Level level = Log::Level::info);

#ifdef DEBUG
        std::atomic<Log::Level> threshold{Log::Level::all};
#else
        std::atomic<Log::Level> threshold{Log::Level::info};
#endif

#ifndef __EMSCRIPTEN__
        mutable std::mutex logMutex;
#endif
    };

    inline Log::~Log()
    {
        if (!s.empty())
            logger.log(s, level);
    }

    template <class T, std::size_t rows, std::size_t cols>
    std::string toString(const math::Matrix<T, rows, cols>& val)
    {
        bool first = true;

        std::string s;
        for (const T c : val.m)
        {
            if (!first) s += ",";
            first = false;
            s += std::to_string(c);
        }
        return s;
    }

    template <class T, std::size_t dims>
    std::string toString(const math::Size<T, dims>& val)
    {
        bool first = true;

        std::string s;
        for (const T c : val.v)
        {
            if (!first) s += ",";
            first = false;
            s += std::to_string(c);
        }
        return s;
    }

    template <class T, std::size_t dims>
    std::string toString(const math::Vector<T, dims>& val)
    {
        bool first = true;

        std::string s;
        for (const T c : val.v)
        {
            if (!first) s += ",";
            first = false;
            s += std::to_string(c);
        }
        return s;
    }

    template <typename T>
    std::string toString(const math::Quaternion<T>& val)
    {
        return std::to_string(val.v[0]) + "," +
            std::to_string(val.v[1]) + "," +
            std::to_string(val.v[2]) + "," +
            std::to_string(val.v[3]);
    }

    inline std::string toString(const storage::Path& val)
    {
        return val.getGeneric();
    }

    extern Logger log;
}

#endif // OUZEL_UTILS_LOG_HPP
