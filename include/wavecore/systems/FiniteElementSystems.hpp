#ifndef WAVECORE_FINITE_ELEMENT_SYSTEMS_HPP
#define WAVECORE_FINITE_ELEMENT_SYSTEMS_HPP

#include <array>
#include <cstddef>
#include <cmath>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/mesh/ScatterForce.hpp"
#include "wavecore/relations/ElementRelations.hpp"

namespace wavecore {

struct EnergySnapshot {
    double kinetic = 0.0;
    double strain = 0.0;

    [[nodiscard]] double total() const noexcept { return kinetic + strain; }
};

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

template <std::size_t Dimension>
void clear_nodal_forces(std::span<Node<Dimension>> nodes) noexcept {
    for (auto& node : nodes) {
        node.internal_force().fill(0.0);
        node.external_force().fill(0.0);
    }
}

template <IElementConcept Element, IMaterialConcept Material>
void assemble_lumped_mass(
    ElementArchetype<Element>& elements,
    std::span<typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials) {
    if (elements.size() != element_nodes.size() || elements.size() != element_points.size())
        throw std::invalid_argument("Element relation sizes must match");
    for (auto& node : nodes)
        node.set_mass(0.0);

    for (std::size_t element = 0; element < elements.size(); ++element) {
        const auto point_ids = element_points.points(element);
        double density = 0.0;
        for (const auto point : point_ids) {
            const auto material_id = point_materials.material(point);
            density += materials.material(material_id).density();
        }
        density /= static_cast<double>(Element::gauss_points);
        const double nodal_mass = density * elements.geometry(element).measure *
                                  elements.properties(element).thickness() /
                                  static_cast<double>(Element::nodes_per_element);
        for (const auto node : element_nodes.nodes(element)) {
            if (node >= nodes.size())
                throw std::out_of_range("Mass connectivity exceeds node storage");
            nodes[node].set_mass(nodes[node].mass() + nodal_mass);
        }
    }
}

template <std::size_t Dimension>
void compute_accelerations(std::span<Node<Dimension>> nodes) {
    for (auto& node : nodes) {
        if (!std::isfinite(node.mass()) || node.mass() <= 0.0)
            throw std::domain_error("Explicit dynamics requires positive nodal mass");
        for (std::size_t d = 0; d < Dimension; ++d)
            node.acceleration()[d] =
                (node.external_force()[d] - node.internal_force()[d]) / node.mass();
    }
}

template <std::size_t Dimension>
void first_half_velocity_update(std::span<Node<Dimension>> nodes, double dt) {
    for (auto& node : nodes)
        for (std::size_t d = 0; d < Dimension; ++d)
            node.velocity()[d] += 0.5 * dt * node.acceleration()[d];
}

template <std::size_t Dimension>
void update_nodal_displacements(std::span<Node<Dimension>> nodes, double dt) {
    for (auto& node : nodes)
        for (std::size_t d = 0; d < Dimension; ++d)
            node.displacement()[d] += dt * node.velocity()[d];
}

template <std::size_t Dimension>
void second_half_velocity_update(std::span<Node<Dimension>> nodes, double dt) {
    for (auto& node : nodes)
        for (std::size_t d = 0; d < Dimension; ++d)
            node.velocity()[d] += 0.5 * dt * node.acceleration()[d];
}

template <std::size_t Dimension>
void apply_velocity_constraints(
    std::span<Node<Dimension>> nodes,
    std::span<const std::array<bool, Dimension>> constrained) {
    if (nodes.size() != constrained.size())
        throw std::invalid_argument("Constraint and node storage sizes must match");
    for (std::size_t i = 0; i < nodes.size(); ++i)
        for (std::size_t d = 0; d < Dimension; ++d)
            if (constrained[i][d]) {
                nodes[i].velocity()[d] = 0.0;
                nodes[i].displacement()[d] = 0.0;
            }
}

inline std::vector<std::array<bool, 2>> chiappa_bulk_constraints(
    std::span<const Node2D> nodes, double a = 1.0, double b = 1.0,
    double tolerance = 1.0e-12) {
    std::vector<std::array<bool, 2>> result(nodes.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto& x = nodes[i].coordinates();
        result[i] = {
            std::abs(x[0]) <= tolerance || std::abs(x[0] - a) <= tolerance,
            std::abs(x[1]) <= tolerance || std::abs(x[1] - b) <= tolerance};
    }
    return result;
}

// Box 6.1 getforce, expressed as separate ECS passes rather than one nested
// element/GP loop. Relations provide the gathers/scatters between archetypes.
// This is the current internal-force path, not the complete getforce routine:
// external-load evaluation is absent and critical dt is reduced by the driver.
template <IElementConcept Element, IMaterialConcept Material>
void refresh_material_force_state(
    ElementArchetype<Element>& elements,
    std::span<typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials,
    GaussPointArchetype<Element, Material>& points,
    double dt) {
    // getforce quadrature steps 2-3: current geometry, deformation rate, stress.
    // At initialization dt=0; the current material's rate update adds no history.
    refresh_element_geometry(
        elements, std::span<const typename Element::node_type>(nodes),
        element_nodes);
    refresh_gauss_point_geometry(
        elements, std::span<const typename Element::node_type>(nodes),
        element_nodes, element_points, points);
    compute_strain_rates(
        elements, std::span<const typename Element::node_type>(nodes),
        element_nodes, element_points, points);
    update_material(materials, point_materials, points, dt);
    // getforce initialization and quadrature step 4: integrate B^T sigma and
    // scatter internal forces. Both force arrays are cleared here; no prescribed
    // external loads are restored yet. Acceleration forms f_ext - f_int later.
    clear_nodal_forces(nodes);
    assemble_internal_forces(
        elements, nodes, element_nodes, element_points, materials,
        point_materials, points);
}

template <IElementConcept Element, IMaterialConcept Material>
[[nodiscard]] double critical_timestep(
    const ElementArchetype<Element>& elements,
    const ElementGaussPointRelation<Element>& element_points,
    const GaussPointMaterialRelation& point_materials,
    const MaterialArchetype<Material>& materials,
    double safety_factor = 0.9) {
    if (safety_factor <= 0.0 || safety_factor > 1.0)
        throw std::invalid_argument("Critical timestep safety factor must be in (0, 1]");
    double result = std::numeric_limits<double>::infinity();
    for (std::size_t element = 0; element < elements.size(); ++element) {
        const auto point = point_materials.material(element_points.points(element)[0]);
        const auto& material = materials.material(point);
        if constexpr (requires { material.longitudinal_wave_speed(); }) {
            result = std::min(result, safety_factor *
                elements.geometry(element).characteristic_length /
                material.longitudinal_wave_speed());
        }
    }
    if (!std::isfinite(result))
        throw std::invalid_argument("Material does not provide a wave speed");
    return result;
}

template <IElementConcept Element, IMaterialConcept Material>
void explicit_leapfrog_step(
    ElementArchetype<Element>& elements,
    std::span<typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials,
    GaussPointArchetype<Element, Material>& points,
    double dt,
    std::span<const std::array<bool, Element::dimension>> constrained) {
    if (!std::isfinite(dt) || dt <= 0.0)
        throw std::invalid_argument("Explicit timestep must be finite and positive");
    // Box 6.1, step 5: v^(n+1/2) = v^n + (dt/2) a^n.
    first_half_velocity_update(nodes, dt);
    // Step 6: enforce prescribed half-step velocities (currently zero only).
    apply_velocity_constraints(nodes, constrained);
    // Step 7: d^(n+1) = d^n + dt v^(n+1/2); current positions are X + d.
    update_nodal_displacements(nodes, dt);
    // Keep stationary constrained displacements exactly zero as well.
    apply_velocity_constraints(nodes, constrained);
    // Step 8: getforce at the new configuration; advance material history once.
    refresh_material_force_state(
        elements, nodes, element_nodes, element_points, materials,
        point_materials, points, dt);
    // Step 9: a^(n+1) = M^-1 (f_ext - f_int), with C_damp = 0.
    compute_accelerations(nodes);
    // Step 10: complete the velocity update at integer time t^(n+1).
    second_half_velocity_update(nodes, dt);
    // Project integer-time velocities onto the same stationary constraints.
    apply_velocity_constraints(nodes, constrained);
}

template <std::size_t Dimension>
[[nodiscard]] double nodal_kinetic_energy(
    std::span<const Node<Dimension>> nodes) noexcept {
    double result = 0.0;
    for (const auto& node : nodes) {
        double speed_squared = 0.0;
        for (const double component : node.velocity())
            speed_squared += component * component;
        result += 0.5 * node.mass() * speed_squared;
    }
    return result;
}

template <IElementConcept Element, IMaterialConcept Material>
[[nodiscard]] EnergySnapshot energy_snapshot(
    const ElementArchetype<Element>& elements,
    std::span<const typename Element::node_type> nodes,
    const ElementNodeRelation<Element>& element_nodes,
    const ElementGaussPointRelation<Element>& element_points,
    const MaterialArchetype<Material>& materials,
    const GaussPointMaterialRelation& point_materials,
    const GaussPointArchetype<Element, Material>& points) {
    if constexpr (!requires(typename Element::nodal_displacement_type displacements) {
                      Element{}.gather_displacements(
                          nodes, element_nodes.nodes(0), displacements);
                      Element{}.strain_tensor(
                          typename Element::geometry_state_type{}, displacements);
                  }) {
        throw std::invalid_argument("Element does not provide displacement energy kinematics");
    } else {
        EnergySnapshot result{.kinetic = nodal_kinetic_energy(nodes)};
        for (std::size_t element = 0; element < elements.size(); ++element) {
            typename Element::nodal_displacement_type displacements{};
            elements.element().gather_displacements(
                nodes, element_nodes.nodes(element), displacements);
            const auto point_ids = element_points.points(element);
            for (std::size_t gp = 0; gp < Element::gauss_points; ++gp) {
                const auto point = point_ids[gp];
                typename Element::geometry_state_type geometry{};
                geometry[gp] = points.geometry(point);
                const auto strain = elements.element().strain_tensor(
                    geometry, displacements, gp);
                const auto stress = materials.material(
                    point_materials.material(point)).stress(points.state(point));
                const auto quadrature = elements.element().quadrature()[gp];
                double density = 0.0;
                for (std::size_t i = 0; i < Element::dimension; ++i)
                    for (std::size_t j = 0; j < Element::dimension; ++j)
                        density += 0.5 * stress(i, j) * strain(i, j);
                result.strain += density * quadrature.weight *
                    points.geometry(point).jacobian_determinant *
                    elements.properties(element).thickness();
            }
        }
        return result;
    }
}

} // namespace wavecore

#endif
