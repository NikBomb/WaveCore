#include <array>
#include <limits>
#include <stdexcept>

#include <doctest/doctest.h>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/elements/Quad4.hpp"

namespace {

using Stress = wavecore::ElementMatrix<wavecore::Quad4>;
using Forces = wavecore::ElementLocalVector<wavecore::Quad4>;
using Nodes = std::array<wavecore::Node2D, 4>;
const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

Nodes unit_square() {
    return {wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{1.0, 0.0}},
            wavecore::Node2D{{1.0, 1.0}}, wavecore::Node2D{{0.0, 1.0}}};
}

void check_forces(const Forces& actual, const Forces& expected) {
    for (std::size_t node = 0; node < actual.size(); ++node) {
        for (std::size_t d = 0; d < 2; ++d) {
            CHECK(actual[node](d) == doctest::Approx(expected[node](d)));
        }
    }
}

} // namespace

TEST_CASE("Quad4 internal forces match analytical normal and shear stresses") {
    auto nodes = unit_square();
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);
    std::array<Stress, 1> stresses{};
    Forces expected{};

    SUBCASE("normal stresses") {
        stresses[0] = Stress{{8.0, 0.0}, {0.0, 4.0}};
        expected = Forces{{{-4.0, -2.0}, {4.0, -2.0},
                           {4.0, 2.0}, {-4.0, 2.0}}};
    }
    SUBCASE("shear stress") {
        stresses[0] = Stress{{0.0, 6.0}, {6.0, 0.0}};
        expected = Forces{{{-3.0, -3.0}, {-3.0, 3.0},
                           {3.0, 3.0}, {3.0, -3.0}}};
    }
    SUBCASE("zero stress") {}

    check_forces(element.internal_force(stresses, wavecore::PlaneElementProperties{1.0}),
                 expected);
}

TEST_CASE("Quad4 internal forces scale with thickness") {
    auto nodes = unit_square();
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);
    const std::array<Stress, 1> stresses{Stress{{8.0, 2.0}, {2.0, 4.0}}};
    const auto full = element.internal_force(stresses, wavecore::PlaneElementProperties{1.0});
    const auto thin = element.internal_force(stresses, wavecore::PlaneElementProperties{0.25});
    for (std::size_t node = 0; node < full.size(); ++node) {
        for (std::size_t d = 0; d < 2; ++d) {
            CHECK(thin[node](d) == doctest::Approx(0.25 * full[node](d)));
        }
    }
}

TEST_CASE("Quad4 internal forces on skewed geometry balance force and moment") {
    Nodes nodes{wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{2.0, 0.0}},
                wavecore::Node2D{{3.0, 3.0}}, wavecore::Node2D{{1.0, 3.0}}};
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);
    const std::array<Stress, 1> stresses{Stress{{4.0, 2.0}, {2.0, 6.0}}};
    const auto forces = element.internal_force(stresses, wavecore::PlaneElementProperties{1.0});
    // Analytical constant boundary traction, distributed equally to edge nodes.
    check_forces(forces, Forces{{{-7.0, -6.0}, {3.0, -6.0},
                                {7.0, 6.0}, {-3.0, 6.0}}});

    double sum_x = 0.0;
    double sum_y = 0.0;
    double moment = 0.0;
    for (std::size_t node = 0; node < forces.size(); ++node) {
        sum_x += forces[node](0);
        sum_y += forces[node](1);
        moment += nodes[node].coordinates()[0] * forces[node](1)
                - nodes[node].coordinates()[1] * forces[node](0);
    }
    CHECK(sum_x == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(sum_y == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(moment == doctest::Approx(0.0).epsilon(1e-12));
}

TEST_CASE("Repeated force queries leave stresses and nodal forces unchanged") {
    auto nodes = unit_square();
    for (auto& node : nodes) {
        node.internal_force() = {11.0, 12.0};
    }
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);
    std::array<Stress, 1> stresses{Stress{{8.0, 2.0}, {2.0, 4.0}}};
    const auto original = stresses;
    const auto& const_element = element;
    const wavecore::PlaneElementProperties properties{1.0};
    const auto first = const_element.internal_force(stresses, properties);
    check_forces(const_element.internal_force(stresses, properties), first);
    CHECK(stresses == original);
    for (const auto& node : nodes) {
        CHECK(node.internal_force()[0] == 11.0);
        CHECK(node.internal_force()[1] == 12.0);
    }
}

TEST_CASE("Quad4 internal force rejects invalid integration-point Jacobians") {
    auto nodes = unit_square();
    auto indices = connectivity;
    bool gather = true;

    SUBCASE("clockwise connectivity") { indices = {0, 3, 2, 1}; }
    SUBCASE("collapsed geometry") { nodes = Nodes{}; }
    SUBCASE("NaN coordinate") {
        nodes[0] = wavecore::Node2D{{std::numeric_limits<double>::quiet_NaN(), 0.0}};
    }
    SUBCASE("infinite coordinate") {
        nodes[0] = wavecore::Node2D{{std::numeric_limits<double>::infinity(), 0.0}};
    }
    SUBCASE("geometry not gathered") { gather = false; }

    wavecore::Quad4 element;
    if (gather) {
        element.gather(nodes, indices);
    }
    const std::array<Stress, 1> stresses{Stress{{8.0, 2.0}, {2.0, 4.0}}};
    CHECK_THROWS_AS(
        static_cast<void>(element.internal_force(stresses, wavecore::PlaneElementProperties{1.0})),
        std::domain_error);
}
