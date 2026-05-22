#ifndef WAVECORE_MESH_NODE_HPP
#define WAVECORE_MESH_NODE_HPP
#include <array>
#include <cstddef>


namespace wavecore {
    template<std::size_t Dimension>
    struct Node {
        static_assert(
            Dimension == 2 || Dimension == 3,
            "WaveCore only supports 2D and 3D nodes"
        );

        std::array<double,Dimension>  x{}; // Coordinates
        std::array<double, Dimension> u{};
        std::array<double, Dimension> v{};
        std::array<double, Dimension> a{};

        double mass = 0.0;

        std::array<double, Dimension> f_int{};
        std::array<double, Dimension> f_ext{};       
    };
    
    using Node2D = Node<2>;
    using Node3D = Node<3>;
}

#endif