#ifndef WAVECORE_QUADRATURE_POINT_HPP
#define WAVECORE_QUADRATURE_POINT_HPP

#include <cstddef>

#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

// Parent-domain coordinates and weight. Physical integration additionally
// requires the Jacobian determinant and any applicable section properties.
template <std::size_t Dimension>
struct QuadraturePoint {
    Vector<double, Dimension> coordinates;
    double weight;
};

} // namespace wavecore

#endif
