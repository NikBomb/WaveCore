#include <array>
#include <doctest/doctest.h>
#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 quadrature integrates constants and physical area") {
    const wavecore::Quad4 element;
    const auto points = element.quadrature();
    REQUIRE(points.size() == 1);
    CHECK(points[0].weight == 4.0);
    CHECK(2.0 * points[0].weight == doctest::Approx(8.0));
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{2, 0}},
        wavecore::Node2D{{3, 3}}, wavecore::Node2D{{1, 3}}};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4::geometry_state_type geometry{};
    element.refresh_geometry(nodes, connectivity, geometry);
    CHECK(points[0].weight * geometry[0].jacobian_determinant == doctest::Approx(6.0));
}
