#include "wavecore/utils/Matrix.hpp"
#include <doctest/doctest.h>

TEST_CASE("Matrix initializer list construction") {
    SUBCASE("2x3 matrix from initializer list") {
        wavecore::Matrix<double, 2, 3> matrix = { 
            {1.0, 2.0, 3.0}, 
            {4.0, 5.0, 6.0} 
        };
        
        CHECK_EQ(matrix(0, 0), 1.0);
        CHECK_EQ(matrix(0, 1), 2.0);
        CHECK_EQ(matrix(0, 2), 3.0);
        CHECK_EQ(matrix(1, 0), 4.0);
        CHECK_EQ(matrix(1, 1), 5.0);
        CHECK_EQ(matrix(1, 2), 6.0);
    }

    SUBCASE("3x2 matrix from initializer list") {
        wavecore::Matrix<float, 3, 2> matrix = { 
            {1.5f, 2.5f}, 
            {3.5f, 4.5f},
            {5.5f, 6.5f}
        };
        
        CHECK_EQ(matrix(0, 0), 1.5f);
        CHECK_EQ(matrix(0, 1), 2.5f);
        CHECK_EQ(matrix(1, 0), 3.5f);
        CHECK_EQ(matrix(1, 1), 4.5f);
        CHECK_EQ(matrix(2, 0), 5.5f);
        CHECK_EQ(matrix(2, 1), 6.5f);
    }

    SUBCASE("1x4 matrix (row vector) from initializer list") {
        wavecore::Matrix<int, 1, 4> matrix = { {1, 2, 3, 4} };
        
        CHECK_EQ(matrix(0, 0), 1);
        CHECK_EQ(matrix(0, 1), 2);
        CHECK_EQ(matrix(0, 2), 3);
        CHECK_EQ(matrix(0, 3), 4);
    }

    SUBCASE("4x1 matrix (column vector) from initializer list") {
        wavecore::Matrix<double, 4, 1> matrix = { {1.0}, {2.0}, {3.0}, {4.0} };
        
        CHECK_EQ(matrix(0, 0), 1.0);
        CHECK_EQ(matrix(1, 0), 2.0);
        CHECK_EQ(matrix(2, 0), 3.0);
        CHECK_EQ(matrix(3, 0), 4.0);
    }

    SUBCASE("2x2 identity matrix from initializer list") {
        wavecore::Matrix<double, 2, 2> matrix = { 
            {1.0, 0.0}, 
            {0.0, 1.0} 
        };
        
        CHECK_EQ(matrix(0, 0), 1.0);
        CHECK_EQ(matrix(0, 1), 0.0);
        CHECK_EQ(matrix(1, 0), 0.0);
        CHECK_EQ(matrix(1, 1), 1.0);
    }
}

TEST_CASE("Matrix initializer list with Vector alias") {
    SUBCASE("Vector construction from initializer list") {
        // Using Vector alias (which is Matrix<T, N, 1>)
        wavecore::Vector<double, 3> vector = { 1.0 , 2.0, 3.0 };
        
        CHECK_EQ(vector(0), 1.0);
        CHECK_EQ(vector(1), 2.0);
        CHECK_EQ(vector(2), 3.0);
    }
}
