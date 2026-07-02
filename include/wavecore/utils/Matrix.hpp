#ifndef WAVECORE_MATRIX_HPP
#define WAVECORE_MATRIX_HPP

#include <array>
#include <cassert>
#include <stdexcept>

namespace WaveCore {

/**
 * @brief A template matrix class for finite element computations
 * 
 * This class provides basic matrix operations with compile-time dimension checking.
 * Specialized functions are provided for square matrix operations like determinant and inverse.
 * 
 * @tparam T The numeric type (e.g., double, float)
 * @tparam Rows Number of rows
 * @tparam Cols Number of columns
 */
template<typename T, size_t Rows, size_t Cols>
class Matrix {
private:
    std::array<std::array<T, Cols>, Rows> data_;

public:
    // Constructors
    constexpr Matrix() : data_() {
        // Initialize all elements to zero
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                data_[i][j] = T{0};
            }
        }
    }

    constexpr Matrix(const Matrix& other) : data_(other.data_) {}

    // Assignment operator
    constexpr Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            data_ = other.data_;
        }
        return *this;
    }

    // Accessor methods
    constexpr T& operator()(size_t row, size_t col) {
        assert(row < Rows && col < Cols);
        return data_[row][col];
    }

    constexpr const T& operator()(size_t row, size_t col) const {
        assert(row < Rows && col < Cols);
        return data_[row][col];
    }

    // Basic arithmetic operations
    constexpr Matrix operator+(const Matrix& other) const {
        Matrix result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                result(i, j) = data_[i][j] + other.data_[i][j];
            }
        }
        return result;
    }

    constexpr Matrix operator-(const Matrix& other) const {
        Matrix result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                result(i, j) = data_[i][j] - other.data_[i][j];
            }
        }
        return result;
    }

    // Matrix multiplication with compile-time dimension checking
    template<size_t OtherCols>
    constexpr Matrix<T, Rows, OtherCols> operator*(const Matrix<T, Cols, OtherCols>& other) const {
        Matrix<T, Rows, OtherCols> result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < OtherCols; ++j) {
                T sum = T{0};
                for (size_t k = 0; k < Cols; ++k) {
                    sum += data_[i][k] * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

    // Transpose operation
    constexpr Matrix<T, Cols, Rows> transpose() const {
        Matrix<T, Cols, Rows> result;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                result(j, i) = data_[i][j];
            }
        }
        return result;
    }

    // Compound assignment operators
    constexpr Matrix& operator+=(const Matrix& other) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                data_[i][j] += other.data_[i][j];
            }
        }
        return *this;
    }

    constexpr Matrix& operator-=(const Matrix& other) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                data_[i][j] -= other.data_[i][j];
            }
        }
        return *this;
    }

    // Check if matrices are equal
    constexpr bool operator==(const Matrix& other) const {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                if (data_[i][j] != other.data_[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    // Check if matrices are not equal
    constexpr bool operator!=(const Matrix& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Specialized determinant function for square matrices (2x2 and 3x3)
 * 
 * @tparam T The numeric type
 * @tparam N Size of the square matrix (must be 2 or 3)
 * @param matrix Input square matrix
 * @return Determinant value
 */
template<typename T, size_t N>
constexpr T determinant(const Matrix<T, N, N>& matrix) {
    static_assert(N == 2 || N == 3, "Determinant is only implemented for 2x2 and 3x3 matrices");
    
    if constexpr (N == 2) {
        // For 2x2 matrix: det = ad - bc
        return matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0);
    } else {  // N == 3
        // For 3x3 matrix: det = a(ei − fh) − b(di − fg) + c(dh − eg)
        return matrix(0, 0) * (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1))
             - matrix(0, 1) * (matrix(1, 0) * matrix(2, 2) - matrix(1, 2) * matrix(2, 0))
             + matrix(0, 2) * (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0));
    }
}

/**
 * @brief Specialized inverse function for square matrices (2x2 and 3x3)
 * 
 * @tparam T The numeric type
 * @tparam N Size of the square matrix (must be 2 or 3)
 * @param matrix Input square matrix
 * @return Inverse matrix
 */
template<typename T, size_t N>
constexpr Matrix<T, N, N> inverse(const Matrix<T, N, N>& matrix) {
    static_assert(N == 2 || N == 3, "Inverse is only implemented for 2x2 and 3x3 matrices");
    
    if constexpr (N == 2) {
        // For 2x2 matrix: A^(-1) = (1/det(A)) * [d, -b; -c, a]
        T det = determinant(matrix);
        if (det == T{0}) {
            throw std::runtime_error("Matrix is singular and cannot be inverted");
        }
        
        Matrix<T, 2, 2> result;
        result(0, 0) = matrix(1, 1) / det;
        result(0, 1) = -matrix(0, 1) / det;
        result(1, 0) = -matrix(1, 0) / det;
        result(1, 1) = matrix(0, 0) / det;
        return result;
    } else {  // N == 3
        // For 3x3 matrix: A^(-1) = (1/det(A)) * adjugate(A)
        T det = determinant(matrix);
        if (det == T{0}) {
            throw std::runtime_error("Matrix is singular and cannot be inverted");
        }
        
        Matrix<T, 3, 3> result;
        // Calculate cofactors and transpose to get adjugate matrix
        result(0, 0) = (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1)) / det;
        result(1, 0) = (matrix(1, 2) * matrix(2, 0) - matrix(1, 0) * matrix(2, 2)) / det;
        result(2, 0) = (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0)) / det;
        result(0, 1) = (matrix(0, 2) * matrix(2, 1) - matrix(0, 1) * matrix(2, 2)) / det;
        result(1, 1) = (matrix(0, 0) * matrix(2, 2) - matrix(0, 2) * matrix(2, 0)) / det;
        result(2, 1) = (matrix(0, 1) * matrix(2, 0) - matrix(0, 0) * matrix(2, 1)) / det;
        result(0, 2) = (matrix(0, 1) * matrix(1, 2) - matrix(0, 2) * matrix(1, 1)) / det;
        result(1, 2) = (matrix(0, 2) * matrix(1, 0) - matrix(0, 0) * matrix(1, 2)) / det;
        result(2, 2) = (matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0)) / det;
        
        return result;
    }
}

/**
 * @brief Type alias for Vector as a specialization of Matrix with 1 column
 * 
 * This provides a convenient shorthand: Vector<T, N> instead of Matrix<T, N, 1>
 * 
 * @tparam T The numeric type (e.g., double, float)
 * @tparam N Number of rows (elements in the vector)
 */
template<typename T, size_t N>
using Vector = WaveCore::Matrix<T, N, 1>;

} // namespace WaveCore



#endif // WAVECORE_MATRIX_HPP
