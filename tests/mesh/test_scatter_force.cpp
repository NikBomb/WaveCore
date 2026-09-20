#include <array>
#include <span>
#include <stdexcept>

#include <doctest/doctest.h>

#include "wavecore/elements/Quad4.hpp"
#include "wavecore/mesh/ScatterForce.hpp"

TEST_CASE("Scatter maps local forces to selected nodes and preserves existing forces") {
    std::array<wavecore::Node3D, 6> nodes{};
    nodes[4].internal_force() = {10.0, 20.0, 30.0};
    const std::array<std::size_t, 2> connectivity{4, 1};
    const std::array<wavecore::Vector<double, 3>, 2> forces{
        wavecore::Vector<double, 3>{1.0, 2.0, 3.0},
        wavecore::Vector<double, 3>{-1.0, -2.0, -3.0}
    };
    wavecore::scatter_force(std::span{nodes}, connectivity, forces);
    CHECK(nodes[4].internal_force() == wavecore::Node3D::Vector{11.0, 22.0, 33.0});
    CHECK(nodes[1].internal_force() == wavecore::Node3D::Vector{-1.0, -2.0, -3.0});
    for (const std::size_t untouched : {0U, 2U, 3U, 5U}) {
        CHECK(nodes[untouched].internal_force() == wavecore::Node3D::Vector{});
    }
}

TEST_CASE("Two adjacent quads accumulate internal forces at shared nodes") {
    std::array<wavecore::Node2D, 6> nodes{
        wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{1.0, 0.0}},
        wavecore::Node2D{{2.0, 0.0}}, wavecore::Node2D{{0.0, 1.0}},
        wavecore::Node2D{{1.0, 1.0}}, wavecore::Node2D{{2.0, 1.0}}
    };
    const std::array<std::size_t, 4> left{0, 1, 4, 3};
    const std::array<std::size_t, 4> right{1, 2, 5, 4};
    const std::array<wavecore::Matrix<double, 2, 2>, 1> stresses{
        wavecore::Matrix<double, 2, 2>{{2.0, 0.0}, {0.0, 0.0}}
    };
    const wavecore::PlaneElementProperties properties{1.0};
    std::span<wavecore::Node2D> node_view{nodes};
    wavecore::Quad4 element;
    for (const auto& connectivity : {left, right}) {
        element.gather(node_view, connectivity);
        const auto forces = element.internal_force(stresses, properties);
        wavecore::scatter_force(node_view, connectivity, forces);
    }
    const std::array<double, 6> expected_x{-1.0, 0.0, 1.0, -1.0, 0.0, 1.0};
    for (std::size_t node = 0; node < nodes.size(); ++node) {
        CHECK(nodes[node].internal_force()[0] == doctest::Approx(expected_x[node]));
        CHECK(nodes[node].internal_force()[1] == 0.0);
    }
}

TEST_CASE("Invalid scatter connectivity leaves all nodal forces unchanged") {
    std::array<wavecore::Node2D, 2> nodes{};
    nodes[0].internal_force() = {3.0, 4.0};
    const std::array<std::size_t, 2> connectivity{0, 2};
    const std::array<wavecore::Vector<double, 2>, 2> forces{
        wavecore::Vector<double, 2>{1.0, 2.0}, wavecore::Vector<double, 2>{3.0, 4.0}
    };
    CHECK_THROWS_AS(wavecore::scatter_force(std::span{nodes}, connectivity, forces),
                    std::out_of_range);
    CHECK(nodes[0].internal_force() == wavecore::Node2D::Vector{3.0, 4.0});
    CHECK(nodes[1].internal_force() == wavecore::Node2D::Vector{});
}
