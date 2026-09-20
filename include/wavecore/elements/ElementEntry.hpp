#ifndef WAVECORE_ELEMENT_ENTRY_HPP
#define WAVECORE_ELEMENT_ENTRY_HPP

#include <array>
#include <cstddef>
#include <utility>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/materials/IMaterialConcept.hpp"

namespace wavecore {

// Matching dimensions is necessary but does not establish formulation
// compatibility (for example, plane stress versus plane strain).
template <IElementConcept Element, IMaterialConcept Material>
    requires (Element::dimension == Material::dimension)
struct ElementEntry {
    using connectivity_type = std::array<std::size_t, Element::nodes_per_element>;
    using properties_type = typename Element::properties_type;
    using states_type = std::array<typename Material::state_type, Element::gauss_points>;

    // The material is used only for initialization; the future block owns it.
    // Connectivity indices must be validated against the mesh when binding it.
    ElementEntry(connectivity_type connectivity_value,
                 properties_type properties_value, const Material& material)
        : element{}, connectivity(std::move(connectivity_value)),
          properties(std::move(properties_value)),
          states{} {
        // States must be default-constructible and assignable. The material
        // supplies the actual initial history for every Gauss point.
        for (auto& state : states) {
            state = material.initial_state();
        }
    }

    Element element;
    connectivity_type connectivity;
    properties_type properties;
    states_type states;

};

} // namespace wavecore

#endif
