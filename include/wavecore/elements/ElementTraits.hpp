#ifndef WAVECORE_ELEMENT_TRAITS_HPP
#define WAVECORE_ELEMENT_TRAITS_HPP

#include "wavecore/elements/Quad4.hpp"
#include <cstddef>

namespace wavecore {

    template <typename ElementType>
    class ElementTraits {
    public:
        static constexpr bool is_specialized = false;

        static constexpr std::size_t dimension = 0;
        static constexpr std::size_t nodes_per_element = 0;
        static constexpr std::size_t gauss_points = 0;
    };

    template <>
    struct ElementTraits<Quad4> {
        static constexpr bool is_specialized = true;
        static constexpr std::size_t dimension = 2;
        static constexpr std::size_t gauss_points = 1;
        static constexpr std::size_t nodes_per_element = 4; 
    };

    template <typename ElementType>
    concept ElementTraitsDefined =
    ElementTraits<ElementType>::is_specialized &&
    (ElementTraits<ElementType>::dimension == 2 ||
     ElementTraits<ElementType>::dimension == 3) &&
    (ElementTraits<ElementType>::nodes_per_element > 0) &&
    (ElementTraits<ElementType>::gauss_points > 0);
}
#endif