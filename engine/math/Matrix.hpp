// Ouzel by Elviss Strazdins

#ifndef OUZEL_MATH_MATRIX_HPP
#define OUZEL_MATH_MATRIX_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include "Constants.hpp"
#include "ConvexVolume.hpp"
#include "Plane.hpp"
#include "Quaternion.hpp"
#include "Simd.hpp"
#include "Vector.hpp"

namespace ouzel::math
{
    template <typename T, std::size_t rows, std::size_t cols = rows> class Matrix final
    {
    public:
#if defined(OUZEL_SIMD_SSE) || defined(OUZEL_SIMD_NEON)
        alignas(std::is_same_v<T, float> && rows == 4 && cols == 4 ? cols * sizeof(T) : alignof(T))
#endif
        std::array<T, cols * rows> m; // row-major matrix (transformation is pre-multiplying)

        [[nodiscard]] auto operator[](const std::size_t row) noexcept { return &m[row * cols]; }
        [[nodiscard]] constexpr auto operator[](const std::size_t row) const noexcept { return &m[row * cols]; }

        [[nodiscard]] constexpr auto isIdentity() const noexcept
        {
            if constexpr (cols != rows) return false;

            for (std::size_t i = 0; i < rows; ++i)
                for (std::size_t j = 0; j < cols; ++j)
                    if (m[i * cols + j] != (i == j ? T(1) : T(0)))
                        return false;
            return true;
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformPoint(math::Vector<T, 3>& point) const noexcept
        {
            transformVector(point.v[0], point.v[1], point.v[2], T(1), point);
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformPoint(const math::Vector<T, 3>& point, math::Vector<T, 3>& dst) const noexcept
        {
            transformVector(point.v[0], point.v[1], point.v[2], T(1), dst);
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformVector(math::Vector<T, 3>& v) const noexcept
        {
            math::Vector<T, 4> t;
            transformVector(math::Vector<T, 4>{v.v[0], v.v[1], v.v[2], T(0)}, t);
            v = math::Vector<T, 3>{t.v[0], t.v[1], t.v[2]};
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformVector(const math::Vector<T, 3>& v, math::Vector<T, 3>& dst) const noexcept
        {
            transformVector(v.v[0], v.v[1], v.v[2], T(0), dst);
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformVector(const T x, const T y, const T z, const T w,
                             math::Vector<T, 3>& dst) const noexcept
        {
            math::Vector<T, 4> t;
            transformVector(math::Vector<T, 4>{x, y, z, w}, t);
            dst = math::Vector<T, 3>{t.v[0], t.v[1], t.v[2]};
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        void transformVector(math::Vector<T, 4>& v) const noexcept
        {
            transformVector(v, v);
        }

        void transformVector(const math::Vector<T, 4>& v, math::Vector<T, 4>& dst) const noexcept
        {
            assert(&v != &dst);
            dst.v[0] = v.v[0] * m[0] + v.v[1] * m[4] + v.v[2] * m[8] + v.v[3] * m[12];
            dst.v[1] = v.v[0] * m[1] + v.v[1] * m[5] + v.v[2] * m[9] + v.v[3] * m[13];
            dst.v[2] = v.v[0] * m[2] + v.v[1] * m[6] + v.v[2] * m[10] + v.v[3] * m[14];
            dst.v[3] = v.v[0] * m[3] + v.v[1] * m[7] + v.v[2] * m[11] + v.v[3] * m[15];
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 3 && c == 3)>* = nullptr>
        [[nodiscard]] constexpr auto getTranslation() const noexcept
        {
            return math::Vector<T, 2>{m[6], m[7]};
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        [[nodiscard]] constexpr auto getTranslation() const noexcept
        {
            return math::Vector<T, 3>{m[12], m[13], m[14]};
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 3 && c == 3)>* = nullptr>
        [[nodiscard]] auto getScale() const noexcept
        {
            math::Vector<T, 2> scale;
            scale.v[0] = math::Vector<T, 2>{m[0], m[1]}.length();
            scale.v[1] = math::Vector<T, 2>{m[3], m[4]}.length();

            return scale;
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        [[nodiscard]] auto getScale() const noexcept
        {
            math::Vector<T, 3> scale;
            scale.v[0] = math::Vector<T, 3>{m[0], m[1], m[2]}.length();
            scale.v[1] = math::Vector<T, 3>{m[4], m[5], m[6]}.length();
            scale.v[2] = math::Vector<T, 3>{m[8], m[9], m[10]}.length();

            return scale;
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 3 && c == 3)>* = nullptr>
        [[nodiscard]] auto getRotation() const noexcept
        {
            return std::atan2(-m[3], m[0]);
        }

        template <auto r = rows, auto c = cols, std::enable_if_t<(r == 4 && c == 4)>* = nullptr>
        [[nodiscard]] auto getRotation() const noexcept
        {
            const auto scale = getScale();

            const auto m11 = m[0] / scale.v[0];
            const auto m21 = m[1] / scale.v[0];
            const auto m31 = m[2] / scale.v[0];

            const auto m12 = m[4] / scale.v[1];
            const auto m22 = m[5] / scale.v[1];
            const auto m32 = m[6] / scale.v[1];

            const auto m13 = m[8] / scale.v[2];
            const auto m23 = m[9] / scale.v[2];
            const auto m33 = m[10] / scale.v[2];

            math::Quaternion<T> result{
                std::sqrt(std::max(T(0), T(1) + m11 - m22 - m33)) / T(2),
                std::sqrt(std::max(T(0), T(1) - m11 + m22 - m33)) / T(2),
                std::sqrt(std::max(T(0), T(1) - m11 - m22 + m33)) / T(2),
                std::sqrt(std::max(T(0), T(1) + m11 + m22 + m33)) / T(2)
            };

            // The problem with using copysign: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/paul.htm
            result.v[0] = std::copysign(result.v[0], m32 - m23);
            result.v[1] = std::copysign(result.v[1], m13 - m31);
            result.v[2] = std::copysign(result.v[2], m21 - m12);

            result.normalize();

            return result;
        }
    };

    template <typename T, std::size_t size, std::size_t ...i>
    constexpr auto generateIdentityMatrix(std::index_sequence<i...>) noexcept
    {
        return Matrix<T, size, size>{(i % size == i / size) ? T(1) : T(0)...};
    }

    template <typename T, std::size_t size>
    constexpr auto identityMatrix = generateIdentityMatrix<T, size>(std::make_index_sequence<size * size>{});

    template <typename T, std::size_t size>
    constexpr void setIdentity(Matrix<T, size, size>& matrix) noexcept
    {
        for (std::size_t i = 0; i < size; ++i)
            for (std::size_t j = 0; j < size; ++j)
                matrix.m[j * size + i] = (j == i) ? T(1) : T(0);
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator==(const Matrix<T, rows, cols>& matrix1,
                                            const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            if (matrix1.m[i] != matrix2.m[i]) return false;
        return true;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator!=(const Matrix<T, rows, cols>& matrix1,
                                            const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            if (matrix1.m[i] != matrix2.m[i]) return true;
        return false;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator+(const Matrix<T, rows, cols>& matrix) noexcept
    {
        return matrix;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator-(const Matrix<T, rows, cols>& matrix)noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i) result.m[i] = -matrix.m[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    constexpr void negate(Matrix<T, rows, cols>& matrix) noexcept
    {
        for (auto& c : matrix.m) c = -c;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator+(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, rows, cols>& matrix2) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m[i] = matrix1.m[i] + matrix2.m[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator+=(Matrix<T, rows, cols>& matrix1,
                     const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; ++i)
            matrix1.m[i] += matrix2.m[i];
        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator-(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, rows, cols>& matrix2) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m[i] = matrix1.m[i] - matrix2.m[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator-=(Matrix<T, rows, cols>& matrix1,
                     const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; ++i)
            matrix1.m[i] -= matrix2.m[i];
        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator*(const Matrix<T, rows, cols>& matrix,
                                           const T scalar) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m[i] = matrix.m[i] * scalar;
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator*=(Matrix<T, rows, cols>& matrix,
                     const T scalar) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; ++i)
            matrix.m[i] *= scalar;
        return matrix;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator/(const Matrix<T, rows, cols>& matrix,
                                           const T scalar) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m[i] = matrix.m[i] / scalar;
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator/=(Matrix<T, rows, cols>& matrix,
                     const T scalar) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; ++i)
            matrix.m[i] /= scalar;
        return matrix;
    }

    template <typename T, std::size_t rows, std::size_t cols, std::size_t cols2>
    [[nodiscard]] constexpr auto operator*(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, cols, cols2>& matrix2) noexcept
    {
        Matrix<T, rows, cols2> result{};

        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols2; ++j)
                for (std::size_t k = 0; k < cols; ++k)
                    result.m[i * cols2 + j] += matrix1.m[i * cols + k] * matrix2.m[k * cols2 + j];

        return result;
    }

    template <typename T, std::size_t size>
    auto& operator*=(Matrix<T, size, size>& matrix1,
                     const Matrix<T, size, size>& matrix2) noexcept
    {
        std::array<T, size * size> result{};

        for (std::size_t i = 0; i < size; ++i)
            for (std::size_t j = 0; j < size; ++j)
                for (std::size_t k = 0; k < size; ++k)
                    result[i * size + j] += matrix1.m[i * size + k] * matrix2.m[k * size + j];

        matrix1.m = result;

        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] auto operator*(const T scalar,
                                 const Matrix<T, rows, cols>& mat) noexcept
    {
        return mat * scalar;
    }

    template <
        typename T, std::size_t dims,
        std::size_t size,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    [[nodiscard]] auto operator*(const Vector<T, dims>& vector,
                                 const Matrix<T, size, size>& matrix) noexcept
    {
        Vector<T, dims> result{};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                result.v[i] += vector.v[j] * matrix.m[j * size + i];

        return result;
    }

    template <
        typename T, std::size_t dims,
        std::size_t size,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    auto& operator*=(Vector<T, dims>& vector,
                     const Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(dims <= size);
        const auto temp = vector.v;
        vector.v = {};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                vector.v[i] += temp[j] * matrix.m[j * size + i];

        return vector;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto transposed(const Matrix<T, rows, cols>& matrix) noexcept
    {
        Matrix<T, cols, rows> result;
        for (std::size_t i = 0; i < cols; ++i)
            for (std::size_t j = 0; j < rows; ++j)
                result.m[i * rows + j] = matrix.m[j * cols + i];
        return result;
    }

    template <typename T, std::size_t size>
    void transpose(Matrix<T, size, size>& matrix) noexcept
    {
        for (std::size_t i = 1; i < size; ++i)
            for (std::size_t j = 0; j < i; ++j)
            {
                T temp = std::move(matrix.m[i * size + j]);
                matrix.m[i * size + j] = std::move(matrix.m[j * size + i]);
                matrix.m[j * size + i] = std::move(temp);
            }
    }

    template <typename T, std::size_t size>
    void invert(Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(size <= 4);

        if constexpr (size == 1)
            matrix.m[0] = 1.0F / matrix.m[0];
        else if constexpr (size == 2)
        {
            const auto det = matrix.m[0] * matrix.m[3] - matrix.m[1] * matrix.m[2];
            const std::array<T, size * size> adjugate{
                matrix.m[3],
                -matrix.m[1],
                -matrix.m[2],
                matrix.m[0]
            };

            matrix.m[0] = adjugate[0] / det;
            matrix.m[1] = adjugate[1] / det;
            matrix.m[2] = adjugate[2] / det;
            matrix.m[3] = adjugate[3] / det;
        }
        else if constexpr (size == 3)
        {
            const auto a0 = matrix.m[4] * matrix.m[8] - matrix.m[5] * matrix.m[7];
            const auto a1 = matrix.m[3] * matrix.m[8] - matrix.m[5] * matrix.m[6];
            const auto a2 = matrix.m[3] * matrix.m[7] - matrix.m[4] * matrix.m[6];

            const auto det = matrix.m[0] * a0 - matrix.m[1] * a1 + matrix.m[2] * a2;

            const std::array<T, size * size> adjugate{
                a0,
                -matrix.m[1] * matrix.m[8] + matrix.m[2] * matrix.m[7],
                matrix.m[1] * matrix.m[5] - matrix.m[2] * matrix.m[4],

                -a1,
                matrix.m[0] * matrix.m[8] - matrix.m[2] * matrix.m[6],
                -matrix.m[0] * matrix.m[5] + matrix.m[2] * matrix.m[3],

                a2,
                -matrix.m[0] * matrix.m[7] + matrix.m[1] * matrix.m[6],
                matrix.m[0] * matrix.m[4] - matrix.m[1] * matrix.m[3]
            };

            matrix.m[0] = adjugate[0] / det;
            matrix.m[1] = adjugate[1] / det;
            matrix.m[2] = adjugate[2] / det;
            matrix.m[3] = adjugate[3] / det;
            matrix.m[4] = adjugate[4] / det;
            matrix.m[5] = adjugate[5] / det;
            matrix.m[6] = adjugate[6] / det;
            matrix.m[7] = adjugate[7] / det;
            matrix.m[8] = adjugate[8] / det;
        }
        else if constexpr (size == 4)
        {
            const auto a0 = matrix.m[0] * matrix.m[5] - matrix.m[1] * matrix.m[4];
            const auto a1 = matrix.m[0] * matrix.m[6] - matrix.m[2] * matrix.m[4];
            const auto a2 = matrix.m[0] * matrix.m[7] - matrix.m[3] * matrix.m[4];
            const auto a3 = matrix.m[1] * matrix.m[6] - matrix.m[2] * matrix.m[5];
            const auto a4 = matrix.m[1] * matrix.m[7] - matrix.m[3] * matrix.m[5];
            const auto a5 = matrix.m[2] * matrix.m[7] - matrix.m[3] * matrix.m[6];
            const auto b0 = matrix.m[8] * matrix.m[13] - matrix.m[9] * matrix.m[12];
            const auto b1 = matrix.m[8] * matrix.m[14] - matrix.m[10] * matrix.m[12];
            const auto b2 = matrix.m[8] * matrix.m[15] - matrix.m[11] * matrix.m[12];
            const auto b3 = matrix.m[9] * matrix.m[14] - matrix.m[10] * matrix.m[13];
            const auto b4 = matrix.m[9] * matrix.m[15] - matrix.m[11] * matrix.m[13];
            const auto b5 = matrix.m[10] * matrix.m[15] - matrix.m[11] * matrix.m[14];

            const auto det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            const std::array<T, size * size> adjugate{
                matrix.m[5] * b5 - matrix.m[6] * b4 + matrix.m[7] * b3,
                -(matrix.m[1] * b5 - matrix.m[2] * b4 + matrix.m[3] * b3),
                matrix.m[13] * a5 - matrix.m[14] * a4 + matrix.m[15] * a3,
                -(matrix.m[9] * a5 - matrix.m[10] * a4 + matrix.m[11] * a3),

                -(matrix.m[4] * b5 - matrix.m[6] * b2 + matrix.m[7] * b1),
                matrix.m[0] * b5 - matrix.m[2] * b2 + matrix.m[3] * b1,
                -(matrix.m[12] * a5 - matrix.m[14] * a2 + matrix.m[15] * a1),
                matrix.m[8] * a5 - matrix.m[10] * a2 + matrix.m[11] * a1,

                matrix.m[4] * b4 - matrix.m[5] * b2 + matrix.m[7] * b0,
                -(matrix.m[0] * b4 - matrix.m[1] * b2 + matrix.m[3] * b0),
                matrix.m[12] * a4 - matrix.m[13] * a2 + matrix.m[15] * a0,
                -(matrix.m[8] * a4 - matrix.m[9] * a2 + matrix.m[11] * a0),

                -(matrix.m[4] * b3 - matrix.m[5] * b1 + matrix.m[6] * b0),
                matrix.m[0] * b3 - matrix.m[1] * b1 + matrix.m[2] * b0,
                -(matrix.m[12] * a3 - matrix.m[13] * a1 + matrix.m[14] * a0),
                matrix.m[8] * a3 - matrix.m[9] * a1 + matrix.m[10] * a0
            };

            matrix.m[0] = adjugate[0] / det;
            matrix.m[1] = adjugate[1] / det;
            matrix.m[2] = adjugate[2] / det;
            matrix.m[3] = adjugate[3] / det;
            matrix.m[4] = adjugate[4] / det;
            matrix.m[5] = adjugate[5] / det;
            matrix.m[6] = adjugate[6] / det;
            matrix.m[7] = adjugate[7] / det;
            matrix.m[8] = adjugate[8] / det;
            matrix.m[9] = adjugate[9] / det;
            matrix.m[10] = adjugate[10] / det;
            matrix.m[11] = adjugate[11] / det;
            matrix.m[12] = adjugate[12] / det;
            matrix.m[13] = adjugate[13] / det;
            matrix.m[14] = adjugate[14] / det;
            matrix.m[15] = adjugate[15] / det;
        }
    }

    template <typename T, std::size_t size>
    [[nodiscard]] constexpr auto inverse(const Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(size <= 4);

        Matrix<T, size, size> result;

        if constexpr (size == 1)
            result.m[0] = 1.0F / matrix.m[0];
        else if constexpr (size == 2)
        {
            const auto det = matrix.m[0] * matrix.m[3] - matrix.m[1] * matrix.m[2];
            result.m[0] = matrix.m[3] / det;
            result.m[1] = -matrix.m[1] / det;
            result.m[2] = -matrix.m[2] / det;
            result.m[3] = matrix.m[0] / det;
        }
        else if constexpr (size == 3)
        {
            const auto a0 = matrix.m[4] * matrix.m[8] - matrix.m[5] * matrix.m[7];
            const auto a1 = matrix.m[3] * matrix.m[8] - matrix.m[5] * matrix.m[6];
            const auto a2 = matrix.m[3] * matrix.m[7] - matrix.m[4] * matrix.m[6];

            const auto det = matrix.m[0] * a0 - matrix.m[1] * a1 + matrix.m[2] * a2;

            result.m[0] = a0 / det;
            result.m[1] = -(matrix.m[1] * matrix.m[8] - matrix.m[2] * matrix.m[7]) / det;
            result.m[2] = (matrix.m[1] * matrix.m[5] - matrix.m[2] * matrix.m[4]) / det;

            result.m[3] = -a1 / det;
            result.m[4] = (matrix.m[0] * matrix.m[8] - matrix.m[2] * matrix.m[6]) / det;
            result.m[5] = -(matrix.m[0] * matrix.m[5] - matrix.m[2] * matrix.m[3]) / det;

            result.m[6] = a2 / det;
            result.m[7] = -(matrix.m[0] * matrix.m[7] - matrix.m[1] * matrix.m[6]) / det;
            result.m[8] = (matrix.m[0] * matrix.m[4] - matrix.m[1] * matrix.m[3]) / det;
        }
        else if constexpr (size == 4)
        {
            const auto a0 = matrix.m[0] * matrix.m[5] - matrix.m[1] * matrix.m[4];
            const auto a1 = matrix.m[0] * matrix.m[6] - matrix.m[2] * matrix.m[4];
            const auto a2 = matrix.m[0] * matrix.m[7] - matrix.m[3] * matrix.m[4];
            const auto a3 = matrix.m[1] * matrix.m[6] - matrix.m[2] * matrix.m[5];
            const auto a4 = matrix.m[1] * matrix.m[7] - matrix.m[3] * matrix.m[5];
            const auto a5 = matrix.m[2] * matrix.m[7] - matrix.m[3] * matrix.m[6];
            const auto b0 = matrix.m[8] * matrix.m[13] - matrix.m[9] * matrix.m[12];
            const auto b1 = matrix.m[8] * matrix.m[14] - matrix.m[10] * matrix.m[12];
            const auto b2 = matrix.m[8] * matrix.m[15] - matrix.m[11] * matrix.m[12];
            const auto b3 = matrix.m[9] * matrix.m[14] - matrix.m[10] * matrix.m[13];
            const auto b4 = matrix.m[9] * matrix.m[15] - matrix.m[11] * matrix.m[13];
            const auto b5 = matrix.m[10] * matrix.m[15] - matrix.m[11] * matrix.m[14];

            const auto det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            result.m[0] = (matrix.m[5] * b5 - matrix.m[6] * b4 + matrix.m[7] * b3) / det;
            result.m[1] = -(matrix.m[1] * b5 - matrix.m[2] * b4 + matrix.m[3] * b3) / det;
            result.m[2] = (matrix.m[13] * a5 - matrix.m[14] * a4 + matrix.m[15] * a3) / det;
            result.m[3] = -(matrix.m[9] * a5 - matrix.m[10] * a4 + matrix.m[11] * a3) / det;

            result.m[4] = -(matrix.m[4] * b5 - matrix.m[6] * b2 + matrix.m[7] * b1) / det;
            result.m[5] = (matrix.m[0] * b5 - matrix.m[2] * b2 + matrix.m[3] * b1) / det;
            result.m[6] = -(matrix.m[12] * a5 - matrix.m[14] * a2 + matrix.m[15] * a1) / det;
            result.m[7] = (matrix.m[8] * a5 - matrix.m[10] * a2 + matrix.m[11] * a1) / det;

            result.m[8] = (matrix.m[4] * b4 - matrix.m[5] * b2 + matrix.m[7] * b0) / det;
            result.m[9] = -(matrix.m[0] * b4 - matrix.m[1] * b2 + matrix.m[3] * b0) / det;
            result.m[10] = (matrix.m[12] * a4 - matrix.m[13] * a2 + matrix.m[15] * a0) / det;
            result.m[11] = -(matrix.m[8] * a4 - matrix.m[9] * a2 + matrix.m[11] * a0) / det;

            result.m[12] = -(matrix.m[4] * b3 - matrix.m[5] * b1 + matrix.m[6] * b0) / det;
            result.m[13] = (matrix.m[0] * b3 - matrix.m[1] * b1 + matrix.m[2] * b0) / det;
            result.m[14] = -(matrix.m[12] * a3 - matrix.m[13] * a1 + matrix.m[14] * a0) / det;
            result.m[15] = (matrix.m[8] * a3 - matrix.m[9] * a1 + matrix.m[10] * a0) / det;
        }

        return result;
    }

    template <typename T>
    [[nodiscard]] auto getUpVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{matrix.m[4], matrix.m[5], matrix.m[6]};
    }

    template <typename T>
    [[nodiscard]] auto getDownVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{-matrix.m[4], -matrix.m[5], -matrix.m[6]};
    }

    template <typename T>
    [[nodiscard]] auto getLeftVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{-matrix.m[0], -matrix.m[1], -matrix.m[2]};
    }

    template <typename T>
    [[nodiscard]] auto getRightVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{matrix.m[0], matrix.m[1], matrix.m[2]};
    }

    template <typename T>
    [[nodiscard]] auto getForwardVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{-matrix.m[8], -matrix.m[9], -matrix.m[10]};
    }

    template <typename T>
    [[nodiscard]] auto getBackVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return math::Vector<T, 3>{matrix.m[8], matrix.m[9], matrix.m[10]};
    }

    template <typename T>
    [[nodiscard]] auto getFrustumLeftPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] + matrix.m[0],
                                          matrix.m[7] + matrix.m[4],
                                          matrix.m[11] + matrix.m[8],
                                          matrix.m[15] + matrix.m[12]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumRightPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] - matrix.m[0],
                                          matrix.m[7] - matrix.m[4],
                                          matrix.m[11] - matrix.m[8],
                                          matrix.m[15] - matrix.m[12]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumBottomPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] + matrix.m[1],
                                          matrix.m[7] + matrix.m[5],
                                          matrix.m[11] + matrix.m[9],
                                          matrix.m[15] + matrix.m[13]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumTopPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] - matrix.m[1],
                                          matrix.m[7] - matrix.m[5],
                                          matrix.m[11] - matrix.m[9],
                                          matrix.m[15] - matrix.m[13]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumNearPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] + matrix.m[2],
                                          matrix.m[7] + matrix.m[6],
                                          matrix.m[11] + matrix.m[10],
                                          matrix.m[15] + matrix.m[14]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumFarPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Plane<T>::makeFrustumPlane(matrix.m[3] - matrix.m[2],
                                          matrix.m[7] - matrix.m[6],
                                          matrix.m[11] - matrix.m[10],
                                          matrix.m[15] - matrix.m[14]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustum(const Matrix<T, 4, 4>& matrix) noexcept(false)
    {
        return ConvexVolume<T>({
            getFrustumLeftPlane(matrix),
            getFrustumRightPlane(matrix),
            getFrustumBottomPlane(matrix),
            getFrustumTopPlane(matrix),
            getFrustumNearPlane(matrix),
            getFrustumFarPlane(matrix)
        });
    }

    template <typename T>
    void setLookAt(Matrix<T, 4, 4>& matrix,
                   const T eyePositionX, const T eyePositionY, const T eyePositionZ,
                   const T targetPositionX, const T targetPositionY, const T targetPositionZ,
                   const T upX, const T upY, const T upZ) noexcept
    {
        const math::Vector<T, 3> eye{eyePositionX, eyePositionY, eyePositionZ};
        const math::Vector<T, 3> target{targetPositionX, targetPositionY, targetPositionZ};
        math::Vector<T, 3> up{upX, upY, upZ};
        up.normalize();

        math::Vector<T, 3> zaxis = target - eye;
        zaxis.normalize();

        math::Vector<T, 3> xaxis = up.cross(zaxis);
        xaxis.normalize();

        math::Vector<T, 3> yaxis = zaxis.cross(xaxis);
        yaxis.normalize();

        matrix.m[0] = xaxis.v[0];
        matrix.m[1] = yaxis.v[0];
        matrix.m[2] = zaxis.v[0];
        matrix.m[3] = T(0);

        matrix.m[4] = xaxis.v[1];
        matrix.m[5] = yaxis.v[1];
        matrix.m[6] = zaxis.v[1];
        matrix.m[7] = T(0);

        matrix.m[8] = xaxis.v[2];
        matrix.m[9] = yaxis.v[2];
        matrix.m[10] = zaxis.v[2];
        matrix.m[11] = T(0);

        matrix.m[12] = xaxis.dot(-eye);
        matrix.m[13] = yaxis.dot(-eye);
        matrix.m[14] = zaxis.dot(-eye);
        matrix.m[15] = T(1);
    }

    template <typename T>
    void setLookAt(Matrix<T, 4, 4>& matrix,
                   const math::Vector<T, 3>& eyePosition,
                   const math::Vector<T, 3>& targetPosition,
                   const math::Vector<T, 3>& up) noexcept
    {
        setLookAt(matrix,
                  eyePosition.v[0], eyePosition.v[1], eyePosition.v[2],
                  targetPosition.v[0], targetPosition.v[1], targetPosition.v[2],
                  up.v[0], up.v[1], up.v[2]);
    }

    template <typename T>
    void setPerspective(Matrix<T, 4, 4>& matrix,
                        const T fieldOfView, const T aspectRatio,
                        const T nearClip, const T farClip) noexcept
    {
        assert(aspectRatio);
        assert(farClip != nearClip);

        const auto theta = fieldOfView / T(2);
        if (std::fabs(std::fmod(theta, pi<T> / T(2))) <= std::numeric_limits<T>::epsilon())
            return;

        const auto divisor = std::tan(theta);
        assert(divisor);
        const auto factor = T(1) / divisor;

        matrix.m = {};

        matrix.m[0] = factor / aspectRatio;
        matrix.m[5] = factor;
        matrix.m[10] = farClip / (farClip - nearClip);
        matrix.m[11] = T(1);
        matrix.m[14] = -nearClip * farClip / (farClip - nearClip);
    }

    template <typename T>
    void setOrthographic(Matrix<T, 4, 4>& matrix,
                         const T width, const T height,
                         const T nearClip, const T farClip) noexcept
    {
        assert(width);
        assert(height);
        assert(farClip != nearClip);

        matrix.m = {};

        matrix.m[0] = T(2) / width;
        matrix.m[5] = T(2) / height;
        matrix.m[10] = T(1) / (farClip - nearClip);
        matrix.m[14] = nearClip / (nearClip - farClip);
        matrix.m[15] = T(1);
    }

    template <typename T>
    void setOrthographic(Matrix<T, 4, 4>& matrix,
                         const T left, const T right,
                         const T bottom, const T top,
                         const T nearClip, const T farClip) noexcept
    {
        assert(right != left);
        assert(top != bottom);
        assert(farClip != nearClip);

        matrix.m = {};

        matrix.m[0] = T(2) / (right - left);
        matrix.m[5] = T(2) / (top - bottom);
        matrix.m[10] = T(1) / (farClip - nearClip);
        matrix.m[12] = (left + right) / (left - right);
        matrix.m[13] = (bottom + top) / (bottom - top);
        matrix.m[14] = nearClip / (nearClip - farClip);
        matrix.m[15] = T(1);
    }

    template <typename T, std::size_t size>
    void setScale(Matrix<T, size, size>& matrix,
                  const T scale) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < size - 1; ++i)
            matrix.m[i * size + i] = scale;
    }

