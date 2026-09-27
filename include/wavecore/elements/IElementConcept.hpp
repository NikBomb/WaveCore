#ifndef WAVECORE_ELEMENT_CONCEPT_HPP
#define WAVECORE_ELEMENT_CONCEPT_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <span>

#include "wavecore/elements/QuadraturePoint.hpp"
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

template <class E> using ElementVector = Vector<double, E::dimension>;
template <class E> using ElementMatrix = Matrix<double, E::dimension, E::dimension>;
template <class E> using ElementQuadrature =
    std::array<QuadraturePoint<E::dimension>, E::gauss_points>;
template <class E> using ElementLocalVector =
    std::array<ElementVector<E>, E::nodes_per_element>;
template <class E> using ElementStressSpan =
    std::span<const ElementMatrix<E>, E::gauss_points>;
template <class E> using ElementConnectivity =
    std::span<const std::size_t, E::nodes_per_element>;
template <class E> using ConstNodeSpan = std::span<const typename E::node_type>;

template <typename E>
concept IElementConcept = requires {
    { E::dimension } -> std::convertible_to<std::size_t>;
    { E::nodes_per_element } -> std::convertible_to<std::size_t>;
    { E::gauss_points } -> std::convertible_to<std::size_t>;
    requires (E::dimension == 2 || E::dimension == 3);
    requires (E::nodes_per_element > 0);
    requires (E::gauss_points > 0);
    typename E::node_type;
    typename E::properties_type;
    typename E::geometry_state_type;
    typename E::nodal_velocity_type;
} && requires(const E element, ConstNodeSpan<E> nodes,
              ElementConnectivity<E> connectivity,
              typename E::geometry_state_type& geometry,
              typename E::nodal_velocity_type& velocities,
              const typename E::geometry_state_type& const_geometry,
              const typename E::nodal_velocity_type& const_velocities,
              ElementStressSpan<E> stresses,
              const typename E::properties_type& properties) {
    { element.quadrature() } -> std::same_as<ElementQuadrature<E>>;
    { element.refresh_geometry(nodes, connectivity, geometry) } -> std::same_as<void>;
    { element.gather_velocities(nodes, connectivity, velocities) } -> std::same_as<void>;
    { element.strain_rate_tensor(const_geometry, const_velocities) }
        -> std::same_as<ElementMatrix<E>>;
    { element.internal_force(stresses, properties, const_geometry) }
        -> std::same_as<ElementLocalVector<E>>;
};

} // namespace wavecore

#endif
