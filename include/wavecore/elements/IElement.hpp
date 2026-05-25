#ifndef WAVECORE_ELEMENT_TRAITS_HPP
#define WAVECORE_ELEMENT_TRAITS_HPP

#include <cstddef>
#include <concepts>
#include <span>
#include "wavecore/mesh/Node.hpp"


namespace wavecore {

    template <typename ElementType>
    concept IElement = requires {
        
        {ElementType::dimension} -> std::convertible_to<size_t>;
        {ElementType::nodes_per_element} -> std::convertible_to<size_t>;
        {ElementType::gauss_points} -> std::convertible_to<size_t>;
        typename ElementType::node_type;
        requires std::same_as<typename ElementType::node_type, wavecore::Node<ElementType::dimension>>; 
    }  &&
    requires (ElementType elem) {    
        {elem.gather(std::declval<std::span<typename ElementType::node_type>>())} -> std::same_as<void>;
        
    } &&
    requires (ElementType const const_elem) {
        {const_elem.characteristic_length()} -> std::same_as<double>;
        {const_elem.measure()} -> std::same_as<double>;
    } &&
    (ElementType::dimension == 2 || ElementType::dimension == 3) &&
    (ElementType::nodes_per_element > 0) &&
    (ElementType::gauss_points > 0);
}
#endif