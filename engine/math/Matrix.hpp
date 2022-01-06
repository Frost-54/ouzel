// Ouzel by Elviss Strazdins

#ifndef OUZEL_MATH_MATRIX_HPP
#define OUZEL_MATH_MATRIX_HPP

#include <algorithm>
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
#include "Vector.hpp"

namespace ouzel::math
{
    template <typename T, std::size_t rows, std::size_t cols = rows>
    struct MatrixElements final
    {
        T v[rows * cols];
    };

    template <typename T, std::size_t rows, std::size_t cols, std::size_t ...i>
    constexpr auto transpose(const MatrixElements<T, cols, rows> a,
                             const std::index_sequence<i...>) noexcept
    {
        return MatrixElements<T, cols, rows>{a.v[((i % rows) * cols + i / rows)]...};
    }

    template <typename T, std::size_t size, std::size_t ...i>
    constexpr auto identity(std::index_sequence<i...>) noexcept
    {
        return MatrixElements<T, size, size>{(i % size == i / size) ? T(1) : T(0)...};
    }

    template <typename T, std::size_t rows, std::size_t cols = rows>
    class Matrix final
    {
    public:
#if defined(__SSE__) || defined(_M_X64) || _M_IX86_FP >= 1 || defined(__ARM_NEON__)
        alignas(std::is_same_v<T, float> && rows == 4 && cols == 4 ? cols * sizeof(T) : alignof(T))
#endif
#if (defined(__SSE2__) || defined(_M_X64) || _M_IX86_FP >= 2) || (defined(__ARM_NEON__) && defined(__aarch64__))
        alignas(std::is_same_v<T, double> && rows == 4 && cols == 4 ? cols * sizeof(T) : alignof(T))
#endif
        MatrixElements<T, cols, rows> m; // column-major matrix

        constexpr Matrix() noexcept = default;

        template <typename ...A>
        explicit constexpr Matrix(const A... args) noexcept:
            m{transpose(MatrixElements<T, cols, rows>{args...}, std::make_index_sequence<rows * cols>{})}
        {
        }

        [[nodiscard]] auto& operator()(const std::size_t row, const std::size_t col) noexcept { return m.v[col * rows + row]; }
        [[nodiscard]] constexpr auto operator()(const std::size_t row, const std::size_t col) const noexcept { return m.v[col * rows + row]; }
    };

    template <typename T, std::size_t size>
    constexpr auto identityMatrix = Matrix<T, size, size>{identity<T, size>(std::make_index_sequence<size * size>{})};

