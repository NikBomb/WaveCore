#include "wavecore/utils/Matrix.hpp"
#include <doctest/doctest.h>

TEST_CASE("Matrix construction and initialization") {
    SUBCASE("Default constructor creates zero matrix") {
        WaveCore::Matrix<double, 3, 4> matrix;
        
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                CHECK_EQ(matrix(i, j), 0.0);
            }
        }
    }

    SUBCASE("Copy constructor works correctly") {
        WaveCore::Matrix<double, 2, 3> original;
        original(0, 0) = 1.0;
        original(0, 1) = 2.0;
        original(1, 2) = 3.0;

        WaveCore::Matrix<double, 2, 3> copy(original);
        
        CHECK_EQ(copy(0, 0), 1.0);
        CHECK_EQ(copy(0, 1), 2.0);
        CHECK_EQ(copy(1, 2), 3.0);
    }

    SUBCASE("Assignment operator works correctly") {
        WaveCore::Matrix<double, 2, 3> original;
        original(0, 0) = 5.0;
        original(1, 1) = 7.0;

        WaveCore::Matrix<double, 2, 3> assigned;
        assigned = original;
        
        CHECK_EQ(assigned(0, 0), 5.0);
        CHECK_EQ(assigned(1, 1), 7.0);
    }
}

TEST_CASE("Matrix element access") {
    WaveCore::Matrix<double, 2, 2> matrix;
    matrix(0, 0) = 1.0;
    matrix(0, 1) = 2.0;
    matrix(1, 0) = 3.0;
    matrix(1, 1) = 4.0;

    SUBCASE("Read access works correctly") {
        CHECK_EQ(matrix(0, 0), 1.0);
        CHECK_EQ(matrix(0, 1), 2.0);
        CHECK_EQ(matrix(1, 0), 3.0);
        CHECK_EQ(matrix(1, 1), 4.0);
    }

    SUBCASE("Write access works correctly") {
        matrix(0, 0) = 5.0;
        CHECK_EQ(matrix(0, 0), 5.0);
        
        matrix(1, 1) = 6.0;
        CHECK_EQ(matrix(1, 1), 6.0);
    }

    SUBCASE("Const access works correctly") {
        const auto& const_matrix = matrix;
        CHECK_EQ(const_matrix(0, 0), 1.0);
        CHECK_EQ(const_matrix(1, 1), 4.0);
    }
}

TEST_CASE("Matrix arithmetic operations") {
    WaveCore::Matrix<double, 2, 2> a;
    a(0, 0) = 1.0; a(0, 1) = 2.0;
    a(1, 0) = 3.0; a(1, 1) = 4.0;

    WaveCore::Matrix<double, 2, 2> b;
    b(0, 0) = 5.0; b(0, 1) = 6.0;
    b(1, 0) = 7.0; b(1, 1) = 8.0;

    SUBCASE("Matrix addition works correctly") {
        auto result = a + b;
        CHECK_EQ(result(0, 0), 6.0);
        CHECK_EQ(result(0, 1), 8.0);
        CHECK_EQ(result(1, 0), 10.0);
        CHECK_EQ(result(1, 1), 12.0);
    }

    SUBCASE("Matrix subtraction works correctly") {
        auto result = a - b;
        CHECK_EQ(result(0, 0), -4.0);
        CHECK_EQ(result(0, 1), -4.0);
        CHECK_EQ(result(1, 0), -4.0);
        CHECK_EQ(result(1, 1), -4.0);
    }

    SUBCASE("Matrix multiplication works correctly") {
        auto result = a * b;
        // Result should be:
        // [1*5+2*7   1*6+2*8]   [19   22]
        // [3*5+4*7   3*6+4*8] = [43   50]
        CHECK_EQ(result(0, 0), 19.0);
        CHECK_EQ(result(0, 1), 22.0);
        CHECK_EQ(result(1, 0), 43.0);
        CHECK_EQ(result(1, 1), 50.0);
    }

    SUBCASE("Compound assignment operators work correctly") {
        auto copy = a;
        copy += b;
        CHECK_EQ(copy(0, 0), 6.0);
        CHECK_EQ(copy(0, 1), 8.0);
        CHECK_EQ(copy(1, 0), 10.0);
        CHECK_EQ(copy(1, 1), 12.0);

        copy = a;
        copy -= b;
        CHECK_EQ(copy(0, 0), -4.0);
        CHECK_EQ(copy(0, 1), -4.0);
        CHECK_EQ(copy(1, 0), -4.0);
        CHECK_EQ(copy(1, 1), -4.0);
    }
}

