#include <array>
#include <doctest/doctest.h>

#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/archetypes/NodeArchetype.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"
#include "wavecore/relations/ElementRelations.hpp"

using Element = wavecore::Quad4;
using Material = wavecore::LinearElasticPlaneStrain;

TEST_CASE("Separate archetypes own components and relations own indexes") {
    wavecore::NodeArchetype<2> nodes;
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

    CHECK(elements.size() == 1);
    CHECK(element == 0);
    CHECK(materials.size() == 1);
    CHECK(points.size() == 1);
    CHECK(element_nodes.nodes(node_relation)[2] == 2);
    CHECK(element_points.points(point_relation)[0] == point);
    CHECK(point_materials.material(material_relation) == material);
    CHECK(points.state(point).stress(0, 0) == 0.0);
    CHECK(nodes.size() == 0);
}

TEST_CASE("Gauss-point archetype stores geometry and independent material state") {
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

    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{1, 0}},
        wavecore::Node2D{{1, 1}}, wavecore::Node2D{{0, 1}}};
    Element::geometry_state_type local_geometry{};
    elements.element().refresh_geometry(nodes, element_nodes.nodes(node_relation), local_geometry);
    points.geometry(point) = local_geometry[0];
    CHECK(points.geometry(point).jacobian_determinant == doctest::Approx(.25));
    points.state(point).stress(0, 0) = 7.0;
    CHECK(points.state(point).stress(0, 0) == 7.0);
    CHECK(element == 0);
    CHECK(element_points.points(point_relation)[0] == point);
    CHECK(point_materials.material(material_relation) == material);
}
