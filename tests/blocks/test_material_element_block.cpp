#include <doctest/doctest.h>

#include "wavecore/blocks/MaterialElementBlock.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"

TEST_CASE("Material element block owns typed element data and material history") {
    using Block = wavecore::MaterialElementBlock<
        wavecore::Quad4, wavecore::LinearElasticPlaneStrain>;

    Block block{wavecore::LinearElasticPlaneStrain{100.0, 0.25, 1.0}};
    const auto first = block.add_element(
        {0, 1, 2, 3}, wavecore::PlaneElementProperties{0.5});
    const auto second = block.add_element(
        {3, 4, 5, 6}, wavecore::PlaneElementProperties{1.0});

    CHECK(first == 0);
    CHECK(second == 1);
    CHECK(block.size() == 2);
    CHECK(block.connectivity(first)[0] == 0);
    CHECK(block.connectivity(second)[0] == 3);
    CHECK(block.properties(first).thickness() == 0.5);
    CHECK(block.properties(second).thickness() == 1.0);
    CHECK(block.states(first).size() == wavecore::Quad4::gauss_points);
    CHECK(block.states(first)[0].stress(0, 0) == 0.0);

    block.states(first)[0].stress(0, 0) = 12.0;
    CHECK(block.states(first)[0].stress(0, 0) == 12.0);
    CHECK(block.states(second)[0].stress(0, 0) == 0.0);
}

TEST_CASE("Material element block owns geometry and velocity columns") {
    using Block = wavecore::MaterialElementBlock<
        wavecore::Quad4, wavecore::LinearElasticPlaneStrain>;
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{1, 0}},
        wavecore::Node2D{{1, 1}}, wavecore::Node2D{{0, 1}}};
    Block block{wavecore::LinearElasticPlaneStrain{100, .25, 1}};
    const auto index = block.add_element(
        {0, 1, 2, 3}, wavecore::PlaneElementProperties{1});
    CHECK(index == 0);
    block.refresh_geometry(nodes);
    block.gather_velocities(nodes);
    CHECK(block.geometry(0)[0].jacobian_determinant == doctest::Approx(.25));
    CHECK(block.velocities(0)(0, 0) == 0.0);
}
