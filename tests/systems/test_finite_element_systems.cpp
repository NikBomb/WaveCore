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
