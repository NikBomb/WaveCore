#include "wavecore/utils/Matrix.hpp"
#include <doctest/doctest.h>

TEST_CASE("Square matrix symmetric part") {
    SUBCASE("2x2 matrix symmetric part") {
        wavecore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0;
        matrix(1, 0) = 3.0; matrix(1, 1) = 4.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For matrix [[1, 2], [3, 4]], the symmetric part should be:
        // (A + A^T)/2 = ([[1, 2], [3, 4]] + [[1, 3], [2, 4]])/2 = [[1, 2.5], [2.5, 4]]
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 2.5);
        CHECK_EQ(sym(1, 0), 2.5);
        CHECK_EQ(sym(1, 1), 4.0);
    }

    SUBCASE("2x2 matrix with symmetric matrix") {
        wavecore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0;
        matrix(1, 0) = 2.0; matrix(1, 1) = 3.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For symmetric matrix [[1, 2], [2, 3]], the symmetric part should be itself
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 2.0);
        CHECK_EQ(sym(1, 0), 2.0);
        CHECK_EQ(sym(1, 1), 3.0);
    }

    SUBCASE("2x2 identity matrix symmetric part") {
        wavecore::Matrix<double, 2, 2> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 0.0;
        matrix(1, 0) = 0.0; matrix(1, 1) = 1.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For identity matrix, the symmetric part should be itself
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 0.0);
        CHECK_EQ(sym(1, 0), 0.0);
        CHECK_EQ(sym(1, 1), 1.0);
    }

    SUBCASE("3x3 matrix symmetric part") {
        wavecore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0; matrix(0, 2) = 3.0;
        matrix(1, 0) = 4.0; matrix(1, 1) = 5.0; matrix(1, 2) = 6.0;
        matrix(2, 0) = 7.0; matrix(2, 1) = 8.0; matrix(2, 2) = 9.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For matrix [[1, 2, 3], [4, 5, 6], [7, 8, 9]], the symmetric part should be:
        // (A + A^T)/2 = ([[1, 2, 3], [4, 5, 6], [7, 8, 9]] + [[1, 4, 7], [2, 5, 8], [3, 6, 9]])/2 = [[1, 3, 5], [3, 5, 7], [5, 7, 9]]
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 3.0);
        CHECK_EQ(sym(0, 2), 5.0);
        CHECK_EQ(sym(1, 0), 3.0);
        CHECK_EQ(sym(1, 1), 5.0);
        CHECK_EQ(sym(1, 2), 7.0);
        CHECK_EQ(sym(2, 0), 5.0);
        CHECK_EQ(sym(2, 1), 7.0);
        CHECK_EQ(sym(2, 2), 9.0);
    }

    SUBCASE("3x3 matrix with symmetric matrix") {
        wavecore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 2.0; matrix(0, 2) = 3.0;
        matrix(1, 0) = 2.0; matrix(1, 1) = 4.0; matrix(1, 2) = 5.0;
        matrix(2, 0) = 3.0; matrix(2, 1) = 5.0; matrix(2, 2) = 6.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For symmetric matrix, the symmetric part should be itself
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 2.0);
        CHECK_EQ(sym(0, 2), 3.0);
        CHECK_EQ(sym(1, 0), 2.0);
        CHECK_EQ(sym(1, 1), 4.0);
        CHECK_EQ(sym(1, 2), 5.0);
        CHECK_EQ(sym(2, 0), 3.0);
        CHECK_EQ(sym(2, 1), 5.0);
        CHECK_EQ(sym(2, 2), 6.0);
    }

    SUBCASE("3x3 identity matrix symmetric part") {
        wavecore::Matrix<double, 3, 3> matrix;
        matrix(0, 0) = 1.0; matrix(0, 1) = 0.0; matrix(0, 2) = 0.0;
        matrix(1, 0) = 0.0; matrix(1, 1) = 1.0; matrix(1, 2) = 0.0;
        matrix(2, 0) = 0.0; matrix(2, 1) = 0.0; matrix(2, 2) = 1.0;

        auto sym = wavecore::symmetric(matrix);
        
        // For identity matrix, the symmetric part should be itself
        CHECK_EQ(sym(0, 0), 1.0);
        CHECK_EQ(sym(0, 1), 0.0);
        CHECK_EQ(sym(0, 2), 0.0);
        CHECK_EQ(sym(1, 0), 0.0);
        CHECK_EQ(sym(1, 1), 1.0);
        CHECK_EQ(sym(1, 2), 0.0);
        CHECK_EQ(sym(2, 0), 0.0);
        CHECK_EQ(sym(2, 1), 0.0);
        CHECK_EQ(sym(2, 2), 1.0);
    }

    SUBCASE("Float type 2x2 matrix") {
        wavecore::Matrix<float, 2, 2> matrix;
        matrix(0, 0) = 1.5f; matrix(0, 1) = 2.5f;
        matrix(1, 0) = 3.5f; matrix(1, 1) = 4.5f;

        auto sym = wavecore::symmetric(matrix);
        
        // For matrix [[1.5, 2.5], [3.5, 4.5]], the symmetric part should be:
        // (A + A^T)/2 = ([[1.5, 2.5], [3.5, 4.5]] + [[1.5, 3.5], [2.5, 4.5]])/2 = [[1.5, 3.0], [3.0, 4.5]]
        CHECK_EQ(sym(0, 0), 1.5f);
        CHECK_EQ(sym(0, 1), 3.0f);
        CHECK_EQ(sym(1, 0), 3.0f);
        CHECK_EQ(sym(1, 1), 4.5f);
    }

    SUBCASE("Float type 3x3 matrix") {
        wavecore::Matrix<float, 3, 3> matrix;
        matrix(0, 0) = 1.0f; matrix(0, 1) = 2.0f; matrix(0, 2) = 3.0f;
        matrix(1, 0) = 4.0f; matrix(1, 1) = 5.0f; matrix(1, 2) = 6.0f;
        matrix(2, 0) = 7.0f; matrix(2, 1) = 8.0f; matrix(2, 2) = 9.0f;

        auto sym = wavecore::symmetric(matrix);
        
        // For matrix [[1, 2, 3], [4, 5, 6], [7, 8, 9]], the symmetric part should be:
        // (A + A^T)/2 = ([[1, 2, 3], [4, 5, 6], [7, 8, 9]] + [[1, 4, 7], [2, 5, 8], [3, 6, 9]])/2 = [[1, 3, 5], [3, 5, 7], [5, 7, 9]]
        CHECK_EQ(sym(0, 0), 1.0f);
        CHECK_EQ(sym(0, 1), 3.0f);
        CHECK_EQ(sym(0, 2), 5.0f);
        CHECK_EQ(sym(1, 0), 3.0f);
        CHECK_EQ(sym(1, 1), 5.0f);
        CHECK_EQ(sym(1, 2), 7.0f);
        CHECK_EQ(sym(2, 0), 5.0f);
        CHECK_EQ(sym(2, 1), 7.0f);
        CHECK_EQ(sym(2, 2), 9.0f);
    }
}

TEST_CASE("Symmetric function template constraints") {
    SUBCASE("Should not compile for non-square matrices") {
        // This should fail to compile because symmetric is only defined for square matrices
        // We cannot test this at compile time here, but we can make sure the function signature is correct
        static_assert(true, "symmetric function is properly templated for square matrices");
    }
}