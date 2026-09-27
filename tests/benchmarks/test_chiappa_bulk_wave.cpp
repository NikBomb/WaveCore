#include <cmath>
#include <cstddef>
#include <array>
#include <vector>

#include <doctest/doctest.h>

#include "wavecore/benchmarks/ChiappaBulkWave.hpp"
#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/archetypes/NodeArchetype.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"
#include "wavecore/relations/ElementRelations.hpp"
#include "wavecore/systems/FiniteElementSystems.hpp"

TEST_CASE("Chiappa bulk-wave reference uses the paper parameters") {
    const wavecore::ChiappaBulkWave reference;

    CHECK(reference.longitudinal_speed() == doctest::Approx(6339.7).epsilon(1e-4));
    CHECK(reference.transverse_speed() == doctest::Approx(3169.9).epsilon(1e-4));
    CHECK(reference.initial_velocity(0.5, 0.5)(0) == doctest::Approx(1.0));
    CHECK(reference.initial_velocity(0.5, 0.5)(1) == doctest::Approx(0.0));
    CHECK(reference.initial_velocity(0.1, 0.1)(0) == doctest::Approx(0.0));
}

TEST_CASE("Chiappa bulk-wave displacement satisfies constrained-slip boundaries") {
    const wavecore::ChiappaBulkWave reference;
    const double time = 7.1e-5;

    CHECK(reference.displacement(0.0, 0.37, time)(0) == doctest::Approx(0.0).epsilon(1e-10));
    CHECK(reference.displacement(1.0, 0.37, time)(0) == doctest::Approx(0.0).epsilon(1e-10));
    CHECK(reference.displacement(0.37, 0.0, time)(1) == doctest::Approx(0.0).epsilon(1e-10));
    CHECK(reference.displacement(0.37, 1.0, time)(1) == doctest::Approx(0.0).epsilon(1e-10));
}

TEST_CASE("Chiappa bulk-wave displacement is zero initially") {
    const wavecore::ChiappaBulkWave reference;
    const auto displacement = reference.displacement(0.25, 0.25, 0.0);
    CHECK(displacement(0) == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(displacement(1) == doctest::Approx(0.0).epsilon(1e-12));
}

TEST_CASE("Chiappa Fourier series reproduces the prescribed initial velocity") {
    const wavecore::ChiappaBulkWave reference;
    const auto inside = reference.analytical_velocity(0.5, 0.5, 0.0);
    const auto outside = reference.analytical_velocity(0.25, 0.25, 0.0);
    CHECK(inside(0) == doctest::Approx(1.0).epsilon(0.12));
    CHECK(inside(1) == doctest::Approx(0.0).epsilon(1e-10));
    CHECK(std::abs(outside(0)) < 0.02);
    CHECK(outside(1) == doctest::Approx(0.0).epsilon(1e-10));
}

TEST_CASE("One-point Quad4 leapfrog runs the Chiappa bulk benchmark") {
    using Element = wavecore::Quad4;
    using Material = wavecore::LinearElasticPlaneStrain;
    constexpr std::size_t subdivisions = 20;
    constexpr double length = 1.0;
    constexpr double spacing = length / static_cast<double>(subdivisions);

    wavecore::ChiappaBulkWave reference;
    wavecore::NodeArchetype<2> nodes;
    for (std::size_t j = 0; j <= subdivisions; ++j)
        for (std::size_t i = 0; i <= subdivisions; ++i) {
            const std::array<double, 2> coordinate{
                static_cast<double>(i) * spacing,
                static_cast<double>(j) * spacing};
            const auto index = nodes.add_node(wavecore::Node2D{coordinate});
            const auto initial_velocity = reference.initial_velocity(
                coordinate[0], coordinate[1]);
            nodes.node(index).velocity() = {
                initial_velocity(0), initial_velocity(1)};
        }

    wavecore::ElementArchetype<Element> elements;
    wavecore::MaterialArchetype<Material> materials;
    wavecore::GaussPointArchetype<Element, Material> points;
    wavecore::ElementNodeRelation<Element> element_nodes;
    wavecore::ElementGaussPointRelation<Element> element_points;
    wavecore::GaussPointMaterialRelation point_materials;
    const auto material = materials.add_material(Material{209.0e9, 1.0 / 3.0, 7800.0});

    for (std::size_t j = 0; j < subdivisions; ++j)
        for (std::size_t i = 0; i < subdivisions; ++i) {
            static_cast<void>(elements.add_element(wavecore::PlaneElementProperties{1.0}));
            const std::size_t n0 = j * (subdivisions + 1) + i;
            const std::size_t n1 = n0 + 1;
            const std::size_t n3 = (j + 1) * (subdivisions + 1) + i;
            const std::size_t n2 = n3 + 1;
            static_cast<void>(element_nodes.add({n0, n1, n2, n3}));
            const auto point = points.add_point(materials.material(material));
            static_cast<void>(element_points.add({point}));
            static_cast<void>(point_materials.add(material));
        }

    wavecore::refresh_element_geometry(elements, nodes.values(), element_nodes);
    wavecore::refresh_gauss_point_geometry(
        elements, nodes.values(), element_nodes, element_points, points);
    wavecore::assemble_lumped_mass(
        elements, nodes.values(), element_nodes, element_points,
        materials, point_materials);
    wavecore::refresh_material_force_state(
        elements, nodes.values(), element_nodes, element_points,
        materials, point_materials, points, 0.0);
    wavecore::compute_accelerations(nodes.values());

    const auto constraints = wavecore::chiappa_bulk_constraints(nodes.values());
    constexpr double dt = 1.0e-8;
    constexpr std::size_t steps = 7100;
    for (std::size_t step = 0; step < steps; ++step)
        wavecore::explicit_leapfrog_step(
            elements, nodes.values(), element_nodes, element_points,
            materials, point_materials, points, dt, constraints);

    const auto& observed = nodes.node(5 * (subdivisions + 1) + 5);
    const auto expected = reference.displacement(0.25, 0.25, steps * dt);
    CHECK(std::isfinite(observed.displacement()[0]));
    CHECK(std::isfinite(observed.displacement()[1]));
    CHECK(std::abs(observed.displacement()[0] - expected(0)) < 5.0e-6);
    CHECK(std::abs(observed.displacement()[1] - expected(1)) < 5.0e-6);

    const auto energy = wavecore::energy_snapshot(
        elements, nodes.values(), element_nodes, element_points,
        materials, point_materials, points);
    CHECK(std::isfinite(energy.kinetic));
    CHECK(std::isfinite(energy.strain));
    CHECK(energy.kinetic >= 0.0);
    CHECK(energy.strain >= 0.0);
}
