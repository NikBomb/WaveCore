#include <array>
#include <doctest/doctest.h>

#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/archetypes/NodeArchetype.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"
#include "wavecore/relations/ElementRelations.hpp"
#include "wavecore/systems/FiniteElementSystems.hpp"
#include "wavecore/systems/ExplicitDynamicsSystem.hpp"

TEST_CASE("Dynamics system owns initialization and integration through final time") {
    using E = wavecore::Quad4;
    using M = wavecore::LinearElasticPlaneStrain;
    wavecore::NodeArchetype<2> nodes;
    for (auto coordinate : std::array<std::array<double, 2>, 4>{{
             {0, 0}, {1, 0}, {1, 1}, {0, 1}}})
        static_cast<void>(nodes.add_node(wavecore::Node2D{coordinate}));
    wavecore::ElementArchetype<E> elements;
    wavecore::MaterialArchetype<M> materials;
    wavecore::GaussPointArchetype<E, M> points;
    wavecore::ElementNodeRelation<E> en;
    wavecore::ElementGaussPointRelation<E> eg;
    wavecore::GaussPointMaterialRelation gm;
    static_cast<void>(elements.add_element(wavecore::PlaneElementProperties{1.0}));
    const auto material = materials.add_material(M{100.0, .25, 1.0});
    const auto point = points.add_point(materials.material(material));
    static_cast<void>(en.add({0, 1, 2, 3}));
    static_cast<void>(eg.add({point}));
    static_cast<void>(gm.add(material));
    const wavecore::ExplicitDynamicsQuery<E, M> query{
        elements, nodes.values(), en, eg, materials, gm, points};
    std::array<std::array<bool, 2>, 4> constraints{};
    double final_time = .025;
    double maximum_dt = .01;
    bool fixed = false;
    wavecore::DynamicsOutputSchedule schedule{.01, {}};
    SUBCASE("rigid translation with shortened final step") {}
    SUBCASE("stationary constraints override incompatible IC") {
        fixed = true;
        constraints.fill({true, true});
    }
    SUBCASE("zero duration initializes and observes without advancing") { final_time = 0; }
    SUBCASE("automatic timestep uses current geometry") {
        maximum_dt = std::numeric_limits<double>::infinity();
    }
    SUBCASE("off-grid snapshots are sorted and duplicate times emit once") {
        schedule.snapshot_times = {.017, .007, .017};
    }
    int initialized = 0;
    const auto ic = [&](std::span<wavecore::Node2D> values) {
        ++initialized;
        for (auto& node : values) node.velocity() = {1.0, 0.0};
    };
    std::vector<double> times;
    std::vector<double> snapshots;
    const auto observer = [&](wavecore::DynamicsOutputEvent event,
                              std::span<const wavecore::Node2D> values) {
        const double time = event.time;
        CHECK((event.initial || event.final || event.history || event.snapshot));
        times.push_back(time);
        if (event.snapshot) snapshots.push_back(time);
        for (const auto& node : values) {
            CHECK(node.mass() == doctest::Approx(.25));
            CHECK(node.displacement()[0] == doctest::Approx(fixed ? 0.0 : time));
            CHECK(node.velocity()[0] == doctest::Approx(fixed ? 0.0 : 1.0));
        }
    };
    SUBCASE("invalid configuration is rejected before IC") {
        CHECK_THROWS_AS(wavecore::ExplicitDynamicsSystem::run(
            query, constraints, ic, -1.0, observer), std::invalid_argument);
        CHECK(initialized == 0);
        return;
    }
    wavecore::ExplicitDynamicsSystem::run(
        query, constraints, ic, final_time, observer, {maximum_dt, .9}, schedule);
    CHECK(initialized == 1);
    REQUIRE_FALSE(times.empty());
    CHECK(times.front() == 0.0);
    CHECK(times.back() == final_time);
    if (!schedule.snapshot_times.empty()) {
        CHECK(times == std::vector<double>{0.0, .007, .01, .017, .02, .025});
        CHECK(snapshots == std::vector<double>{.007, .017});
    } else if (final_time > 0) {
        CHECK(times == std::vector<double>{0.0, .01, .02, .025});
    }
    CHECK(points.state(point).stress(0, 0) == doctest::Approx(0.0));
}

