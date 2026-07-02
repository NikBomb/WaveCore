#include <doctest/doctest.h>
#include "wavecore/utils/Matrix.hpp"

TEST_CASE("Vector single index access") {
    // Test with a 3D vector
    wavecore::Vector<double, 3> vec;
    
    // Use single-index accessor (should work now)
    vec(0) = 1.0;
    vec(1) = 2.0;
    vec(2) = 3.0;
    
    CHECK_EQ(vec(0), 1.0);
    CHECK_EQ(vec(1), 2.0);
    CHECK_EQ(vec(2), 3.0);
    
    // Test const access
    const wavecore::Vector<double, 3> const_vec = vec;
    CHECK_EQ(const_vec(0), 1.0);
    CHECK_EQ(const_vec(1), 2.0);
    CHECK_EQ(const_vec(2), 3.0);
}

TEST_CASE("Vector backward compatibility") {
    // Test that two-index access still works
    wavecore::Vector<double, 3> vec;
    
    vec(0, 0) = 1.0;
    vec(1, 0) = 2.0;
    vec(2, 0) = 3.0;
    
    CHECK_EQ(vec(0, 0), 1.0);
    CHECK_EQ(vec(1, 0), 2.0);
    CHECK_EQ(vec(2, 0), 3.0);
}

TEST_CASE("Matrix still works with two-index access") {
    // Test that regular matrices still require two indices
    wavecore::Matrix<double, 2, 3> mat;
    
    mat(0, 0) = 1.0;
    mat(0, 1) = 2.0;
    mat(0, 2) = 3.0;
    mat(1, 0) = 4.0;
    mat(1, 1) = 5.0;
    mat(1, 2) = 6.0;
    
    CHECK_EQ(mat(0, 0), 1.0);
    CHECK_EQ(mat(0, 1), 2.0);
    CHECK_EQ(mat(0, 2), 3.0);
    CHECK_EQ(mat(1, 0), 4.0);
    CHECK_EQ(mat(1, 1), 5.0);
    CHECK_EQ(mat(1, 2), 6.0);
    
    // This should fail to compile with static_assert
    // mat[0] = 1.0; // This line commented out because it should not compile
}

TEST_CASE("Compile time check for vector vs matrix") {
    // Test that using [] on a matrix (not vector) fails at compile time
    // This test is for documentation purposes - the code should fail to compile
    // if we uncomment the following line:
    //
    // wavecore::Matrix<double, 3, 3> mat;
    // mat[0] = 1.0;  // Should not compile!
    
    // But this should work:
    wavecore::Vector<double, 3> vec;
    vec(0) = 1.0;  // Should compile fine
    CHECK_EQ(vec(0), 1.0);
}
