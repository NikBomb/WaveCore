#include <array>

#include <doctest/doctest.h>

#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 gather refreshes cached geometry and velocities") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{1.0, 0.0}},
        wavecore::Node2D{{1.0, 1.0}}, wavecore::Node2D{{0.0, 1.0}}
    };
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);
    CHECK(element.measure() == doctest::Approx(1.0));
    CHECK(element.strain_rate_tensor({0.0, 0.0})(0, 0) == 0.0);

    nodes[1] = wavecore::Node2D{{2.0, 0.0}};
    nodes[2] = wavecore::Node2D{{2.0, 3.0}};
    nodes[3] = wavecore::Node2D{{0.0, 3.0}};
    for (auto& node : nodes) {
        node.velocity() = {2.0 * node.coordinates()[0], -node.coordinates()[1]};
    }
    // Cached values remain a snapshot until the next gather.
    CHECK(element.measure() == doctest::Approx(1.0));
    CHECK(element.strain_rate_tensor({0.0, 0.0})(0, 0) == 0.0);

    SUBCASE("refresh from original storage") { element.gather(nodes, connectivity); }
    SUBCASE("refresh from different storage and connectivity") {
        std::array<wavecore::Node2D, 4> reordered{nodes[1], nodes[3], nodes[0], nodes[2]};
        const std::array<std::size_t, 4> remapped{2, 0, 3, 1};
        element.gather(reordered, remapped);
    }

    CHECK(element.measure() == doctest::Approx(6.0));
    const auto strain_rate = element.strain_rate_tensor({0.0, 0.0});
    CHECK(strain_rate(0, 0) == doctest::Approx(2.0));
    CHECK(strain_rate(1, 1) == doctest::Approx(-1.0));
    CHECK(strain_rate(0, 1) == doctest::Approx(0.0));
    CHECK(strain_rate(1, 0) == doctest::Approx(0.0));
}
