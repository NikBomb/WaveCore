#ifndef WAVECORE_QUAD4_HPP
#define WAVECORE_QUAD4_HPP

#include <cstddef>
#include <span>

#include "wavecore/elements/IElementUpdater.hpp"
#include "wavecore/mesh/Node.hpp"



namespace wavecore {
    class Quad4 : public IElementUpdater {
        public:
        
        using node_type = wavecore::Node2D;

        static constexpr size_t dimension = 2;
        static constexpr size_t gauss_points = 1;
        static constexpr size_t nodes_per_element = 4;

        private:
        friend struct IElementUpdater;
        [[nodiscard]] double measure_impl() const noexcept {
            return 4.0;
        }
    };
}
#endif