// Ouzel by Elviss Strazdins

#ifndef OUZEL_UTILS_UTILS_HPP
#define OUZEL_UTILS_UTILS_HPP

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace ouzel
{
    template <class T>
    auto getVectorSize(const T& vec) noexcept
    {
        return sizeof(typename T::value_type) * vec.size();
    }

    template <typename T, typename Iterator>
    std::enable_if_t<std::is_unsigned_v<T>, T> decodeBigEndian(Iterator iterator) noexcept
    {
        T result = T(0);

        for (std::size_t i = 0; i < sizeof(T); ++i, ++iterator)
            result |= static_cast<T>(static_cast<std::uint8_t>(*iterator) << ((sizeof(T) - i - 1) * 8));

        return result;
    }

    template <typename T, typename Iterator>
    std::enable_if_t<std::is_unsigned_v<T>, T> decodeLittleEndian(Iterator iterator) noexcept
    {
        T result = T(0);

        for (std::size_t i = 0; i < sizeof(T); ++i, ++iterator)
            result |= static_cast<T>(static_cast<std::uint8_t>(*iterator) << (i * 8));

        return result;
    }

    template <typename T, std::enable_if_t<std::is_unsigned_v<T>>* = nullptr>
    void encodeBigEndian(std::uint8_t* buffer, const T value) noexcept
    {
        for (std::size_t i = 0; i < sizeof(T); ++i)
            buffer[i] = static_cast<std::uint8_t>(value >> ((sizeof(T) - i - 1) * 8));
    }

    template <typename T, std::enable_if_t<std::is_unsigned_v<T>>* = nullptr>
    void encodeLittleEndian(std::uint8_t* buffer, const T value) noexcept
    {
        for (std::size_t i = 0; i < sizeof(T); ++i)
            buffer[i] = static_cast<std::uint8_t>(value >> (i * 8));
    }

    inline auto explodeString(const std::string& str, char delimiter = ' ')
    {
        std::vector<std::string> result;
        for (std::size_t position = 0, beginPosition = 0; position != std::string::npos; beginPosition = position + 1)
        {
            position = str.find(delimiter, beginPosition);
            const std::size_t endPosition = (position == std::string::npos) ? str.size() : position;
            if (endPosition != beginPosition)
                result.push_back(str.substr(beginPosition, endPosition - beginPosition));
        }
        return result;
    }
}

#endif // OUZEL_UTILS_UTILS_HPP
