#include <array>
#include <limits>
#include <doctest/doctest.h>
#include "wavecore/elements/Quad4.hpp"

using Stress = wavecore::Matrix<double, 2, 2>;

TEST_CASE("Quad4 internal force uses supplied geometry") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{1, 0}},
        wavecore::Node2D{{1, 1}}, wavecore::Node2D{{0, 1}}};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4 element;
    wavecore::Quad4::geometry_state_type geometry{};
    element.refresh_geometry(nodes, connectivity, geometry);
    const std::array<Stress, 1> stresses{Stress{{8, 0}, {0, 4}}};
    const auto forces = element.internal_force(
        stresses, wavecore::PlaneElementProperties{1}, geometry);
    CHECK(forces[0](0) == doctest::Approx(-4));
    CHECK(forces[0](1) == doctest::Approx(-2));
    CHECK(forces[2](0) == doctest::Approx(4));
    CHECK(forces[2](1) == doctest::Approx(2));
}

TEST_CASE("Quad4 internal force scales with thickness and rejects invalid geometry") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{1, 0}},
        wavecore::Node2D{{1, 1}}, wavecore::Node2D{{0, 1}}};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4 element;
    wavecore::Quad4::geometry_state_type geometry{};
    element.refresh_geometry(nodes, connectivity, geometry);
    const std::array<Stress, 1> stresses{Stress{{8, 2}, {2, 4}}};
    const auto full = element.internal_force(
        stresses, wavecore::PlaneElementProperties{1}, geometry);
    const auto thin = element.internal_force(
        stresses, wavecore::PlaneElementProperties{.25}, geometry);
    CHECK(thin[1](0) == doctest::Approx(.25 * full[1](0)));
    geometry[0].jacobian_determinant = std::numeric_limits<double>::quiet_NaN();
    CHECK_THROWS_AS(static_cast<void>(element.internal_force(
                        stresses, wavecore::PlaneElementProperties{1}, geometry)),
                    std::domain_error);
}
