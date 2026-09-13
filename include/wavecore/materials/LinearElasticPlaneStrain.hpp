#ifndef WAVECORE_LINEAR_ELASTIC_PLANE_STRAIN_HPP
#define WAVECORE_LINEAR_ELASTIC_PLANE_STRAIN_HPP

#include <cmath>
#include <stdexcept>

#include "wavecore/materials/IMaterialConcept.hpp"

namespace wavecore {

// Isotropic small-strain elasticity with epsilon_zz = epsilon_xz = epsilon_yz = 0.
// Use consistent units for Young's modulus, density, strain rate, and time.
class LinearElasticPlaneStrain {
public:
    static constexpr std::size_t dimension = 2;
    using tensor_type = MaterialTensor<LinearElasticPlaneStrain>;

    struct state_type {
        tensor_type stress{};
        // Plane strain generally produces a nonzero out-of-plane normal stress.
        double stress_zz = 0.0;
    };

    LinearElasticPlaneStrain(double young_modulus, double poisson_ratio,
                             double density)
        : density_(density) {
        if (!std::isfinite(young_modulus) || young_modulus <= 0.0) {
            throw std::invalid_argument("Young's modulus must be finite and positive");
        }
        if (!std::isfinite(poisson_ratio) || poisson_ratio <= -1.0 ||
            poisson_ratio >= 0.5) {
            throw std::invalid_argument("Poisson's ratio must be finite and in (-1, 0.5)");
        }
        if (!std::isfinite(density) || density <= 0.0) {
            throw std::invalid_argument("Density must be finite and positive");
        }

        shear_modulus_ = young_modulus / (2.0 * (1.0 + poisson_ratio));
        lambda_ = young_modulus / (1.0 + poisson_ratio) *
                  (poisson_ratio / (1.0 - 2.0 * poisson_ratio));
        if (!std::isfinite(shear_modulus_) || !std::isfinite(lambda_)) {
            throw std::invalid_argument("Elastic moduli exceed the representable range");
        }
    }

    [[nodiscard]] double density() const noexcept { return density_; }

    [[nodiscard]] state_type initial_state() const { return {}; }

    // strain_rate is a finite, symmetric small-strain-rate tensor: its xy entry
    // is half the engineering shear rate. Integrate d_sigma = lambda tr(d_epsilon) I
    // + 2 mu d_epsilon, assuming a constant rate over dt. No finite-rotation correction.
    void update(state_type& state, const tensor_type& strain_rate, double dt) const {
        if (!std::isfinite(dt) || dt < 0.0) {
            throw std::invalid_argument("Time step must be finite and nonnegative");
        }
        if (dt == 0.0) {
            return;
        }

        const double volumetric_increment =
            lambda_ * (strain_rate(0, 0) + strain_rate(1, 1)) * dt;
        for (std::size_t i = 0; i < dimension; ++i) {
            for (std::size_t j = 0; j < dimension; ++j) {
                state.stress(i, j) += 2.0 * shear_modulus_ * strain_rate(i, j) * dt;
            }
            state.stress(i, i) += volumetric_increment;
        }
        state.stress_zz += volumetric_increment;
    }

    [[nodiscard]] const tensor_type& stress(const state_type& state) const noexcept {
        return state.stress;
    }

private:
    double density_;
    double shear_modulus_;
    double lambda_;
};

static_assert(IMaterialConcept<LinearElasticPlaneStrain>);

} // namespace wavecore

#endif
