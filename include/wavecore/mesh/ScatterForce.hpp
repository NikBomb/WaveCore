#ifndef WAVECORE_SCATTER_FORCE_HPP
#define WAVECORE_SCATTER_FORCE_HPP

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>

#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"
#include "wavecore/fields/FieldStorage.hpp"

namespace wavecore {

// Assembly needs only the mutable force field, not every nodal component.
template <std::size_t Dimension, std::size_t NodeCount, FieldEntry Entry>
void scatter_force(
    FieldView<Entry,double> forces,
    const std::array<std::size_t,NodeCount>& connectivity,
    const std::array<Vector<double,Dimension>,NodeCount>& local_forces) {
    static_assert(FieldView<Entry,double>::entry_count == Dimension);
    for (const auto row:connectivity)
        if (row >= forces.size()) throw std::out_of_range("Force scattering connectivity exceeds node storage");
    for (std::size_t local=0;local<NodeCount;++local)
        for (std::size_t d=0;d<Dimension;++d)
            forces[static_cast<Entry>(d)][connectivity[local]] += local_forces[local](d);
}

// Accumulate positive internal forces in connectivity order. The caller clears
// nodal forces once before an assembly pass, not between elements or blocks.
// Concurrent calls that share nodes require synchronization by the caller.
template <std::size_t Dimension, std::size_t NodeCount, class Nodes>
void scatter_force(
    Nodes&& nodes,
    const std::array<std::size_t, NodeCount>& connectivity,
    const std::array<Vector<double, Dimension>, NodeCount>& local_forces) {
    // Validate before changing any node so a bad index cannot partially scatter.
    for (const auto index : connectivity) {
        if (index >= nodes.size()) {
            throw std::out_of_range("Force scattering connectivity exceeds the node span");
        }
    }
    for (std::size_t local = 0; local < NodeCount; ++local) {
        auto&& force = nodes[connectivity[local]].internal_force();
        for (std::size_t d = 0; d < Dimension; ++d) {
            force[d] += local_forces[local](d);
        }
    }
}

} // namespace wavecore

#endif