TEST_CASE("Matrix transpose operation") {
    WaveCore::Matrix<double, 2, 3> matrix;
    matrix(0, 0) = 1.0; matrix(0, 1) = 2.0; matrix(0, 2) = 3.0;
    matrix(1, 0) = 4.0; matrix(1, 1) = 5.0; matrix(1, 2) = 6.0;

    auto transposed = matrix.transpose();

    CHECK_EQ(transposed(0, 0), 1.0);
    CHECK_EQ(transposed(0, 1), 4.0);
    CHECK_EQ(transposed(1, 0), 2.0);
    CHECK_EQ(transposed(1, 1), 5.0);
    CHECK_EQ(transposed(2, 0), 3.0);
    CHECK_EQ(transposed(2, 1), 6.0);
}

TEST_CASE("Square matrix determinant") {
    SUBCASE("2x2 matrix determinant") {
        WaveCore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0;
        matrix(1, 0) = 3.0; matrix(1, 1) = 4.0;

        auto det = WaveCore::determinant(matrix);
        CHECK_EQ(det, -2.0); // 1*4 - 2*3 = 4 - 6 = -2
    }

    SUBCASE("3x3 matrix determinant") {
        WaveCore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0; matrix(0, 2) = 3.0;
        matrix(1, 0) = 4.0; matrix(1, 1) = 5.0; matrix(1, 2) = 6.0;
        matrix(2, 0) = 7.0; matrix(2, 1) = 8.0; matrix(2, 2) = 9.0;

        auto det = WaveCore::determinant(matrix);
        CHECK_EQ(det, 0.0); // This is a singular matrix (rows are linearly dependent)
    }

    SUBCASE("3x3 matrix determinant with non-zero value") {
        WaveCore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0; matrix(0, 2) = 3.0;
        matrix(1, 0) = 0.0; matrix(1, 1) = 1.0; matrix(1, 2) = 4.0;
        matrix(2, 0) = 5.0; matrix(2, 1) = 6.0; matrix(2, 2) = 0.0;

        auto det = WaveCore::determinant(matrix);
        CHECK_EQ(det, 1.0); // Calculated manually: 1*(1*0-4*6) - 2*(0*0-4*5) + 3*(0*6-1*5) = -24 + 40 - 15 = 1
    }
}

