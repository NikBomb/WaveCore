#include "wavecore/elements/Quad4.hpp"
#include "wavecore/mesh/Node.hpp"
#include <doctest/doctest.h>

TEST_CASE("Quad4 Jacobian Determinant") {
  SUBCASE("Unit square Jacobian determinant") {
    // Create a simple square element with coordinates
    wavecore::Node2D nodes[4];
    nodes[0] = wavecore::Node2D{{0.0, 0.0}}; // Bottom-left
    nodes[1] = wavecore::Node2D{{1.0, 0.0}}; // Bottom-right
    nodes[2] = wavecore::Node2D{{1.0, 1.0}}; // Top-right
    nodes[3] = wavecore::Node2D{{0.0, 1.0}}; // Top-left

    // Create connectivity
    std::array<size_t, 4> connectivity = {0, 1, 2, 3};

    // Create Quad4 element
    wavecore::Quad4 quad4;

    // Gather the element with nodes and connectivity
    quad4.gather(nodes, connectivity);

    // Test jacobian determinant at center (0,0)
    auto coords = std::array<double, 2>{0.0, 0.0};
    double det = quad4.jacobian_determinant(coords);

    // For a unit square, the Jacobian determinant should be 1.0
    CHECK_EQ(det, 0.25);
  }

  SUBCASE("Transformed Quad") {
    // Create a transformed square element
    wavecore::Node2D nodes[4];
    nodes[0] = wavecore::Node2D{{0.0, 0.0}}; // Bottom-left
    nodes[1] = wavecore::Node2D{{2.0, 0.0}}; // Bottom-right (doubled in x)
    nodes[2] = wavecore::Node2D{{2.0, 1.0}}; // Top-right
    nodes[3] = wavecore::Node2D{{0.0, 1.0}}; // Top-left

    // Create connectivity
    std::array<size_t, 4> connectivity = {0, 1, 2, 3};

    // Create Quad4 element
    wavecore::Quad4 quad4;

    // Gather the element with nodes and connectivity
    quad4.gather(nodes, connectivity);

    // Test jacobian determinant at center (0,0)
    auto coords = std::array<double, 2>{0.0, 0.0};
    double det = quad4.jacobian_determinant(coords);

    // For a rectangle with width 2 and height 1, the Jacobian determinant
    // should be 2.0
    CHECK_EQ(det, 0.5);
  }
}
