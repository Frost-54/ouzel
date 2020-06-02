// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_UTILS_LOG_HPP
#define OUZEL_UTILS_LOG_HPP

#include <array>
#include <atomic>
#include <condition_variable>
#include <initializer_list>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "../math/Matrix.hpp"
#include "../math/Quaternion.hpp"
#include "../math/Size.hpp"
#include "../math/Vector.hpp"
#include "../storage/Path.hpp"
#include "Thread.hpp"

namespace ouzel
{
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
            logger(initLogger), level(initLevel)
        {
        }

        Log(const Log& other):
            logger(other.logger),
            level(other.level),
            s(other.s)
        {
        }

        Log(Log&& other) noexcept:
            logger(other.logger),
            level(other.level),
            s(std::move(other.s))
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

        Log& operator<<(const bool val)
        {
            s += val ? "true" : "false";
            return *this;
        }

        Log& operator<<(const uint8_t val)
        {
            constexpr char digits[] = "0123456789abcdef";

            for (std::uint32_t p = 0; p < 2; ++p)
                s.push_back(digits[(val >> (4 - p * 4)) & 0x0F]);

            return *this;
        }

        template <typename T, typename std::enable_if<std::is_arithmetic<T>::value &&
            !std::is_same<T, bool>::value &&
            !std::is_same<T, std::uint8_t>::value>::type* = nullptr>
        Log& operator<<(const T val)
        {
            s += std::to_string(val);
            return *this;
        }

        Log& operator<<(const std::string& val)
        {
            s += val;
            return *this;
        }

        Log& operator<<(const char* val)
        {
            s += val;
            return *this;
        }

        template <typename T, typename std::enable_if<!std::is_same<T, char>::value>::type* = nullptr>
        Log& operator<<(const T* val)
        {
            constexpr char digits[] = "0123456789abcdef";

            std::uintptr_t ptrValue;
            std::memcpy(&ptrValue, &val, sizeof(ptrValue));

            for (std::size_t i = 0; i < sizeof(val) * 2; ++i)
                s.push_back(digits[(ptrValue >> (sizeof(ptrValue) * 2 - i - 1) * 4) & 0x0F]);

            return *this;
        }

        Log& operator<<(const storage::Path& val)
        {
            s += val;
            return *this;
        }

        template <typename T> struct isContainer: std::false_type{};
        template <typename T, std::size_t N> struct isContainer<std::array<T, N>>: std::true_type{};
        template <typename... Args> struct isContainer<std::initializer_list<Args...>>: std::true_type{};
        template <typename... Args> struct isContainer<std::list<Args...>>: std::true_type{};
        template <typename... Args> struct isContainer<std::set<Args...>>: std::true_type{};
        template <typename... Args> struct isContainer<std::vector<Args...>>: std::true_type{};

        template <typename T, typename std::enable_if<isContainer<T>::value>::type* = nullptr>
        Log& operator<<(const T& val)
        {
            bool first = true;

            for (const auto& i : val)
            {
                if (!first) s += ", ";
                first = false;

                operator<<(i);
            }

            return *this;
        }

        template <std::size_t N, std::size_t M, class T>
        Log& operator<<(const Matrix<N, M, T>& val)
        {
            bool first = true;

            for (const T c : val.m)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }

            return *this;
        }

        template <typename T>
        Log& operator<<(const Quaternion<T>& val)
        {
            s += std::to_string(val.v[0]) + "," + std::to_string(val.v[1]) + "," +
                std::to_string(val.v[2]) + "," + std::to_string(val.v[3]);
            return *this;
        }

        template <std::size_t N, class T>
        Log& operator<<(const Size<N, T>& val)
        {
            bool first = true;

            for (const T c : val.v)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }
            return *this;
        }

        template <std::size_t N, class T>
        Log& operator<<(const Vector<N, T>& val)
        {
            bool first = true;

            for (const T c : val.v)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }
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
            threshold(initThreshold)
        {
#if !defined(__EMSCRIPTEN__)
            logThread = Thread(&Logger::logLoop, this);
#endif
        }

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        ~Logger()
        {
#if !defined(__EMSCRIPTEN__)
            std::unique_lock<std::mutex> lock(queueMutex);
            commandQueue.push(Command(Command::Type::quit));
            lock.unlock();
            queueCondition.notify_all();
#endif
        }

        Log log(const Log::Level level = Log::Level::info) const
        {
            return Log(*this, level);
        }

        void log(const std::string& str, const Log::Level level = Log::Level::info) const
        {
            if (level <= threshold)
            {
#if defined(__EMSCRIPTEN__)
                logString(str, level);
#else
                std::unique_lock<std::mutex> lock(queueMutex);
                commandQueue.push(Command(Command::Type::logString, level, str));
                lock.unlock();
                queueCondition.notify_all();
#endif
            }
        }

    private:
        static void logString(const std::string& str, const Log::Level level = Log::Level::info);

#ifdef DEBUG
        std::atomic<Log::Level> threshold{Log::Level::all};
#else
        std::atomic<Log::Level> threshold{Log::Level::info};
#endif

#if !defined(__EMSCRIPTEN__)
        class Command
        {
        public:
            enum class Type
            {
                logString,
                quit
            };

            explicit Command(const Type initType):
                type(initType)
            {
            }

            Command(const Type initType, const Log::Level initLevel,
                    const std::string& initString):
                type(initType),
                level(initLevel),
                str(initString)
            {
            }

            Type type;
            Log::Level level;
            std::string str;
        };

        void logLoop()
        {
            for (;;)
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                while (commandQueue.empty()) queueCondition.wait(lock);
                auto command = std::move(commandQueue.front());
                commandQueue.pop();
                lock.unlock();

                if (command.type == Command::Type::logString)
                    logString(command.str, command.level);
                else if (command.type == Command::Type::quit)
                    break;
            }
        }

        mutable std::condition_variable queueCondition;
        mutable std::mutex queueMutex;
        mutable std::queue<Command> commandQueue;
        Thread logThread;
#endif
    };
}

#endif // OUZEL_UTILS_LOG_HPP