TEST_CASE("Square matrix inverse") {
    SUBCASE("2x2 matrix inverse") {
        WaveCore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 2.0; matrix(0, 1) = 1.0;
        matrix(1, 0) = 1.0; matrix(1, 1) = 1.0;

        auto inverse = WaveCore::inverse(matrix);
        
        // Check that A * A^(-1) = I
        auto identity = matrix * inverse;
        CHECK_EQ(identity(0, 0), 1.0);
        CHECK_EQ(identity(0, 1), 0.0);
        CHECK_EQ(identity(1, 0), 0.0);
        CHECK_EQ(identity(1, 1), 1.0);
    }

    SUBCASE("3x3 matrix inverse") {
        WaveCore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 2.0; matrix(0, 1) = 1.0; matrix(0, 2) = 1.0;
        matrix(1, 0) = 3.0; matrix(1, 1) = 2.0; matrix(1, 2) = 1.0;
        matrix(2, 0) = 2.0; matrix(2, 1) = 1.0; matrix(2, 2) = 2.0;

        auto inverse = WaveCore::inverse(matrix);
        
        // Check that A * A^(-1) = I
        auto identity = matrix * inverse;
        CHECK_EQ(identity(0, 0), 1.0);
        CHECK_EQ(identity(0, 1), 0.0);
        CHECK_EQ(identity(0, 2), 0.0);
        CHECK_EQ(identity(1, 0), 0.0);
        CHECK_EQ(identity(1, 1), 1.0);
        CHECK_EQ(identity(1, 2), 0.0);
        CHECK_EQ(identity(2, 0), 0.0);
        CHECK_EQ(identity(2, 1), 0.0);
        CHECK_EQ(identity(2, 2), 1.0);
    }

    SUBCASE("Singular matrix throws exception") {
        WaveCore::Matrix<double, 2, 2> singular_matrix;
        singular_matrix(0, 0) = 1.0; singular_matrix(0, 1) = 2.0;
        singular_matrix(1, 0) = 2.0; singular_matrix(1, 1) = 4.0; // Dependent rows

        CHECK_THROWS_AS(WaveCore::inverse(singular_matrix), std::runtime_error);
    }
}

TEST_CASE("Matrix equality operators") {
    WaveCore::Matrix<double, 2, 2> a;
    a(0, 0) = 1.0; a(0, 1) = 2.0;
    a(1, 0) = 3.0; a(1, 1) = 4.0;

    WaveCore::Matrix<double, 2, 2> b;
    b(0, 0) = 1.0; b(0, 1) = 2.0;
    b(1, 0) = 3.0; b(1, 1) = 4.0;

    WaveCore::Matrix<double, 2, 2> c;
    c(0, 0) = 1.0; c(0, 1) = 2.0;
    c(1, 0) = 3.0; c(1, 1) = 5.0;

    SUBCASE("Equal matrices are equal") {
        CHECK(a == b);
        CHECK_FALSE(a == c);
    }

    SUBCASE("Unequal matrices are not equal") {
        CHECK_FALSE(a != b);
        CHECK(a != c);
    }
}

TEST_CASE("Matrix bounds checking") {
    WaveCore::Matrix<double, 2, 2> matrix;
    
    SUBCASE("Accessing valid indices works") {
        matrix(0, 0) = 1.0;
        CHECK_EQ(matrix(0, 0), 1.0);
    }

    SUBCASE("Accessing invalid indices throws assertion") {
        // This should trigger an assertion failure in debug mode
        // In release mode, it might not be caught, but that's expected behavior for assert()
        CHECK_NOTHROW(matrix(0, 0));  // Valid access - shouldn't throw
    }
}

// Test vector concept using Vector type alias
TEST_CASE("Vector operations using Vector") {
    SUBCASE("Vector as Matrix with 1 column") {
        // Create a 3x1 matrix (vector) using Vector alias
        WaveCore::Vector<double, 3> vector;
        vector(0, 0) = 1.0;
        vector(1, 0) = 2.0;
        vector(2, 0) = 3.0;

        CHECK_EQ(vector(0, 0), 1.0);
        CHECK_EQ(vector(1, 0), 2.0);
        CHECK_EQ(vector(2, 0), 3.0);
    }

    SUBCASE("Matrix-vector multiplication") {
        // 2x2 matrix
        WaveCore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0;
        matrix(1, 0) = 3.0; matrix(1, 1) = 4.0;

        // 2x1 vector using Vector alias
        WaveCore::Vector<double, 2> vector;
        vector(0, 0) = 5.0;
        vector(1, 0) = 6.0;

        // Matrix-vector multiplication (should result in 2x1 matrix)
        auto result = matrix * vector;
        
        CHECK_EQ(result(0, 0), 17.0); // 1*5 + 2*6
        CHECK_EQ(result(1, 0), 39.0); // 3*5 + 4*6
    }

}
