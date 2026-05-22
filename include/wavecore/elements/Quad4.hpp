#ifndef WAVECORE_QUAD4_HPP
#define WAVECORE_QUAD4_HPP

#include <cstddef>

namespace wavecore {
    class Quad4{
        public:
        static constexpr size_t dimension = 2;
        static constexpr size_t gauss_points = 1;
        static constexpr size_t nodes_per_element = 4;
    };
}
#endif