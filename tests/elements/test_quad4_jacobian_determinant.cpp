#include <array>
#include <doctest/doctest.h>
#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 stores the Gauss-point Jacobian determinant") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{2, 0}},
        wavecore::Node2D{{2, 1}}, wavecore::Node2D{{0, 1}}};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4::geometry_state_type geometry{};
    wavecore::Quad4{}.refresh_geometry(nodes, connectivity, geometry);
    CHECK(geometry[0].jacobian_determinant == doctest::Approx(.5));
}
