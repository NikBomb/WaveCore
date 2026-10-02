#ifndef WAVECORE_GAUSS_POINT_ARCHETYPE_HPP
#define WAVECORE_GAUSS_POINT_ARCHETYPE_HPP

#include <cstddef>
#include <utility>
#include "wavecore/fields/SimulationRecords.hpp"

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
        geometry_.add();
        states_.add(material.initial_state());
        strain_rates_.push_back({});
        return index;
    }

    [[nodiscard]] auto geometry(std::size_t index) { return geometry_.at(index); }
    [[nodiscard]] auto geometry(std::size_t index) const { return geometry_.at(index); }
    [[nodiscard]] auto state(std::size_t index) { return states_.at(index); }
    [[nodiscard]] auto state(std::size_t index) const { return states_.at(index); }
    [[nodiscard]] auto strain_rate(std::size_t index) {
        if (index >= size()) throw std::out_of_range("Strain-rate row exceeds storage");
        return SymmetricMatrixRow<typename DimensionEntries<Element::dimension>::StrainRate,double,Element::dimension>{strain_rates_.view(),index};
    }
    [[nodiscard]] auto strain_rate(std::size_t index) const {
        if (index >= size()) throw std::out_of_range("Strain-rate row exceeds storage");
        return SymmetricMatrixRow<typename DimensionEntries<Element::dimension>::StrainRate,const double,Element::dimension>{strain_rates_.view(),index};
    }

    [[nodiscard]] auto stress_view() noexcept { return states_.stress.view(); }
    [[nodiscard]] auto stress_view() const noexcept { return states_.stress.view(); }
    [[nodiscard]] auto strain_rate_view() noexcept { return strain_rates_.view(); }
    [[nodiscard]] auto strain_rate_view() const noexcept { return strain_rates_.view(); }
    [[nodiscard]] auto geometry_view() noexcept { return geometry_.view(); }
    [[nodiscard]] auto geometry_view() const noexcept { return geometry_.view(); }
    [[nodiscard]] auto state_view() noexcept { return states_.view(); }
    [[nodiscard]] auto state_view() const noexcept { return states_.view(); }
private:
    PointGeometryFields<Element> geometry_;
    MaterialStateFields<Material> states_;
    FieldStorage<typename DimensionEntries<Element::dimension>::StrainRate> strain_rates_;
};

} // namespace wavecore

#endif
