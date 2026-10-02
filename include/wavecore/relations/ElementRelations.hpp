#ifndef WAVECORE_ELEMENT_RELATIONS_HPP
#define WAVECORE_ELEMENT_RELATIONS_HPP

#include <array>
#include <cstddef>
#include <utility>
#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldStorage.hpp"

#include "wavecore/elements/IElementConcept.hpp"

namespace wavecore {

// Every relation value is a storage row, not a stable entity ID. Each named
// slot has its own contiguous integer array. Reordering entities requires
// updating these external row mappings. Views are invalidated by growth.

template <IElementConcept Element>
class ElementNodeRelation {
    using Entry = typename ElementRelationEntries<Element>::Node;
    static_assert(static_cast<std::size_t>(Entry::count) == Element::nodes_per_element);
public:
    using connectivity_type = std::array<std::size_t, Element::nodes_per_element>;

    [[nodiscard]] std::size_t size() const noexcept { return connectivity_.size(); }
    [[nodiscard]] std::size_t add(connectivity_type connectivity) {
        const auto index = connectivity_.size();
        connectivity_.push_back(std::move(connectivity));
        return index;
    }
    [[nodiscard]] connectivity_type nodes(std::size_t element) const {
        if (element >= size()) throw std::out_of_range("Element node relation row exceeds storage");
        connectivity_type result{};
        for (std::size_t i=0;i<result.size();++i) result[i]=connectivity_[static_cast<Entry>(i)][element];
        return result;
    }

    [[nodiscard]] auto view() const noexcept { return connectivity_.view(); }
private:
    FieldStorage<Entry,std::size_t> connectivity_;
};

template <IElementConcept Element>
class ElementGaussPointRelation {
    using Entry = typename ElementRelationEntries<Element>::Point;
    static_assert(static_cast<std::size_t>(Entry::count) == Element::gauss_points);
public:
    using points_type = std::array<std::size_t, Element::gauss_points>;

    [[nodiscard]] std::size_t size() const noexcept { return points_.size(); }
    [[nodiscard]] std::size_t add(points_type points) {
        const auto index = points_.size();
        points_.push_back(std::move(points));
        return index;
    }
    [[nodiscard]] points_type points(std::size_t element) const {
        if (element >= size()) throw std::out_of_range("Element point relation row exceeds storage");
        points_type result{};
        for (std::size_t i=0;i<result.size();++i) result[i]=points_[static_cast<Entry>(i)][element];
        return result;
    }

    [[nodiscard]] auto view() const noexcept { return points_.view(); }
private:
    FieldStorage<Entry,std::size_t> points_;
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

    [[nodiscard]] auto view() const noexcept { return material_indices_.view(); }
private:
    ScalarStorage<std::size_t> material_indices_;
};

} // namespace wavecore

#endif
