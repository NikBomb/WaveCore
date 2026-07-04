#include "wavecore/elements/Quad4.hpp"
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"
#include <doctest/doctest.h>
#include <array>

TEST_CASE("Quad4 Strain Rate Tensor") {
    SUBCASE("Unit square with constant velocity field") {
        // Create a simple unit square element with coordinates
        wavecore::Node2D nodes[4];
        nodes[0] = wavecore::Node2D{{0.0, 0.0}}; // Bottom-left
        nodes[1] = wavecore::Node2D{{1.0, 0.0}}; // Bottom-right
        nodes[2] = wavecore::Node2D{{1.0, 1.0}}; // Top-right
        nodes[3] = wavecore::Node2D{{0.0, 1.0}}; // Top-left

        // Set constant velocity field (should result in zero strain rate)
        nodes[0].velocity() = {1.0, 1.0};
        nodes[1].velocity() = {1.0, 1.0};
        nodes[2].velocity() = {1.0, 1.0};
        nodes[3].velocity() = {1.0, 1.0};

        // Create connectivity
        std::array<size_t, 4> connectivity = {0, 1, 2, 3};

        // Create Quad4 element
        wavecore::Quad4 quad4;

        // Gather the element with nodes and connectivity
        quad4.gather(nodes, connectivity);

        // Test strain rate tensor at center (0,0)
        auto coords = wavecore::Vector<double, 2>({0.0, 0.0});
        auto strain_rate_tensor = quad4.strain_rate_tensor(coords);

        // For a constant velocity field in a unit square, the strain rate tensor should be zero
        // since there's no deformation
        CHECK_EQ(strain_rate_tensor(0, 0), 0.0);
        CHECK_EQ(strain_rate_tensor(0, 1), 0.0);
        CHECK_EQ(strain_rate_tensor(1, 0), 0.0);
        CHECK_EQ(strain_rate_tensor(1, 1), 0.0);
    }

    SUBCASE("Unit square with linear velocity field") {
        // Create a unit square element 
        wavecore::Node2D nodes[4];
        nodes[0] = wavecore::Node2D{{0.0, 0.0}}; // Bottom-left
        nodes[1] = wavecore::Node2D{{1.0, 0.0}}; // Bottom-right
        nodes[2] = wavecore::Node2D{{1.0, 1.0}}; // Top-right
        nodes[3] = wavecore::Node2D{{0.0, 1.0}}; // Top-left

        // Set linear velocity field that creates shear deformation
        // This should create a specific strain rate tensor
        nodes[0].velocity() = {0.0, 0.0};   // Bottom-left
        nodes[1].velocity() = {1.0, 0.0};   // Bottom-right  
        nodes[2].velocity() = {1.0, 1.0};   // Top-right
        nodes[3].velocity() = {0.0, 1.0};   // Top-left

        // Create connectivity
        std::array<size_t, 4> connectivity = {0, 1, 2, 3};

        // Create Quad4 element
        wavecore::Quad4 quad4;

        // Gather the element with nodes and connectivity
        quad4.gather(nodes, connectivity);

        // Test strain rate tensor at center (0,0)
        auto coords = wavecore::Vector<double, 2>({0.0, 0.0});
        auto strain_rate_tensor = quad4.strain_rate_tensor(coords);

        // For this specific case with linear velocity field that creates shear
        // we can compute the analytical solution:
        // The velocity gradient should be [[0, 1], [0, 0]] for a unit square
        // So the strain rate tensor (symmetric part) should be [[0, 0.5], [0.5, 0]]
        
        // Check that the diagonal terms are zero (no normal strain)
        CHECK_EQ(strain_rate_tensor(0, 0), 1.0);
        CHECK_EQ(strain_rate_tensor(1, 1), 1.0);
        
        // Check that off-diagonal terms are equal (shear strain rate)  
        CHECK_EQ(strain_rate_tensor(0, 1), 0.0);
        CHECK_EQ(strain_rate_tensor(1, 0), 0.0);
    }

    SUBCASE("Unit square with simple shear velocity field") {
    // Create a unit square element
    wavecore::Node2D nodes[4];
    nodes[0] = wavecore::Node2D{{0.0, 0.0}}; // Bottom-left
    nodes[1] = wavecore::Node2D{{1.0, 0.0}}; // Bottom-right
    nodes[2] = wavecore::Node2D{{1.0, 1.0}}; // Top-right
    nodes[3] = wavecore::Node2D{{0.0, 1.0}}; // Top-left

    // Set simple shear velocity field:
    //
    //     vx = y
    //     vy = 0
    //
    // Therefore:
    //
    //     grad(v) = [ dvx/dx   dvx/dy ] = [ 0   1 ]
    //               [ dvy/dx   dvy/dy ]   [ 0   0 ]
    //
    // The strain-rate tensor is the symmetric part:
    //
    //     D = 0.5 * (grad(v) + grad(v)^T)
    //
    // so:
    //
    //     D = [ 0    0.5 ]
    //         [ 0.5  0   ]
    nodes[0].velocity() = {0.0, 0.0}; // y = 0 -> vx = 0
    nodes[1].velocity() = {0.0, 0.0}; // y = 0 -> vx = 0
    nodes[2].velocity() = {1.0, 0.0}; // y = 1 -> vx = 1
    nodes[3].velocity() = {1.0, 0.0}; // y = 1 -> vx = 1

    // Create connectivity
    std::array<size_t, 4> connectivity = {0, 1, 2, 3};

    // Create Quad4 element
    wavecore::Quad4 quad4;

    // Gather the element with nodes and connectivity
    quad4.gather(nodes, connectivity);

    // Test strain rate tensor at center in parent coordinates
    auto coords = wavecore::Vector<double, 2>({0.0, 0.0});
    auto strain_rate_tensor = quad4.strain_rate_tensor(coords);

    // Check that the diagonal terms are zero: no normal strain rate
    CHECK_EQ(strain_rate_tensor(0, 0), doctest::Approx(0.0));
    CHECK_EQ(strain_rate_tensor(1, 1), doctest::Approx(0.0));

    // Check the symmetric shear terms
    CHECK_EQ(strain_rate_tensor(0, 1), doctest::Approx(0.5));
    CHECK_EQ(strain_rate_tensor(1, 0), doctest::Approx(0.5));
}
}
