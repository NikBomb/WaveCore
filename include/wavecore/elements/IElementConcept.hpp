#ifndef wavecore_ELEMENT_TRAITS_HPP
#define wavecore_ELEMENT_TRAITS_HPP

#include <array>
#include <cstddef>
#include <concepts>
#include <span>
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"




namespace wavecore {

template<class ElementType>
using ElementVector = wavecore::Vector<double, ElementType::dimension>;

template< class ElementType>
using ElementMatrix = wavecore::Matrix<double, ElementType::dimension, ElementType::dimension>; 

template<class ElementType>
using ElementParentCoordinate = ElementVector<ElementType>;

template<class ElementType>
using ElementLocalVector = 
    std::array<ElementVector<ElementType>, ElementType::nodes_per_element>;

template <class ElementType>
using ElementConnectivity = 
    std::span<const size_t, ElementType::nodes_per_element>;

template<class ElementType>
using ElementCoordinatematrix = ElementMatrix<ElementType>;


template <class ElementType>
using ElementParentDerivativeMatrix =
    wavecore::Matrix<double, ElementType::dimension, ElementType::nodes_per_element>;


template <class ElementType>
using ElementPhysicalDerivativeMatrix =
    ElementParentDerivativeMatrix<ElementType>;

template <class ElementType>
using NodeSpan =
    std::span<typename ElementType::node_type>;

template <typename ElementType>
concept IElementConcept =
    requires {
      //---------------------------------------------
      //  Static element traits
      // --------------------------------------------

      { ElementType::dimension } -> std::convertible_to<size_t>;
      { ElementType::nodes_per_element } -> std::convertible_to<size_t>;
      { ElementType::gauss_points } -> std::convertible_to<size_t>;
      (ElementType::dimension == 2 || ElementType::dimension == 3) &&
          (ElementType::nodes_per_element > 0) &&
          (ElementType::gauss_points > 0);

      typename ElementType::node_type;
      requires std::same_as<typename ElementType::node_type,
                            wavecore::Node<ElementType::dimension>>;
    } && requires(ElementType element, const ElementType const_element,
                  NodeSpan<ElementType> nodes,
                  ElementConnectivity<ElementType> connectivity,
                  const ElementParentCoordinate<ElementType>& parent_coordinate,
                  ElementLocalVector<ElementType> local_force) {
      // -------------------------------------------------------------------------
      // Mutating operations
      // -------------------------------------------------------------------------

      { element.gather(nodes, connectivity) } -> std::same_as<void>;

      //{ element.scatter_force(local_force) } -> std::same_as<void>;

      // -------------------------------------------------------------------------
      // Const geometry operations
      // -------------------------------------------------------------------------

      { const_element.measure() } -> std::same_as<double>;

      { const_element.characteristic_length() } -> std::same_as<double>;

      //{
      //  const_element.derivative_matrix(parent_coordinate)
     // } -> std::same_as<ElementParentDerivativeMatrix<ElementType>>;

       { const_element.jacobian_matrix(parent_coordinate) } -> std::same_as<ElementMatrix<ElementType>>;
       
       { const_element.jacobian_determinant(parent_coordinate) } -> std::same_as<double>;

       {const_element.strain_rate_tensor(parent_coordinate)} -> std::same_as<Matrix<double, ElementType::dimension, ElementType::dimension>>; 

     // { const_element.det_jacobian(parent_coordinate) } -> std::same_as<double>;

      //{
      //  const_element.inverse_jacobian(parent_coordinate)
      //} -> std::same_as<ElementMatrix<ElementType>>;

      //{
      //  const_element.physical_derivative_matrix(parent_coordinate)
      //} -> std::same_as<ElementPhysicalDerivativeMatrix<ElementType>>;

      // -------------------------------------------------------------------------
      // Const kinematic operations
      // -------------------------------------------------------------------------

      //{
      //  const_element.velocity_gradient(parent_coordinate)
      //} -> std::same_as<ElementMatrix<ElementType>>;

      //{
      //  const_element.rate_of_deformation(parent_coordinate)
      //} -> std::same_as<ElementMatrix<ElementType>>;
    };
}
#endif