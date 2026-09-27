#ifndef WAVECORE_ELEMENT_RELATIONS_HPP
#define WAVECORE_ELEMENT_RELATIONS_HPP

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "wavecore/elements/IElementConcept.hpp"

namespace wavecore {

template <IElementConcept Element>
class ElementNodeRelation {
public:
    using connectivity_type = std::array<std::size_t, Element::nodes_per_element>;

    [[nodiscard]] std::size_t size() const noexcept { return connectivity_.size(); }
    [[nodiscard]] std::size_t add(connectivity_type connectivity) {
        const auto index = connectivity_.size();
        connectivity_.push_back(std::move(connectivity));
        return index;
    }
    [[nodiscard]] const connectivity_type& nodes(std::size_t element) const {
        return connectivity_.at(element);
    }

private:
    std::vector<connectivity_type> connectivity_;
};

template <IElementConcept Element>
class ElementGaussPointRelation {
public:
    using points_type = std::array<std::size_t, Element::gauss_points>;

    [[nodiscard]] std::size_t size() const noexcept { return points_.size(); }
    [[nodiscard]] std::size_t add(points_type points) {
        const auto index = points_.size();
        points_.push_back(std::move(points));
        return index;
    }
    [[nodiscard]] const points_type& points(std::size_t element) const {
        return points_.at(element);
    }

private:
    std::vector<points_type> points_;
};

class GaussPointMaterialRelation {
public:
    [[nodiscard]] std::size_t size() const noexcept { return material_indices_.size(); }
    [[nodiscard]] std::size_t add(std::size_t material) {
        const auto index = material_indices_.size();
        material_indices_.push_back(material);
        return index;
    }
    [[nodiscard]] std::size_t material(std::size_t point) const {
        return material_indices_.at(point);
    }

private:
    std::vector<std::size_t> material_indices_;
};

} // namespace wavecore

#endif
