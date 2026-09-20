#ifndef WAVECORE_PLANE_ELEMENT_PROPERTIES_HPP
#define WAVECORE_PLANE_ELEMENT_PROPERTIES_HPP

#include <cmath>
#include <stdexcept>

namespace wavecore {

class PlaneElementProperties {
public:
    explicit PlaneElementProperties(double thickness) : thickness_(thickness) {
        if (!std::isfinite(thickness) || thickness <= 0.0) {
            throw std::invalid_argument("Thickness must be finite and positive");
        }
    }

    [[nodiscard]] double thickness() const noexcept { return thickness_; }

private:
    double thickness_;
};

} // namespace wavecore

#endif