    template <typename T, std::size_t size>
    constexpr void setIdentity(Matrix<T, size, size>& matrix) noexcept
    {
        for (std::size_t i = 0; i < size; ++i)
            for (std::size_t j = 0; j < size; ++j)
                matrix.m.v[j * size + i] = (j == i) ? T(1) : T(0);
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator==(const Matrix<T, rows, cols>& matrix1,
                                            const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            if (matrix1.m.v[i] != matrix2.m.v[i]) return false;
        return true;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator!=(const Matrix<T, rows, cols>& matrix1,
                                            const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            if (matrix1.m.v[i] != matrix2.m.v[i]) return true;
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
        for (std::size_t i = 0; i < rows * cols; ++i) result.m.v[i] = -matrix.m.v[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    constexpr void negate(Matrix<T, rows, cols>& matrix) noexcept
    {
        for (auto& c : matrix.m.v) c = -c;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator+(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, rows, cols>& matrix2) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m.v[i] = matrix1.m.v[i] + matrix2.m.v[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator+=(Matrix<T, rows, cols>& matrix1,
                     const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            matrix1.m.v[i] += matrix2.m.v[i];
        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator-(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, rows, cols>& matrix2) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m.v[i] = matrix1.m.v[i] - matrix2.m.v[i];
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator-=(Matrix<T, rows, cols>& matrix1,
                     const Matrix<T, rows, cols>& matrix2) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            matrix1.m.v[i] -= matrix2.m.v[i];
        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols, std::size_t cols2>
    [[nodiscard]] constexpr auto operator*(const Matrix<T, rows, cols>& matrix1,
                                           const Matrix<T, cols, cols2>& matrix2) noexcept
    {
        Matrix<T, rows, cols2> result{};

        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols2; ++j)
                for (std::size_t k = 0; k < cols; ++k)
                    result.m.v[j * rows + i] += matrix1.m.v[k * rows + i] * matrix2.m.v[j * cols + k];

        return result;
    }

    template <typename T, std::size_t size>
    auto& operator*=(Matrix<T, size, size>& matrix1,
                     const Matrix<T, size, size>& matrix2) noexcept
    {
        Matrix<T, size, size> result{};

        for (std::size_t i = 0; i < size; ++i)
            for (std::size_t j = 0; j < size; ++j)
                for (std::size_t k = 0; k < size; ++k)
                    result.m.v[j * size + i] += matrix1.m.v[k * size + i] * matrix2.m.v[j * size + k];

        matrix1 = std::move(result);
        return matrix1;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator*(const Matrix<T, rows, cols>& matrix,
                                           const T scalar) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m.v[i] = matrix.m.v[i] * scalar;
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator*=(Matrix<T, rows, cols>& matrix,
                     const T scalar) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            matrix.m.v[i] *= scalar;
        return matrix;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto operator/(const Matrix<T, rows, cols>& matrix,
                                           const T scalar) noexcept
    {
        Matrix<T, rows, cols> result;
        for (std::size_t i = 0; i < rows * cols; ++i)
            result.m.v[i] = matrix.m.v[i] / scalar;
        return result;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    auto& operator/=(Matrix<T, rows, cols>& matrix,
                     const T scalar) noexcept
    {
        for (std::size_t i = 0; i < rows * cols; ++i)
            matrix.m.v[i] /= scalar;
        return matrix;
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] auto operator*(const T scalar,
                                 const Matrix<T, rows, cols>& mat) noexcept
    {
        return mat * scalar;
    }

    template <
        typename T,
        std::size_t dims,
        std::size_t size,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    [[nodiscard]] auto operator*(const Vector<T, dims>& vector,
                                 const Matrix<T, size, size>& matrix) noexcept
    {
        Vector<T, dims> result{};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                result.v[i] += vector.v[j] * matrix.m.v[i * size + j];

        return result;
    }

    template <
        typename T,
        std::size_t dims,
        std::size_t size,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    auto& operator*=(Vector<T, dims>& vector,
                     const Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(dims <= size);
        Vector<T, dims> result{};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                result[i] += vector[j] * matrix.m.v[i * size + j];

        vector = std::move(result);
        return vector;
    }

    template <
        typename T,
        std::size_t size,
        std::size_t dims,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    [[nodiscard]] auto operator*(const Matrix<T, size, size>& matrix,
                                 const Vector<T, dims>& vector) noexcept
    {
        Vector<T, dims> result{};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                result.v[i] += matrix.m.v[j * size + i] * vector.v[j];

        return result;
    }

    template <
        typename T,
        std::size_t size,
        std::size_t dims,
        std::enable_if_t<(dims <= size)>* = nullptr
    >
    void transformVector(const Matrix<T, size, size>& matrix,
                         Vector<T, dims>& vector) noexcept
    {
        Vector<T, dims> result{};

        for (std::size_t i = 0; i < dims; ++i)
            for (std::size_t j = 0; j < dims; ++j)
                result[i] += matrix.m.v[j * size + i] * vector.v[j];

        vector = std::move(result);
    }

    template <typename T, std::size_t rows, std::size_t cols>
    [[nodiscard]] constexpr auto transposed(const Matrix<T, rows, cols>& matrix) noexcept
    {
        Matrix<T, cols, rows> result;
        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols; ++j)
                result.m.v[i * cols + j] = matrix.m.v[j * rows + i];
        return result;
    }

    template <typename T, std::size_t size>
    void transpose(Matrix<T, size, size>& matrix) noexcept
    {
        for (std::size_t i = 1; i < size; ++i)
            for (std::size_t j = 0; j < i; ++j)
            {
                T temp = std::move(matrix.m.v[i * size + j]);
                matrix.m.v[i * size + j] = std::move(matrix.m.v[j * size + i]);
                matrix.m.v[j * size + i] = std::move(temp);
            }
    }

    template <typename T, std::size_t size, std::enable_if<(size <= 4)>* = nullptr>
    [[nodiscard]] constexpr auto determinant(const Matrix<T, size, size>& matrix) noexcept
    {
        if constexpr (size == 0)
            return T(1);
        if constexpr (size == 1)
            return matrix.m.v[0];
        else if constexpr (size == 2)
            return matrix.m.v[0] * matrix.m.v[3] - matrix.m.v[1] * matrix.m.v[2];
        else if constexpr (size == 3)
            return matrix.m.v[0] * matrix.m.v[4] * matrix.m.v[8] +
                matrix.m.v[1] * matrix.m.v[5] * matrix.m.v[6] +
                matrix.m.v[2] * matrix.m.v[3] * matrix.m.v[7] -
                matrix.m.v[2] * matrix.m.v[4] * matrix.m.v[6] -
                matrix.m.v[1] * matrix.m.v[3] * matrix.m.v[8] -
                matrix.m.v[0] * matrix.m.v[5] * matrix.m.v[7];
        else if constexpr (size == 4)
        {
            const auto a0 = matrix.m.v[0] * matrix.m.v[5] - matrix.m.v[1] * matrix.m.v[4];
            const auto a1 = matrix.m.v[0] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[4];
            const auto a2 = matrix.m.v[0] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[4];
            const auto a3 = matrix.m.v[1] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[5];
            const auto a4 = matrix.m.v[1] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[5];
            const auto a5 = matrix.m.v[2] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[6];
            const auto b0 = matrix.m.v[8] * matrix.m.v[13] - matrix.m.v[9] * matrix.m.v[12];
            const auto b1 = matrix.m.v[8] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[12];
            const auto b2 = matrix.m.v[8] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[12];
            const auto b3 = matrix.m.v[9] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[13];
            const auto b4 = matrix.m.v[9] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[13];
            const auto b5 = matrix.m.v[10] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[14];

            return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        }
    }

    template <typename T, std::size_t size>
    void invert(Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(size <= 4);

        if constexpr (size == 1)
            matrix.m.v[0] = 1.0F / matrix.m.v[0];
        else if constexpr (size == 2)
        {
            const auto det = matrix.m.v[0] * matrix.m.v[3] - matrix.m.v[1] * matrix.m.v[2];
            const T adjugate[size * size]{
                matrix.m.v[3],
                -matrix.m.v[1],
                -matrix.m.v[2],
                matrix.m.v[0]
            };

            matrix.m.v[0] = adjugate[0] / det;
            matrix.m.v[1] = adjugate[1] / det;
            matrix.m.v[2] = adjugate[2] / det;
            matrix.m.v[3] = adjugate[3] / det;
        }
        else if constexpr (size == 3)
        {
            const auto a0 = matrix.m.v[4] * matrix.m.v[8] - matrix.m.v[5] * matrix.m.v[7];
            const auto a1 = matrix.m.v[3] * matrix.m.v[8] - matrix.m.v[5] * matrix.m.v[6];
            const auto a2 = matrix.m.v[3] * matrix.m.v[7] - matrix.m.v[4] * matrix.m.v[6];

            const auto det = matrix.m.v[0] * a0 - matrix.m.v[1] * a1 + matrix.m.v[2] * a2;

            const T adjugate[size * size]{
                a0,
                -matrix.m.v[1] * matrix.m.v[8] + matrix.m.v[2] * matrix.m.v[7],
                matrix.m.v[1] * matrix.m.v[5] - matrix.m.v[2] * matrix.m.v[4],

                -a1,
                matrix.m.v[0] * matrix.m.v[8] - matrix.m.v[2] * matrix.m.v[6],
                -matrix.m.v[0] * matrix.m.v[5] + matrix.m.v[2] * matrix.m.v[3],

                a2,
                -matrix.m.v[0] * matrix.m.v[7] + matrix.m.v[1] * matrix.m.v[6],
                matrix.m.v[0] * matrix.m.v[4] - matrix.m.v[1] * matrix.m.v[3]
            };

            matrix.m.v[0] = adjugate[0] / det;
            matrix.m.v[1] = adjugate[1] / det;
            matrix.m.v[2] = adjugate[2] / det;
            matrix.m.v[3] = adjugate[3] / det;
            matrix.m.v[4] = adjugate[4] / det;
            matrix.m.v[5] = adjugate[5] / det;
            matrix.m.v[6] = adjugate[6] / det;
            matrix.m.v[7] = adjugate[7] / det;
            matrix.m.v[8] = adjugate[8] / det;
        }
        else if constexpr (size == 4)
        {
            const auto a0 = matrix.m.v[0] * matrix.m.v[5] - matrix.m.v[1] * matrix.m.v[4];
            const auto a1 = matrix.m.v[0] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[4];
            const auto a2 = matrix.m.v[0] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[4];
            const auto a3 = matrix.m.v[1] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[5];
            const auto a4 = matrix.m.v[1] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[5];
            const auto a5 = matrix.m.v[2] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[6];
            const auto b0 = matrix.m.v[8] * matrix.m.v[13] - matrix.m.v[9] * matrix.m.v[12];
            const auto b1 = matrix.m.v[8] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[12];
            const auto b2 = matrix.m.v[8] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[12];
            const auto b3 = matrix.m.v[9] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[13];
            const auto b4 = matrix.m.v[9] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[13];
            const auto b5 = matrix.m.v[10] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[14];

            const auto det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            const T adjugate[size * size]{
                matrix.m.v[5] * b5 - matrix.m.v[6] * b4 + matrix.m.v[7] * b3,
                -(matrix.m.v[1] * b5 - matrix.m.v[2] * b4 + matrix.m.v[3] * b3),
                matrix.m.v[13] * a5 - matrix.m.v[14] * a4 + matrix.m.v[15] * a3,
                -(matrix.m.v[9] * a5 - matrix.m.v[10] * a4 + matrix.m.v[11] * a3),

                -(matrix.m.v[4] * b5 - matrix.m.v[6] * b2 + matrix.m.v[7] * b1),
                matrix.m.v[0] * b5 - matrix.m.v[2] * b2 + matrix.m.v[3] * b1,
                -(matrix.m.v[12] * a5 - matrix.m.v[14] * a2 + matrix.m.v[15] * a1),
                matrix.m.v[8] * a5 - matrix.m.v[10] * a2 + matrix.m.v[11] * a1,

                matrix.m.v[4] * b4 - matrix.m.v[5] * b2 + matrix.m.v[7] * b0,
                -(matrix.m.v[0] * b4 - matrix.m.v[1] * b2 + matrix.m.v[3] * b0),
                matrix.m.v[12] * a4 - matrix.m.v[13] * a2 + matrix.m.v[15] * a0,
                -(matrix.m.v[8] * a4 - matrix.m.v[9] * a2 + matrix.m.v[11] * a0),

                -(matrix.m.v[4] * b3 - matrix.m.v[5] * b1 + matrix.m.v[6] * b0),
                matrix.m.v[0] * b3 - matrix.m.v[1] * b1 + matrix.m.v[2] * b0,
                -(matrix.m.v[12] * a3 - matrix.m.v[13] * a1 + matrix.m.v[14] * a0),
                matrix.m.v[8] * a3 - matrix.m.v[9] * a1 + matrix.m.v[10] * a0
            };

            matrix.m.v[0] = adjugate[0] / det;
            matrix.m.v[1] = adjugate[1] / det;
            matrix.m.v[2] = adjugate[2] / det;
            matrix.m.v[3] = adjugate[3] / det;
            matrix.m.v[4] = adjugate[4] / det;
            matrix.m.v[5] = adjugate[5] / det;
            matrix.m.v[6] = adjugate[6] / det;
            matrix.m.v[7] = adjugate[7] / det;
            matrix.m.v[8] = adjugate[8] / det;
            matrix.m.v[9] = adjugate[9] / det;
            matrix.m.v[10] = adjugate[10] / det;
            matrix.m.v[11] = adjugate[11] / det;
            matrix.m.v[12] = adjugate[12] / det;
            matrix.m.v[13] = adjugate[13] / det;
            matrix.m.v[14] = adjugate[14] / det;
            matrix.m.v[15] = adjugate[15] / det;
        }
    }

    template <typename T, std::size_t size>
    [[nodiscard]] constexpr auto inverse(const Matrix<T, size, size>& matrix) noexcept
    {
        static_assert(size <= 4);

        Matrix<T, size, size> result;

        if constexpr (size == 1)
            result.m.v[0] = 1.0F / matrix.m.v[0];
        else if constexpr (size == 2)
        {
            const auto det = matrix.m.v[0] * matrix.m.v[3] - matrix.m.v[1] * matrix.m.v[2];
            result.m.v[0] = matrix.m.v[3] / det;
            result.m.v[1] = -matrix.m.v[1] / det;
            result.m.v[2] = -matrix.m.v[2] / det;
            result.m.v[3] = matrix.m.v[0] / det;
        }
        else if constexpr (size == 3)
        {
            const auto a0 = matrix.m.v[4] * matrix.m.v[8] - matrix.m.v[5] * matrix.m.v[7];
            const auto a1 = matrix.m.v[3] * matrix.m.v[8] - matrix.m.v[5] * matrix.m.v[6];
            const auto a2 = matrix.m.v[3] * matrix.m.v[7] - matrix.m.v[4] * matrix.m.v[6];

            const auto det = matrix.m.v[0] * a0 - matrix.m.v[1] * a1 + matrix.m.v[2] * a2;

            result.m.v[0] = a0 / det;
            result.m.v[1] = -(matrix.m.v[1] * matrix.m.v[8] - matrix.m.v[2] * matrix.m.v[7]) / det;
            result.m.v[2] = (matrix.m.v[1] * matrix.m.v[5] - matrix.m.v[2] * matrix.m.v[4]) / det;

            result.m.v[3] = -a1 / det;
            result.m.v[4] = (matrix.m.v[0] * matrix.m.v[8] - matrix.m.v[2] * matrix.m.v[6]) / det;
            result.m.v[5] = -(matrix.m.v[0] * matrix.m.v[5] - matrix.m.v[2] * matrix.m.v[3]) / det;

            result.m.v[6] = a2 / det;
            result.m.v[7] = -(matrix.m.v[0] * matrix.m.v[7] - matrix.m.v[1] * matrix.m.v[6]) / det;
            result.m.v[8] = (matrix.m.v[0] * matrix.m.v[4] - matrix.m.v[1] * matrix.m.v[3]) / det;
        }
        else if constexpr (size == 4)
        {
            const auto a0 = matrix.m.v[0] * matrix.m.v[5] - matrix.m.v[1] * matrix.m.v[4];
            const auto a1 = matrix.m.v[0] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[4];
            const auto a2 = matrix.m.v[0] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[4];
            const auto a3 = matrix.m.v[1] * matrix.m.v[6] - matrix.m.v[2] * matrix.m.v[5];
            const auto a4 = matrix.m.v[1] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[5];
            const auto a5 = matrix.m.v[2] * matrix.m.v[7] - matrix.m.v[3] * matrix.m.v[6];
            const auto b0 = matrix.m.v[8] * matrix.m.v[13] - matrix.m.v[9] * matrix.m.v[12];
            const auto b1 = matrix.m.v[8] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[12];
            const auto b2 = matrix.m.v[8] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[12];
            const auto b3 = matrix.m.v[9] * matrix.m.v[14] - matrix.m.v[10] * matrix.m.v[13];
            const auto b4 = matrix.m.v[9] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[13];
            const auto b5 = matrix.m.v[10] * matrix.m.v[15] - matrix.m.v[11] * matrix.m.v[14];

            const auto det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            result.m.v[0] = (matrix.m.v[5] * b5 - matrix.m.v[6] * b4 + matrix.m.v[7] * b3) / det;
            result.m.v[1] = -(matrix.m.v[1] * b5 - matrix.m.v[2] * b4 + matrix.m.v[3] * b3) / det;
            result.m.v[2] = (matrix.m.v[13] * a5 - matrix.m.v[14] * a4 + matrix.m.v[15] * a3) / det;
            result.m.v[3] = -(matrix.m.v[9] * a5 - matrix.m.v[10] * a4 + matrix.m.v[11] * a3) / det;

            result.m.v[4] = -(matrix.m.v[4] * b5 - matrix.m.v[6] * b2 + matrix.m.v[7] * b1) / det;
            result.m.v[5] = (matrix.m.v[0] * b5 - matrix.m.v[2] * b2 + matrix.m.v[3] * b1) / det;
            result.m.v[6] = -(matrix.m.v[12] * a5 - matrix.m.v[14] * a2 + matrix.m.v[15] * a1) / det;
            result.m.v[7] = (matrix.m.v[8] * a5 - matrix.m.v[10] * a2 + matrix.m.v[11] * a1) / det;

            result.m.v[8] = (matrix.m.v[4] * b4 - matrix.m.v[5] * b2 + matrix.m.v[7] * b0) / det;
            result.m.v[9] = -(matrix.m.v[0] * b4 - matrix.m.v[1] * b2 + matrix.m.v[3] * b0) / det;
            result.m.v[10] = (matrix.m.v[12] * a4 - matrix.m.v[13] * a2 + matrix.m.v[15] * a0) / det;
            result.m.v[11] = -(matrix.m.v[8] * a4 - matrix.m.v[9] * a2 + matrix.m.v[11] * a0) / det;

            result.m.v[12] = -(matrix.m.v[4] * b3 - matrix.m.v[5] * b1 + matrix.m.v[6] * b0) / det;
            result.m.v[13] = (matrix.m.v[0] * b3 - matrix.m.v[1] * b1 + matrix.m.v[2] * b0) / det;
            result.m.v[14] = -(matrix.m.v[12] * a3 - matrix.m.v[13] * a1 + matrix.m.v[14] * a0) / det;
            result.m.v[15] = (matrix.m.v[8] * a3 - matrix.m.v[9] * a1 + matrix.m.v[10] * a0) / det;
        }

        return result;
    }

    template <typename T>
    [[nodiscard]] constexpr auto getTranslation(const Matrix<T, 3, 3>& matrix) noexcept
    {
        return Vector<T, 2>{matrix.m.v[6], matrix.m.v[7]};
    }

    template <typename T>
    [[nodiscard]] constexpr auto getTranslation(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{matrix.m.v[12], matrix.m.v[13], matrix.m.v[14]};
    }

    template <typename T>
    [[nodiscard]] auto getScale(const Matrix<T, 3, 3>& matrix) noexcept
    {
        Vector<T, 2> scale;
        scale.v[0] = Vector<T, 2>{matrix.m.v[0], matrix.m.v[1]}.length();
        scale.v[1] = Vector<T, 2>{matrix.m.v[3], matrix.m.v[4]}.length();

        return scale;
    }

    template <typename T>
    [[nodiscard]] auto getScale(const Matrix<T, 4, 4>& matrix) noexcept
    {
        Vector<T, 3> scale;
        scale.v[0] = Vector<T, 3>{matrix.m.v[0], matrix.m.v[1], matrix.m.v[2]}.length();
        scale.v[1] = Vector<T, 3>{matrix.m.v[4], matrix.m.v[5], matrix.m.v[6]}.length();
        scale.v[2] = Vector<T, 3>{matrix.m.v[8], matrix.m.v[9], matrix.m.v[10]}.length();

        return scale;
    }

    template <typename T>
    [[nodiscard]] auto getRotation(const Matrix<T, 3, 3>& matrix) noexcept
    {
        return std::atan2(-matrix.m.v[3], matrix.m.v[0]);
    }

    template <typename T>
    [[nodiscard]] auto getRotation(const Matrix<T, 4, 4>& matrix) noexcept
    {
        const auto scale = getScale(matrix);

        const auto m11 = matrix.m.v[0] / scale.v[0];
        const auto m21 = matrix.m.v[1] / scale.v[0];
        const auto m31 = matrix.m.v[2] / scale.v[0];

        const auto m12 = matrix.m.v[4] / scale.v[1];
        const auto m22 = matrix.m.v[5] / scale.v[1];
        const auto m32 = matrix.m.v[6] / scale.v[1];

        const auto m13 = matrix.m.v[8] / scale.v[2];
        const auto m23 = matrix.m.v[9] / scale.v[2];
        const auto m33 = matrix.m.v[10] / scale.v[2];

        Quaternion<T> result{
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

    template <typename T>
    [[nodiscard]] auto getUpVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{matrix.m.v[4], matrix.m.v[5], matrix.m.v[6]};
    }

    template <typename T>
    [[nodiscard]] auto getDownVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{-matrix.m.v[4], -matrix.m.v[5], -matrix.m.v[6]};
    }

    template <typename T>
    [[nodiscard]] auto getLeftVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{-matrix.m.v[0], -matrix.m.v[1], -matrix.m.v[2]};
    }

    template <typename T>
    [[nodiscard]] auto getRightVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{matrix.m.v[0], matrix.m.v[1], matrix.m.v[2]};
    }

    template <typename T>
    [[nodiscard]] auto getForwardVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{-matrix.m.v[8], -matrix.m.v[9], -matrix.m.v[10]};
    }

    template <typename T>
    [[nodiscard]] auto getBackVector(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return Vector<T, 3>{matrix.m.v[8], matrix.m.v[9], matrix.m.v[10]};
    }

    template <typename T>
    [[nodiscard]] auto getFrustumLeftPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] + matrix.m.v[0],
                                   matrix.m.v[7] + matrix.m.v[4],
                                   matrix.m.v[11] + matrix.m.v[8],
                                   matrix.m.v[15] + matrix.m.v[12]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumRightPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] - matrix.m.v[0],
                                   matrix.m.v[7] - matrix.m.v[4],
                                   matrix.m.v[11] - matrix.m.v[8],
                                   matrix.m.v[15] - matrix.m.v[12]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumBottomPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] + matrix.m.v[1],
                                   matrix.m.v[7] + matrix.m.v[5],
                                   matrix.m.v[11] + matrix.m.v[9],
                                   matrix.m.v[15] + matrix.m.v[13]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumTopPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] - matrix.m.v[1],
                                   matrix.m.v[7] - matrix.m.v[5],
                                   matrix.m.v[11] - matrix.m.v[9],
                                   matrix.m.v[15] - matrix.m.v[13]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumNearPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] + matrix.m.v[2],
                                   matrix.m.v[7] + matrix.m.v[6],
                                   matrix.m.v[11] + matrix.m.v[10],
                                   matrix.m.v[15] + matrix.m.v[14]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustumFarPlane(const Matrix<T, 4, 4>& matrix) noexcept
    {
        return makeFrustumPlane<T>(matrix.m.v[3] - matrix.m.v[2],
                                   matrix.m.v[7] - matrix.m.v[6],
                                   matrix.m.v[11] - matrix.m.v[10],
                                   matrix.m.v[15] - matrix.m.v[14]);
    }

    template <typename T>
    [[nodiscard]] auto getFrustum(const Matrix<T, 4, 4>& matrix) noexcept(false)
    {
        return ConvexVolume<T>{{
            getFrustumLeftPlane(matrix),
            getFrustumRightPlane(matrix),
            getFrustumBottomPlane(matrix),
            getFrustumTopPlane(matrix),
            getFrustumNearPlane(matrix),
            getFrustumFarPlane(matrix)
        }};
    }

    template <typename T>
    void setLookAt(Matrix<T, 4, 4>& matrix,
                   const Vector<T, 3>& eyePosition,
                   const Vector<T, 3>& targetPosition,
                   const Vector<T, 3>& upVector) noexcept
    {
        const auto up = normalized(upVector);
        const auto zaxis = normalized(targetPosition - eyePosition);
        const auto xaxis = normalized(cross(up, zaxis));
        const auto yaxis = normalized(cross(zaxis, xaxis));

        // row 1
        matrix.m.v[0] = xaxis.v[0];
        matrix.m.v[4] = xaxis.v[1];
        matrix.m.v[8] = xaxis.v[2];
        matrix.m.v[12] = dot(xaxis, -eyePosition);

        // row 2
        matrix.m.v[1] = yaxis.v[0];
        matrix.m.v[5] = yaxis.v[1];
        matrix.m.v[9] = yaxis.v[2];
        matrix.m.v[13] = dot(yaxis, -eyePosition);

        // row 3
        matrix.m.v[2] = zaxis.v[0];
        matrix.m.v[6] = zaxis.v[1];
        matrix.m.v[10] = zaxis.v[2];
        matrix.m.v[14] = dot(zaxis, -eyePosition);

        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = T(0);
        matrix.m.v[15] = T(1);
    }

    template <typename T>
    void setPerspective(Matrix<T, 4, 4>& matrix,
                        const T fieldOfView, const T aspectRatio,
                        const T nearClip, const T farClip) noexcept
    {
        assert(aspectRatio);
        assert(farClip != nearClip);

        const auto theta = fieldOfView / T(2);
        if (std::fabs(std::fmod(theta, tau<T> / T(4))) <= std::numeric_limits<T>::epsilon())
            return;

        const auto divisor = std::tan(theta);
        assert(divisor);
        const auto factor = T(1) / divisor;

        // row 1
        matrix.m.v[0] = factor / aspectRatio;
        matrix.m.v[4] = T(0);
        matrix.m.v[8] = T(0);
        matrix.m.v[12] = T(0);
        // row 2
        matrix.m.v[1] = T(0);
        matrix.m.v[5] = factor;
        matrix.m.v[9] = T(0);
        matrix.m.v[13] = T(0);
        // row 3
        matrix.m.v[2] = T(0);
        matrix.m.v[6] = T(0);
        matrix.m.v[10] = -(farClip + nearClip) / (farClip - nearClip);
        matrix.m.v[14] = -T(2) * farClip * nearClip / (farClip - nearClip);
        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = -T(1);
        matrix.m.v[15] = T(0);
    }

    template <typename T>
    void setOrthographic(Matrix<T, 4, 4>& matrix,
                         const T width, const T height,
                         const T nearClip, const T farClip) noexcept
    {
        assert(width);
        assert(height);
        assert(farClip != nearClip);

        // row 1
        matrix.m.v[0] = T(2) / width;
        matrix.m.v[4] = T(0);
        matrix.m.v[8] = T(0);
        matrix.m.v[12] = T(0);
        // row 2
        matrix.m.v[1] = T(0);
        matrix.m.v[5] = T(2) / height;
        matrix.m.v[9] = T(0);
        matrix.m.v[13] = T(0);
        // row 3
        matrix.m.v[2] = T(0);
        matrix.m.v[6] = T(0);
        matrix.m.v[10] = T(1) / (farClip - nearClip);
        matrix.m.v[14] = nearClip / (nearClip - farClip);
        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = T(0);
        matrix.m.v[15] = T(1);
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

        // row 1
        matrix.m.v[0] = T(2) / (right - left);
        matrix.m.v[4] = T(0);
        matrix.m.v[8] = T(0);
        matrix.m.v[12] = (left + right) / (left - right);
        // row 2
        matrix.m.v[1] = T(0);
        matrix.m.v[5] = T(2) / (top - bottom);
        matrix.m.v[9] = T(0);
        matrix.m.v[13] = (bottom + top) / (bottom - top);
        // row 3
        matrix.m.v[2] = T(0);
        matrix.m.v[6] = T(0);
        matrix.m.v[10] = T(1) / (farClip - nearClip);
        matrix.m.v[14] = nearClip / (nearClip - farClip);
        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = T(0);
        matrix.m.v[15] = T(1);
    }

    template <typename T, std::size_t size>
    void setScale(Matrix<T, size, size>& matrix,
                  const T scale) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < size - 1; ++i)
            matrix.m.v[i * size + i] = scale;
    }

    template <typename T, std::size_t size>
    void setScale(Matrix<T, size, size>& matrix,
                  const Vector<T, size - 1>& scale) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < size - 1; ++i)
            matrix.m.v[i * size + i] = scale[i];
    }

    template <typename T>
    void setRotation(Matrix<T, 3, 3>& matrix,
                     const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        // row 1
        matrix.m.v[0] = cosine;
        matrix.m.v[3] = -sine;
        // row 2
        matrix.m.v[1] = sine;
        matrix.m.v[4] = cosine;
    }

    template <typename T>
    void setRotation(Matrix<T, 4, 4>& matrix,
                     const Vector<T, 3>& axis, T angle) noexcept
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

        // row 1
        matrix.m.v[0] = cosine + tx * x;
        matrix.m.v[4] = txy - sz;
        matrix.m.v[8] = txz + sy;
        matrix.m.v[12] = T(0);

        // row 2
        matrix.m.v[1] = txy + sz;
        matrix.m.v[5] = cosine + ty * y;
        matrix.m.v[9] = tyz - sx;
        matrix.m.v[13] = T(0);

        // row 3
        matrix.m.v[2] = txz - sy;
        matrix.m.v[6] = tyz + sx;
        matrix.m.v[10] = cosine + tz * z;
        matrix.m.v[14] = T(0);

        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = T(0);
        matrix.m.v[15] = T(1);
    }

    template <typename T>
    void setRotation(Matrix<T, 4, 4>& matrix,
                     const Quaternion<T>& rotation) noexcept
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

        // row 1
        matrix.m.v[0] = T(1) - T(2) * (yy + zz);
        matrix.m.v[4] = T(2) * (xy - wz);
        matrix.m.v[8] = T(2) * (xz + wy);
        matrix.m.v[12] = T(0);

        // row 2
        matrix.m.v[1] = T(2) * (xy + wz);
        matrix.m.v[5] = T(1) - T(2) * (xx + zz);
        matrix.m.v[9] = T(2) * (yz - wx);
        matrix.m.v[13] = T(0);

        // row 3
        matrix.m.v[2] = T(2) * (xz - wy);
        matrix.m.v[6] = T(2) * (yz + wx);
        matrix.m.v[10] = T(1) - T(2) * (xx + yy);
        matrix.m.v[14] = T(0);

        // row 4
        matrix.m.v[3] = T(0);
        matrix.m.v[7] = T(0);
        matrix.m.v[11] = T(0);
        matrix.m.v[15] = T(1);
    }

    template <typename T>
    void setRotationX(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        // row 2
        matrix.m.v[5] = cosine;
        matrix.m.v[9] = -sine;
        // row 3
        matrix.m.v[6] = sine;
        matrix.m.v[10] = cosine;
    }

    template <typename T>
    void setRotationY(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        // row 1
        matrix.m.v[0] = cosine;
        matrix.m.v[8] = sine;
        // row 3
        matrix.m.v[2] = -sine;
        matrix.m.v[10] = cosine;
    }

    template <typename T>
    void setRotationZ(Matrix<T, 4, 4>& matrix,
                      const T angle) noexcept
    {
        setIdentity(matrix);

        const auto cosine = std::cos(angle);
        const auto sine = std::sin(angle);

        // row 1
        matrix.m.v[0] = cosine;
        matrix.m.v[4] = -sine;
        // row 2
        matrix.m.v[1] = sine;
        matrix.m.v[5] = cosine;
    }

    template <typename T>
    void setTranslation(Matrix<T, 4, 4>& matrix,
                        const Vector<T, 3>& translation) noexcept
    {
        setIdentity(matrix);

        for (std::size_t i = 0; i < 3; ++i)
            matrix.m.v[3 * 4 + i] = translation[i];
    }

    template <typename T>
    void transformPoint(const Matrix<T, 4, 4>& matrix,
                        Vector<T, 3>& point) noexcept
    {
        const auto w = matrix.m.v[0 * 4 + 3] * point.v[0] + matrix.m.v[1 * 4 + 3] * point.v[1] + matrix.m.v[2 * 4 + 3] * point.v[2] + matrix.m.v[3 * 4 + 3];

        assert(w != T(0));

        point = Vector<T, 3>{
            (matrix.m.v[0 * 4 + 0] * point.v[0] + matrix.m.v[1 * 4 + 0] * point.v[1] + matrix.m.v[2 * 4 + 0] * point.v[2] + matrix.m.v[3 * 4 + 0]) / w,
            (matrix.m.v[0 * 4 + 1] * point.v[0] + matrix.m.v[1 * 4 + 1] * point.v[1] + matrix.m.v[2 * 4 + 1] * point.v[2] + matrix.m.v[3 * 4 + 1]) / w,
            (matrix.m.v[0 * 4 + 2] * point.v[0] + matrix.m.v[1 * 4 + 2] * point.v[1] + matrix.m.v[2 * 4 + 2] * point.v[2] + matrix.m.v[3 * 4 + 2]) / w
        };
    }
}

#include "MatrixNeon.hpp"
#include "MatrixSse.hpp"

#endif // OUZEL_MATH_MATRIX_HPP
