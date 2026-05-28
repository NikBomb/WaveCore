#ifndef WAVECORE_QUAD4_HPP
#define WAVECORE_QUAD4_HPP

#include <array>
#include <cstddef>
#include <span>

#include "wavecore/elements/IElementUpdater.hpp"
#include "wavecore/mesh/Node.hpp"



namespace wavecore {
    class Quad4 : public IElementUpdater {
        /*
            implementation of a Quad element with 4 nodes and one Gauss point in the center,
            Node numbering convention is: bottom left and counter-clockwise.
            
            N1 = 0.25 * (1 - csi) * (1 - eta)
            N2 = 0.25 * (1 + csi) * (1 - eta)
            N3 = 0.25 * (1 + csi) * (1 + eta)
            N4 = 0.25 * (1 - csi) * (1 + eta)

        */
        
        
        public:
        
        using node_type = wavecore::Node2D;
        using node_type_ptr = wavecore::Node2D*;

        static constexpr size_t dimension = 2;
        static constexpr size_t gauss_points = 1;
        static constexpr size_t nodes_per_element = 4;

        private:
        std::array<node_type_ptr, 4> nodes_{nullptr,nullptr,nullptr,nullptr};
        friend struct IElementUpdater;
        

        /*
           Return the Jacobian from parent domain to physical domain evaluated at csi, eta 
        */

        double jacobian(double csi = 0.0, double eta = 0.0) const noexcept {
            
            /* 
                Coordinates matrix
                X = [x1 y1; x2 y2; ...]

                Derivative Matrix = [dN1_dcsi dN2_dcsi DN3_dcsi DN4_dcsi;  .... ]
            */
            std::array<std::array<double, dimension>, nodes_per_element> coordinates_matrix;
            std::array<std::array<double, nodes_per_element>, dimension>
                derivative_matrix{{{{-0.25 * (1.0 - eta), 0.25 * (1.0 - eta),
                                     0.25 * (1.0 + eta), -0.25 * (1.0 + eta)}},
                                   {{-0.25 * (1.0 - csi), -0.25 * (1.0 + csi),
                                     0.25 * (1.0 + csi), 0.25 * (1.0 - csi)}}}};


            for (size_t inode = 0; inode < nodes_per_element; ++inode) {
                coordinates_matrix[inode] = nodes_[inode] -> coordinates();
            }

            std::array<std::array<double, dimension>, dimension> jacobian_matrix;

            for (std::size_t iparent_dim = 0; iparent_dim < dimension;
                 iparent_dim++) {
              for (std::size_t iphysical_dim = 0; iphysical_dim < dimension;
                   iphysical_dim++) {
                for (std::size_t inode = 0; inode < nodes_per_element;
                     inode++) {
                  jacobian_matrix[iparent_dim][iphysical_dim] +=
                      derivative_matrix[iparent_dim][inode] *
                      coordinates_matrix[inode][iphysical_dim];
                }
              }
            }

            return jacobian_matrix[0][0] * jacobian_matrix[1][1] - jacobian_matrix[0][1] * jacobian_matrix[1][0]; 
        }

        
        [[nodiscard]] double measure_impl() const noexcept {
            return 4 * jacobian();
        }

        void gather_impl(std::span<node_type> nodes, 
                         std::span<const size_t, nodes_per_element> local_connectivity) noexcept 
        {
            for (std::size_t inode = 0; inode < nodes_per_element; inode++) {
                nodes_[inode] = &nodes[local_connectivity[inode]];
            }

        }



    };
}
#endif