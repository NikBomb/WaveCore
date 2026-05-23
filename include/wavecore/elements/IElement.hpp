#ifndef WAVECORE_ELEMENT_TRAITS_HPP
#define WAVECORE_ELEMENT_TRAITS_HPP

#include <cstddef>
#include <concepts>


namespace wavecore {

    template <typename ElementType>
    concept IElement = requires {
        {ElementType::dimension} -> std::convertible_to<size_t>;
        {ElementType::nodes_per_element} -> std::convertible_to<size_t>;
        {ElementType::gauss_points} -> std::convertible_to<size_t>;
    }  &&
    (ElementType::dimension == 2 || ElementType::dimension == 3) &&
    (ElementType::nodes_per_element > 0) &&
    (ElementType::gauss_points > 0);
}
#endif