// Ouzel by Elviss Strazdins

#ifndef OUZEL_MATH_BOX_HPP
#define OUZEL_MATH_BOX_HPP

#include <cstdint>
#include <limits>
#include "Size.hpp"
#include "Vector.hpp"

namespace ouzel
{
    template <typename T, std::size_t n> class Box final
    {
    public:
        Vector<T, n> min;
        Vector<T, n> max;

        Box() noexcept
        {
            for (T& v : min.v)
                v = std::numeric_limits<T>::max();
            for (T& v : max.v)
                v = std::numeric_limits<T>::lowest();
        }

        constexpr Box(const Vector<T, n>& initMin, const Vector<T, n>& initMax) noexcept:
            min{initMin}, max{initMax}
        {
        }

        template <std::size_t N2>
        explicit Box(const Box<T, N2>& box) noexcept:
            min{Vector<T, n>{box.min}}, max{Vector<T, n>{box.max}}
        {
        }

        auto getCenter() const noexcept
        {
            return (min + max) / T(2);
        }

        auto intersects(const Box& aabb) const noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                if (aabb.min.v[0] > max.v[0]) return false;
                if (aabb.max.v[0] < min.v[0]) return false;
            }

            return true;
        }

        auto containsPoint(const Vector<T, n>& point) const noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                if (point.v[i] < min.v[i]) return false;
                if (point.v[i] > max.v[i]) return false;
            }

            return true;
        }

        void merge(const Box& box) noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                min.v[i] = std::min(min.v[i], box.min.v[i]);
                max.v[i] = std::max(max.v[i], box.max.v[i]);
            }
        }

        void reset() noexcept
        {
            for (T& v : min.v)
                v = std::numeric_limits<T>::max();
            for (T& v : max.v)
                v = std::numeric_limits<T>::lowest();
        }

        auto isEmpty() const noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
                if (min.v[i] > max.v[i])
                    return true;
            return false;
        }

        void insertPoint(const Vector<T, n>& point) noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                if (point.v[i] < min.v[i]) min.v[i] = point.v[i];
                if (point.v[i] > max.v[i]) max.v[i] = point.v[i];
            }
        }

        constexpr auto operator+(const Vector<T, n>& v) const noexcept
        {
            return Box{min + v, max + v};
        }

        auto& operator+=(const Vector<T, n>& v) noexcept
        {
            min += v;
            max += v;
            return *this;
        }

        constexpr auto operator-(const Vector<T, n>& v) const noexcept
        {
            return Box{min - v, max - v};
        }

        auto& operator-=(const Vector<T, n>& v) noexcept
        {
            min -= v;
            max -= v;
            return *this;
        }

        auto getSize() const noexcept
        {
            return Size<T, n>{max - min};
        }
    };
}

#endif // OUZEL_MATH_BOX_HPP
