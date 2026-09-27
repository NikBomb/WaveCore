#ifndef WAVECORE_GAUSS_POINT_ARCHETYPE_HPP
#define WAVECORE_GAUSS_POINT_ARCHETYPE_HPP

#include <cstddef>
#include <utility>
#include <vector>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/materials/IMaterialConcept.hpp"

namespace wavecore {

template <IElementConcept Element, IMaterialConcept Material>
    requires (Element::dimension == Material::dimension)
class GaussPointArchetype {
public:
    using geometry_type = typename Element::geometry_point_type;
    using state_type = typename Material::state_type;
    using strain_rate_type = typename Element::strain_rate_type;

    [[nodiscard]] std::size_t size() const noexcept { return geometry_.size(); }

    [[nodiscard]] std::size_t add_point(const Material& material) {
        const auto index = geometry_.size();
        geometry_.emplace_back();
        states_.push_back(material.initial_state());
        strain_rates_.emplace_back();
        return index;
    }

    [[nodiscard]] geometry_type& geometry(std::size_t index) { return geometry_.at(index); }
    [[nodiscard]] const geometry_type& geometry(std::size_t index) const { return geometry_.at(index); }
    [[nodiscard]] state_type& state(std::size_t index) { return states_.at(index); }
    [[nodiscard]] const state_type& state(std::size_t index) const { return states_.at(index); }
    [[nodiscard]] strain_rate_type& strain_rate(std::size_t index) { return strain_rates_.at(index); }
    [[nodiscard]] const strain_rate_type& strain_rate(std::size_t index) const { return strain_rates_.at(index); }

private:
    std::vector<geometry_type> geometry_;
    std::vector<state_type> states_;
    std::vector<strain_rate_type> strain_rates_;
};

} // namespace wavecore

#endif
