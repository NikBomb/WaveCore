#ifndef WAVECORE_MATERIAL_ELEMENT_BLOCK_HPP
#define WAVECORE_MATERIAL_ELEMENT_BLOCK_HPP

#include <array>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/materials/IMaterialConcept.hpp"

namespace wavecore {

// A typed block is the owning storage unit for one element/material
// combination. The block owns all per-element and per-integration-point data;
// the element is one reusable formulation object for the whole block.
template <IElementConcept Element, IMaterialConcept Material>
    requires (Element::dimension == Material::dimension)
class MaterialElementBlock {
public:
    using element_type = Element;
    using material_type = Material;
    using connectivity_type = std::array<std::size_t, Element::nodes_per_element>;
    using properties_type = typename Element::properties_type;
    using state_type = typename Material::state_type;
    using states_type = std::array<state_type, Element::gauss_points>;
    using geometry_state_type = typename Element::geometry_state_type;
    using nodal_velocity_type = typename Element::nodal_velocity_type;
    using node_type = typename Element::node_type;

    explicit MaterialElementBlock(Material material)
        : material_(std::move(material)) {}

    [[nodiscard]] const Material& material() const noexcept { return material_; }
    [[nodiscard]] const Element& element() const noexcept { return element_; }

    [[nodiscard]] std::size_t size() const noexcept { return connectivity_.size(); }

    // Adds one logical element and initializes every integration-point state
    // through the material definition.
    [[nodiscard]] std::size_t add_element(connectivity_type connectivity,
                                           properties_type properties) {
        states_type states{};
        for (auto& state : states) {
            state = material_.initial_state();
        }

        const auto index = connectivity_.size();
        connectivity_.push_back(std::move(connectivity));
        properties_.push_back(std::move(properties));
        states_.push_back(std::move(states));
        geometry_.emplace_back();
        velocities_.emplace_back();
        return index;
    }

    [[nodiscard]] const connectivity_type& connectivity(std::size_t index) const {
        return connectivity_.at(index);
    }

    [[nodiscard]] const properties_type& properties(std::size_t index) const {
        return properties_.at(index);
    }

    [[nodiscard]] states_type& states(std::size_t index) { return states_.at(index); }

    [[nodiscard]] const states_type& states(std::size_t index) const {
        return states_.at(index);
    }

    [[nodiscard]] geometry_state_type& geometry(std::size_t index) {
        return geometry_.at(index);
    }

    [[nodiscard]] const geometry_state_type& geometry(std::size_t index) const {
        return geometry_.at(index);
    }

    [[nodiscard]] nodal_velocity_type& velocities(std::size_t index) {
        return velocities_.at(index);
    }

    [[nodiscard]] const nodal_velocity_type& velocities(std::size_t index) const {
        return velocities_.at(index);
    }

    void refresh_geometry(std::span<const node_type> nodes) {
        for (std::size_t index = 0; index < size(); ++index) {
            element_.refresh_geometry(nodes, connectivity(index), geometry(index));
        }
    }

    void gather_velocities(std::span<const node_type> nodes) {
        for (std::size_t index = 0; index < size(); ++index) {
            element_.gather_velocities(nodes, connectivity(index), velocities(index));
        }
    }

private:
    Element element_;
    Material material_;
    std::vector<connectivity_type> connectivity_;
    std::vector<properties_type> properties_;
    std::vector<states_type> states_;
    std::vector<geometry_state_type> geometry_;
    std::vector<nodal_velocity_type> velocities_;
};

} // namespace wavecore

#endif
