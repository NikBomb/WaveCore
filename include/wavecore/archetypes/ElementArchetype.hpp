#ifndef WAVECORE_ELEMENT_ARCHETYPE_HPP
#define WAVECORE_ELEMENT_ARCHETYPE_HPP

#include <cstddef>
#include <utility>
#include "wavecore/fields/SimulationRecords.hpp"

#include "wavecore/elements/IElementConcept.hpp"

namespace wavecore {

template <IElementConcept Element>
class ElementArchetype {
public:
    using element_type = Element;
    using properties_type = typename Element::properties_type;
    using geometry_type = typename Element::element_geometry_type;

    [[nodiscard]] const Element& element() const noexcept { return element_; }
    [[nodiscard]] std::size_t size() const noexcept { return properties_.size(); }

    [[nodiscard]] std::size_t add_element(properties_type properties) {
        const auto index = properties_.size();
        properties_.push_back(std::move(properties));
        geometry_.add();
        return index;
    }

    [[nodiscard]] properties_type& properties(std::size_t index) {
        return properties_.at(index);
    }
    [[nodiscard]] const properties_type& properties(std::size_t index) const {
        return properties_.at(index);
    }
    [[nodiscard]] auto geometry(std::size_t index) { return geometry_.at(index); }
    [[nodiscard]] auto geometry(std::size_t index) const { return geometry_.at(index); }

private:
    Element element_;
    ScalarStorage<properties_type> properties_;
    ElementGeometryFields<Element> geometry_;
};

} // namespace wavecore

#endif
