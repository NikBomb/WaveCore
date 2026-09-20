#ifndef WAVECORE_SCATTER_FORCE_HPP
#define WAVECORE_SCATTER_FORCE_HPP

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>

#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

// Accumulate positive internal forces in connectivity order. The caller clears
// nodal forces once before an assembly pass, not between elements or blocks.
// Concurrent calls that share nodes require synchronization by the caller.
template <std::size_t Dimension, std::size_t NodeCount, std::size_t NodeExtent>
void scatter_force(
    std::span<Node<Dimension>, NodeExtent> nodes,
    const std::array<std::size_t, NodeCount>& connectivity,
    const std::array<Vector<double, Dimension>, NodeCount>& local_forces) {
    // Validate before changing any node so a bad index cannot partially scatter.
    for (const auto index : connectivity) {
        if (index >= nodes.size()) {
            throw std::out_of_range("Force scattering connectivity exceeds the node span");
        }
    }
    for (std::size_t local = 0; local < NodeCount; ++local) {
        auto& force = nodes[connectivity[local]].internal_force();
        for (std::size_t d = 0; d < Dimension; ++d) {
            force[d] += local_forces[local](d);
        }
    }
}

} // namespace wavecore

#endif
