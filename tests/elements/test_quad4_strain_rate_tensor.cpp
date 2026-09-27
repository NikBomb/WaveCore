#include <array>
#include <doctest/doctest.h>
#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 computes strain rate from supplied velocities") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{1, 0}},
        wavecore::Node2D{{1, 1}}, wavecore::Node2D{{0, 1}}};
    nodes[0].velocity() = {0, 0}; nodes[1].velocity() = {0, 0};
    nodes[2].velocity() = {1, 0}; nodes[3].velocity() = {1, 0};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4 element;
    wavecore::Quad4::geometry_state_type geometry{};
    wavecore::Quad4::nodal_velocity_type velocities{};
    element.refresh_geometry(nodes, connectivity, geometry);
    element.gather_velocities(nodes, connectivity, velocities);
    const auto strain = element.strain_rate_tensor(geometry, velocities);
    CHECK(strain(0, 0) == doctest::Approx(0));
    CHECK(strain(1, 1) == doctest::Approx(0));
    CHECK(strain(0, 1) == doctest::Approx(.5));
    CHECK(strain(1, 0) == doctest::Approx(.5));
}
