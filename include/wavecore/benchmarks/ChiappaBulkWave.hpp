#ifndef WAVECORE_CHIAPPA_BULK_WAVE_HPP
#define WAVECORE_CHIAPPA_BULK_WAVE_HPP

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

// Closed-form reference solution from Chiappa et al.,
// "An analytical benchmark for a 2D problem of elastic wave propagation in a
// solid", Engineering Structures 229 (2021) 111655, equations (21)-(27).
// The default parameters are the paper's bulk-wave case.
class ChiappaBulkWave {
public:
    struct Parameters {
        double a = 1.0;
        double b = 1.0;
        double young_modulus = 209.0e9;
        double poisson_ratio = 1.0 / 3.0;
        double density = 7800.0;
        double patch_x = 0.45;       // h3
        double patch_y = 0.45;       // b - h1 - h2
        double patch_width = 0.1;    // h4
        double patch_height = 0.1;   // h2
        double horizontal_velocity = 1.0; // F1
        double vertical_velocity = 0.0;   // F2
        std::size_t truncation = 80;
    };

    using vector_type = Vector<double, 2>;

    ChiappaBulkWave() : ChiappaBulkWave(Parameters{}) {}

    explicit ChiappaBulkWave(Parameters parameters) : p_(parameters) {
        if (p_.a <= 0.0 || p_.b <= 0.0 || p_.density <= 0.0 ||
            p_.young_modulus <= 0.0 || p_.poisson_ratio <= -1.0 ||
            p_.poisson_ratio >= 0.5 || p_.truncation == 0)
            throw std::invalid_argument("Invalid Chiappa benchmark parameters");
        shear_modulus_ = p_.young_modulus / (2.0 * (1.0 + p_.poisson_ratio));
        c_longitudinal_ = std::sqrt(
            p_.young_modulus * (1.0 - p_.poisson_ratio) /
            (p_.density * (1.0 + p_.poisson_ratio) *
             (1.0 - 2.0 * p_.poisson_ratio)));
        c_transverse_ = std::sqrt(shear_modulus_ / p_.density);
    }

    [[nodiscard]] double longitudinal_speed() const noexcept { return c_longitudinal_; }
    [[nodiscard]] double transverse_speed() const noexcept { return c_transverse_; }

    [[nodiscard]] vector_type displacement(double x, double y, double time) const {
        vector_type result{};
        for (std::size_t m = 0; m <= p_.truncation; ++m) {
            const double mu = pi() * static_cast<double>(m) / p_.b;
            for (std::size_t n = 1; n <= p_.truncation; ++n) {
                const double lambda = pi() * static_cast<double>(n) / p_.a;
                const double omega_l = c_longitudinal_ * std::sqrt(lambda * lambda + mu * mu);
                const double g1 = coefficient_g1(lambda, mu, m);
                const double g2 = m == 0 ? 0.0 : coefficient_g2(lambda, mu, n);
                const double a1 = (-lambda * g1 - mu * g2) /
                                  (omega_l * (lambda * lambda + mu * mu));
                const double longitudinal = std::sin(omega_l * time);
                result(0) += -lambda * a1 * std::sin(lambda * x) *
                             std::cos(mu * y) * longitudinal;
                result(1) += -mu * a1 * std::cos(lambda * x) *
                             std::sin(mu * y) * longitudinal;

                if (m == 0)
                    continue;
                const double omega_t = c_transverse_ * std::sqrt(lambda * lambda + mu * mu);
                const double b1 = (mu * g1 - lambda * g2) /
                                  (omega_t * (lambda * lambda + mu * mu));
                const double transverse = std::sin(omega_t * time);
                result(0) += mu * b1 * std::sin(lambda * x) *
                             std::cos(mu * y) * transverse;
                result(1) += -lambda * b1 * std::cos(lambda * x) *
                             std::sin(mu * y) * transverse;
            }
        }
        return result;
    }

    [[nodiscard]] vector_type initial_velocity(double x, double y) const noexcept {
        const bool inside = x >= p_.patch_x && x <= p_.patch_x + p_.patch_width &&
                            y >= p_.patch_y && y <= p_.patch_y + p_.patch_height;
        return inside ? vector_type{p_.horizontal_velocity, p_.vertical_velocity}
                      : vector_type{};
    }

    [[nodiscard]] vector_type analytical_velocity(
        double x, double y, double time) const {
        vector_type result{};
        for (std::size_t m = 0; m <= p_.truncation; ++m) {
            const double mu = pi() * static_cast<double>(m) / p_.b;
            for (std::size_t n = 1; n <= p_.truncation; ++n) {
                const double lambda = pi() * static_cast<double>(n) / p_.a;
                const double wave_number_squared = lambda * lambda + mu * mu;
                const double omega_l = c_longitudinal_ * std::sqrt(wave_number_squared);
                const double g1 = coefficient_g1(lambda, mu, m);
                const double g2 = m == 0 ? 0.0 : coefficient_g2(lambda, mu, n);
                const double a1 = (-lambda * g1 - mu * g2) /
                                  (omega_l * wave_number_squared);
                result(0) += -lambda * a1 * omega_l * std::sin(lambda * x) *
                             std::cos(mu * y) * std::cos(omega_l * time);
                result(1) += -mu * a1 * omega_l * std::cos(lambda * x) *
                             std::sin(mu * y) * std::cos(omega_l * time);

                if (m == 0)
                    continue;
                const double omega_t = c_transverse_ * std::sqrt(wave_number_squared);
                const double b1 = (mu * g1 - lambda * g2) /
                                  (omega_t * wave_number_squared);
                result(0) += mu * b1 * omega_t * std::sin(lambda * x) *
                             std::cos(mu * y) * std::cos(omega_t * time);
                result(1) += -lambda * b1 * omega_t * std::cos(lambda * x) *
                             std::sin(mu * y) * std::cos(omega_t * time);
            }
        }
        return result;
    }

private:
    [[nodiscard]] static constexpr double pi() noexcept {
        return 3.141592653589793238462643383279502884;
    }

    [[nodiscard]] double coefficient_g1(double lambda, double mu,
                                        std::size_t m) const noexcept {
        const double x_factor = std::cos(lambda * (p_.patch_x + p_.patch_width)) -
                                std::cos(lambda * p_.patch_x);
        if (m == 0)
            return -2.0 * p_.horizontal_velocity * x_factor * p_.patch_height /
                   (p_.a * p_.b * lambda);
        const double y_factor = std::sin(mu * (p_.patch_y + p_.patch_height)) -
                                std::sin(mu * p_.patch_y);
        return -4.0 * p_.horizontal_velocity * x_factor * y_factor /
               (p_.a * p_.b * lambda * mu);
    }

    [[nodiscard]] double coefficient_g2(double lambda, double mu,
                                        std::size_t n) const noexcept {
        const double y_factor = std::cos(mu * (p_.patch_y + p_.patch_height)) -
                                std::cos(mu * p_.patch_y);
        if (n == 0)
            return -2.0 * p_.vertical_velocity * y_factor * p_.patch_width /
                   (p_.a * p_.b * mu);
        const double x_factor = std::sin(lambda * (p_.patch_x + p_.patch_width)) -
                                std::sin(lambda * p_.patch_x);
        return -4.0 * p_.vertical_velocity * x_factor * y_factor /
               (p_.a * p_.b * lambda * mu);
    }

    Parameters p_;
    double shear_modulus_ = 0.0;
    double c_longitudinal_ = 0.0;
    double c_transverse_ = 0.0;
};

} // namespace wavecore

#endif // WAVECORE_CHIAPPA_BULK_WAVE_HPP
