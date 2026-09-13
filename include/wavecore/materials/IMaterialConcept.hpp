#ifndef WAVECORE_MATERIAL_CONCEPT_HPP
#define WAVECORE_MATERIAL_CONCEPT_HPP

#include <concepts>
#include <cstddef>

#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

template <class MaterialType>
using MaterialTensor =
    wavecore::Matrix<double, MaterialType::dimension, MaterialType::dimension>;

template <class MaterialType>
using MaterialState = typename MaterialType::state_type;

// Material parameters are read-only; history is stored in a separate state for
// each integration point. Tensors match the element's strain_rate_tensor type.
template <typename MaterialType>
concept IMaterialConcept =
    requires {
        // Static material traits
        { MaterialType::dimension } -> std::convertible_to<std::size_t>;
        requires (MaterialType::dimension == 2 || MaterialType::dimension == 3);

        typename MaterialType::state_type;
        typename MaterialType::tensor_type;
        requires std::same_as<typename MaterialType::tensor_type,
                              MaterialTensor<MaterialType>>;
    } && requires(const MaterialType& material,
                  MaterialState<MaterialType>& state,
                  const MaterialState<MaterialType>& const_state,
                  const MaterialTensor<MaterialType>& strain_rate,
                  double dt) {
        // State initialization and mutation
        { material.initial_state() } -> std::same_as<MaterialState<MaterialType>>;
        { material.update(state, strain_rate, dt) } -> std::same_as<void>;

        // Const material and state queries
        { material.density() } -> std::same_as<double>;
        { material.stress(const_state) }
            -> std::same_as<const MaterialTensor<MaterialType>&>;
    };

} // namespace wavecore

#endif