    template <typename T, std::size_t size>
    void setScale(Matrix<T, size, size>& matrix,
                  const math::Vector<T, size - 1>& scale) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < size - 1; ++i)
            matrix.m[i * size + i] = scale[i];
    }

    template <typename T>
    void setRotation(Matrix<T, 3, 3>& matrix,
                     const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        matrix.m[0] = cosine;
        matrix.m[3] = -sine;
        matrix.m[1] = sine;
        matrix.m[4] = cosine;
    }

    template <typename T>
    void setRotation(Matrix<T, 4, 4>& matrix,
                     const math::Vector<T, 3>& axis, T angle) noexcept
    {
        auto x = axis.v[0];
        auto y = axis.v[1];
        auto z = axis.v[2];

        const auto squared = x * x + y * y + z * z;
        if (squared != T(1))
        {
            const auto length = std::sqrt(squared);
            if (length > std::numeric_limits<T>::epsilon()) // bigger than zero
            {
                x /= length;
                y /= length;
                z /= length;
            }
        }

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        const auto t = T(1) - cosine;
        const auto tx = t * x;
        const auto ty = t * y;
        const auto tz = t * z;
        const auto txy = tx * y;
        const auto txz = tx * z;
        const auto tyz = ty * z;
        const auto sx = sine * x;
        const auto sy = sine * y;
        const auto sz = sine * z;

        matrix.m[0] = cosine + tx * x;
        matrix.m[4] = txy - sz;
        matrix.m[8] = txz + sy;
        matrix.m[12] = T(0);

        matrix.m[1] = txy + sz;
        matrix.m[5] = cosine + ty * y;
        matrix.m[9] = tyz - sx;
        matrix.m[13] = T(0);

        matrix.m[2] = txz - sy;
        matrix.m[6] = tyz + sx;
        matrix.m[10] = cosine + tz * z;
        matrix.m[14] = T(0);

        matrix.m[3] = T(0);
        matrix.m[7] = T(0);
        matrix.m[11] = T(0);
        matrix.m[15] = T(1);
    }

    template <typename T>
    void setRotation(Matrix<T, 4, 4>& matrix,
                     const math::Quaternion<T>& rotation) noexcept
    {
        const auto wx = rotation.v[3] * rotation.v[0];
        const auto wy = rotation.v[3] * rotation.v[1];
        const auto wz = rotation.v[3] * rotation.v[2];

        const auto xx = rotation.v[0] * rotation.v[0];
        const auto xy = rotation.v[0] * rotation.v[1];
        const auto xz = rotation.v[0] * rotation.v[2];

        const auto yy = rotation.v[1] * rotation.v[1];
        const auto yz = rotation.v[1] * rotation.v[2];

        const auto zz = rotation.v[2] * rotation.v[2];

        matrix.m[0] = T(1) - T(2) * (yy + zz);
        matrix.m[4] = T(2) * (xy - wz);
        matrix.m[8] = T(2) * (xz + wy);
        matrix.m[12] = T(0);

        matrix.m[1] = T(2) * (xy + wz);
        matrix.m[5] = T(1) - T(2) * (xx + zz);
        matrix.m[9] = T(2) * (yz - wx);
        matrix.m[13] = T(0);

        matrix.m[2] = T(2) * (xz - wy);
        matrix.m[6] = T(2) * (yz + wx);
        matrix.m[10] = T(1) - T(2) * (xx + yy);
        matrix.m[14] = T(0);

        matrix.m[3] = T(0);
        matrix.m[7] = T(0);
        matrix.m[11] = T(0);
        matrix.m[15] = T(1);
    }

    template <typename T>
    void setRotationX(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        matrix.m[5] = cosine;
        matrix.m[9] = -sine;
        matrix.m[6] = sine;
        matrix.m[10] = cosine;
    }

    template <typename T>
    void setRotationY(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        matrix.m[0] = cosine;
        matrix.m[8] = sine;
        matrix.m[2] = -sine;
        matrix.m[10] = cosine;
    }

    template <typename T>
    void setRotationZ(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        matrix.m[0] = cosine;
        matrix.m[4] = -sine;
        matrix.m[1] = sine;
        matrix.m[5] = cosine;
    }

    template <typename T>
    void setTranslation(Matrix<T, 4, 4>& matrix,
                        const math::Vector<T, 3>& translation) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < 3; ++i)
            matrix.m[3 * 4 + i] = translation[i];
    }

    template <typename T>
    void setTranslation(Matrix<T, 3, 3>& matrix,
                        const T x, const T y) noexcept
    {
        setIdentity(matrix);

        matrix.m[6] = x;
        matrix.m[7] = y;
    }

    template <typename T>
    void setTranslation(Matrix<T, 4, 4>& matrix,
                        const T x, const T y, const T z) noexcept
    {
        setIdentity(matrix);

        matrix.m[12] = x;
        matrix.m[13] = y;
        matrix.m[14] = z;
    }
}

#include "MatrixNeon.hpp"
#include "MatrixSse.hpp"

#endif // OUZEL_MATH_MATRIX_HPP
