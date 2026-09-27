#ifndef WAVECORE_FINITE_ELEMENT_SYSTEMS_HPP
#define WAVECORE_FINITE_ELEMENT_SYSTEMS_HPP

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>

#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/mesh/ScatterForce.hpp"
#include "wavecore/relations/ElementRelations.hpp"

namespace wavecore {

template <IElementConcept Element>
void refresh_element_geometry(
    ElementArchetype<Element>& elements,
    std::span<const typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes) {
    if (elements.size() != element_nodes.size())
        throw std::invalid_argument("Element and node relation sizes must match");
    for (std::size_t element = 0; element < elements.size(); ++element) {
        const auto connectivity = element_nodes.nodes(element);
        elements.geometry(element).measure =
            elements.element().measure(nodes, connectivity);
        elements.geometry(element).characteristic_length =
            elements.element().characteristic_length(nodes, connectivity);
        elements.geometry(element).valid = true;
    }
}

template <IElementConcept Element, IMaterialConcept Material>
void refresh_gauss_point_geometry(
    const ElementArchetype<Element>& elements,
    std::span<const typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    GaussPointArchetype<Element, Material>& points) {
    if (elements.size() != element_nodes.size() || elements.size() != element_points.size())
        throw std::invalid_argument("Element relation sizes must match");
    for (std::size_t element = 0; element < elements.size(); ++element) {
        typename Element::geometry_state_type local_geometry{};
        elements.element().refresh_geometry(
            nodes, element_nodes.nodes(element), local_geometry);
        const auto point_ids = element_points.points(element);
        for (std::size_t gp = 0; gp < Element::gauss_points; ++gp)
            points.geometry(point_ids[gp]) = local_geometry[gp];
    }
}

template <IElementConcept Element, IMaterialConcept Material>
void compute_strain_rates(
    const ElementArchetype<Element>& elements,
    std::span<const typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    GaussPointArchetype<Element, Material>& points) {
    if (elements.size() != element_nodes.size() || elements.size() != element_points.size())
        throw std::invalid_argument("Element relation sizes must match");
    for (std::size_t element = 0; element < elements.size(); ++element) {
        typename Element::nodal_velocity_type velocities{};
        elements.element().gather_velocities(
            nodes, element_nodes.nodes(element), velocities);
        typename Element::geometry_state_type geometry{};
        const auto point_ids = element_points.points(element);
        for (std::size_t gp = 0; gp < Element::gauss_points; ++gp)
            geometry[gp] = points.geometry(point_ids[gp]);
        for (std::size_t gp = 0; gp < Element::gauss_points; ++gp)
            points.strain_rate(point_ids[gp]) =
                elements.element().strain_rate_tensor(geometry, velocities, gp);
    }
}

template <IElementConcept Element, IMaterialConcept Material>
void update_material(
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials,
    GaussPointArchetype<Element, Material>& points,
    double dt) {
    if (points.size() != point_materials.size())
        throw std::invalid_argument("Gauss-point relation size must match point storage");
    for (std::size_t point = 0; point < points.size(); ++point) {
        const auto material_id = point_materials.material(point);
        materials.material(material_id).update(
            points.state(point), points.strain_rate(point), dt);
    }
}

template <IElementConcept Element, IMaterialConcept Material>
void assemble_internal_forces(
    const ElementArchetype<Element>& elements,
    std::span<typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials,
    const GaussPointArchetype<Element, Material>& points) {
    if (elements.size() != element_nodes.size() || elements.size() != element_points.size())
        throw std::invalid_argument("Element relation sizes must match");
    for (std::size_t element = 0; element < elements.size(); ++element) {
        typename Element::geometry_state_type geometry{};
        std::array<ElementMatrix<Element>, Element::gauss_points> stresses{};
        const auto point_ids = element_points.points(element);
        for (std::size_t gp = 0; gp < Element::gauss_points; ++gp) {
            const auto point = point_ids[gp];
            geometry[gp] = points.geometry(point);
            const auto material_id = point_materials.material(point);
            stresses[gp] = materials.material(material_id).stress(points.state(point));
        }
        const auto local_forces = elements.element().internal_force(
            stresses, elements.properties(element), geometry);
        scatter_force(nodes, element_nodes.nodes(element), local_forces);
    }
}

} // namespace wavecore

#endif
