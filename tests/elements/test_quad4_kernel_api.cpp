#include <array>

#include <doctest/doctest.h>

#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 kernel API uses caller-owned geometry and velocity storage") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{1.0, 0.0}},
        wavecore::Node2D{{1.0, 1.0}}, wavecore::Node2D{{0.0, 1.0}}};
    nodes[0].velocity() = {0.0, 0.0};
    nodes[1].velocity() = {1.0, 0.0};
    nodes[2].velocity() = {1.0, 1.0};
    nodes[3].velocity() = {0.0, 1.0};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

    wavecore::Quad4 element;
    wavecore::Quad4::geometry_state_type geometry{};
    wavecore::Quad4::nodal_velocity_type velocities{};
    element.refresh_geometry(nodes, connectivity, geometry);
    element.gather_velocities(nodes, connectivity, velocities);

    CHECK(geometry[0].jacobian_determinant == doctest::Approx(0.25));
    CHECK(geometry[0].gradients(0, 0) == doctest::Approx(-0.5));
    CHECK(geometry[0].gradients(1, 0) == doctest::Approx(-0.5));

    const auto strain_rate = element.strain_rate_tensor(geometry, velocities);
    CHECK(strain_rate(0, 0) == doctest::Approx(1.0));
    CHECK(strain_rate(1, 1) == doctest::Approx(1.0));
    CHECK(strain_rate(0, 1) == doctest::Approx(0.0));

    const auto second_geometry = geometry;
    const auto second_strain_rate =
        element.strain_rate_tensor(second_geometry, velocities);
    CHECK(second_strain_rate(0, 0) == doctest::Approx(strain_rate(0, 0)));

    const std::array<wavecore::Matrix<double, 2, 2>, 1> stresses{
        wavecore::Matrix<double, 2, 2>{{8.0, 0.0}, {0.0, 4.0}}};
    const auto forces = element.internal_force(
        stresses, wavecore::PlaneElementProperties{1.0}, geometry);
    CHECK(forces[0](0) == doctest::Approx(-4.0));
    CHECK(forces[0](1) == doctest::Approx(-2.0));
}