TEST_CASE("Finite-element systems join archetypes and update state and forces") {
    using Element = wavecore::Quad4;
    using Material = wavecore::LinearElasticPlaneStrain;

    wavecore::ElementArchetype<Element> elements;
    wavecore::MaterialArchetype<Material> materials;
    wavecore::GaussPointArchetype<Element, Material> points;
    wavecore::ElementNodeRelation<Element> element_nodes;
    wavecore::ElementGaussPointRelation<Element> element_points;
    wavecore::GaussPointMaterialRelation point_materials;

    const auto element = elements.add_element(wavecore::PlaneElementProperties{1.0});
    const auto material = materials.add_material(Material{100.0, .25, 1.0});
    const auto point = points.add_point(materials.material(material));
    const auto node_relation = element_nodes.add({0, 1, 2, 3});
    const auto point_relation = element_points.add({point});
    const auto material_relation = point_materials.add(material);

    wavecore::NodeArchetype<2> nodes;
    const std::array<std::array<double, 2>, 4> coordinates{{
        {0, 0}, {1, 0}, {1, 1}, {0, 1}}};
    for (const auto& coordinate : coordinates)
        static_cast<void>(nodes.add_node(wavecore::Node2D{coordinate}));
    for (std::size_t index = 0; index < nodes.size(); ++index)
        nodes.node(index).velocity() = {
            nodes.node(index).coordinates()[0], nodes.node(index).coordinates()[1]};

    wavecore::refresh_element_geometry(elements, nodes.values(), element_nodes);
    wavecore::refresh_gauss_point_geometry(
        elements, nodes.values(), element_nodes, element_points, points);
    wavecore::compute_strain_rates(
        elements, nodes.values(), element_nodes, element_points, points);
    wavecore::update_material(materials, point_materials, points, 1.0);

    CHECK(element == 0);
    CHECK(node_relation == 0);
    CHECK(elements.geometry(element).measure == doctest::Approx(1.0));
    CHECK(elements.geometry(element).characteristic_length == doctest::Approx(1.0));
    CHECK(points.geometry(point).jacobian_determinant == doctest::Approx(.25));
    CHECK(points.strain_rate(point)(0, 0) == doctest::Approx(1.0));
    CHECK(points.strain_rate(point)(1, 1) == doctest::Approx(1.0));
    CHECK(points.state(point).stress(0, 0) == doctest::Approx(160.0));
    CHECK(point_materials.material(material_relation) == material);
    CHECK(element_points.points(point_relation)[0] == point);

    wavecore::assemble_internal_forces(
        elements, nodes.values(), element_nodes, element_points,
        materials, point_materials, points);
    wavecore::Vector<double, 2> total_force{};
    for (const auto& node : nodes.values()) {
        total_force(0) += node.internal_force()[0];
        total_force(1) += node.internal_force()[1];
    }
    CHECK(total_force(0) == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(total_force(1) == doctest::Approx(0.0).epsilon(1e-12));
}

TEST_CASE("Explicit systems assemble lumped mass and preserve a zero state") {
    using Element = wavecore::Quad4;
    using Material = wavecore::LinearElasticPlaneStrain;

    wavecore::ElementArchetype<Element> elements;
    wavecore::MaterialArchetype<Material> materials;
    wavecore::GaussPointArchetype<Element, Material> points;
    wavecore::ElementNodeRelation<Element> element_nodes;
    wavecore::ElementGaussPointRelation<Element> element_points;
    wavecore::GaussPointMaterialRelation point_materials;
    wavecore::NodeArchetype<2> nodes;

    for (const auto& coordinate : std::array<std::array<double, 2>, 4>{{
             {0, 0}, {1, 0}, {1, 1}, {0, 1}}})
        static_cast<void>(nodes.add_node(wavecore::Node2D{coordinate}));

    static_cast<void>(elements.add_element(wavecore::PlaneElementProperties{1.0}));
    const auto material = materials.add_material(Material{209.0e9, 1.0 / 3.0, 7800.0});
    const auto point = points.add_point(materials.material(material));
    static_cast<void>(element_nodes.add({0, 1, 2, 3}));
    static_cast<void>(element_points.add({point}));
    static_cast<void>(point_materials.add(material));

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

    CHECK(nodes.node(0).mass() == doctest::Approx(1950.0));
    CHECK(wavecore::critical_timestep(
              elements, element_points, point_materials, materials, 0.9) ==
          doctest::Approx(0.9 / 6339.7).epsilon(1e-4));
    CHECK(nodes.node(0).acceleration() == wavecore::Node2D::Vector{});

    const auto constraints = wavecore::chiappa_bulk_constraints(nodes.values());
    wavecore::explicit_leapfrog_step(
        elements, nodes.values(), element_nodes, element_points,
        materials, point_materials, points, 1.0e-8, constraints);
    for (const auto& node : nodes.values()) {
        CHECK(node.displacement() == wavecore::Node2D::Vector{});
        CHECK(node.velocity() == wavecore::Node2D::Vector{});
    }
}
