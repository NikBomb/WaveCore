#ifndef wavecore_QUAD4_HPP
#define wavecore_QUAD4_HPP

#include <array>
#include <cstddef>
#include <span>
#include <cmath>

#include "wavecore/elements/IElement.hpp"
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"



namespace wavecore {
    class Quad4 : public IElement {
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
        static constexpr size_t num_edges = 4;
        
        private:
        std::array<node_type_ptr, 4> nodes_{nullptr,nullptr,nullptr,nullptr};
        wavecore::Matrix<double, nodes_per_element, dimension> coordinates_matrix_;
        wavecore::Matrix<double, nodes_per_element, dimension> nodal_velocities_;


        friend struct IElement;

/*
           Return the Jacobian matrix from parent domain to physical domain evaluated at csi, eta. (vectors are columns) 
        */

         wavecore::Matrix<double, dimension, dimension> jacobian_matrix_impl(const wavecore::Vector<double, dimension>& local_coordinates) const noexcept {
            return derivatives_in_parent_domain(local_coordinates, coordinates_matrix_);
        }

        [[nodiscard]] double jacobian_determinant_impl(wavecore::Vector<double, dimension> local_coordinates) const noexcept {
            auto jacobian_mat = jacobian_matrix_impl(local_coordinates);
            return wavecore::determinant(jacobian_mat);
        }
        
        [[nodiscard]] double measure_impl() const noexcept {
            auto coords = wavecore::Vector<double, 2>({0,0});
            return 4 * jacobian_determinant_impl(coords);
        }

        void gather_impl(std::span<node_type> nodes, 
                         std::span<const size_t, nodes_per_element> local_connectivity) noexcept 
        {
            for (std::size_t inode = 0; inode < nodes_per_element; inode++) {
                nodes_[inode] = &nodes[local_connectivity[inode]];
            }

            for (size_t inode = 0; inode < nodes_per_element; ++inode) {
                coordinates_matrix_( inode, 0) = nodes_[inode] -> coordinates()[0];
                coordinates_matrix_(  inode, 1) = nodes_[inode] -> coordinates()[1];
                nodal_velocities_(inode, 0) = nodes_[inode] -> velocity()[0];
                nodal_velocities_(inode, 1) = nodes_[inode] -> velocity()[1];
            }


        }

        /* return characteristic length as minimum of 4 edges
        1-2, 2-3, 3-4, 4-1 */
        double characteristic_length_impl() const noexcept {
        
            double min_edge_length = 1e10;
            for (size_t iedge = 0; iedge < num_edges; iedge++) {
                double edge_length = std::sqrt(std::pow(coordinates_matrix_(iedge, 0) - coordinates_matrix_( (iedge + 1) % nodes_per_element, 0),2 ) +
                                            std::pow(coordinates_matrix_(iedge, 1) - coordinates_matrix_( (iedge + 1) % nodes_per_element, 1),2 ));
                if (edge_length < min_edge_length)
                    min_edge_length = edge_length; 
            }
    
            return min_edge_length;
        }

        
        [[nodiscard]] wavecore::Matrix<double, dimension, nodes_per_element> derivatives_shape_functions_parent(const wavecore::Vector<double, dimension>& local_c) const noexcept {
            auto csi = local_c(0);
            auto eta = local_c(1);
            wavecore::Matrix<double, dimension, nodes_per_element> derivative_matrix;
            derivative_matrix(0, 0) = -0.25 * (1.0 - eta);
            derivative_matrix(0, 1) =  0.25 * (1.0 - eta);
            derivative_matrix(0, 2) =  0.25 * (1.0 + eta);
            derivative_matrix(0, 3) = -0.25 * (1.0 + eta);
            derivative_matrix(1, 0) = -0.25 * (1.0 - csi);
            derivative_matrix(1, 1) = -0.25 * (1.0 + csi);
            derivative_matrix(1, 2) =  0.25 * (1.0 + csi);
            derivative_matrix(1, 3) =  0.25 * (1.0 - csi);
            return derivative_matrix;  
        }

        
        [[nodiscard]] wavecore::Matrix<double, dimension, dimension> derivatives_in_parent_domain(const wavecore::Vector<double, dimension>& local_c, const wavecore::Matrix<double, nodes_per_element, dimension>& field) const noexcept {
            return derivatives_shape_functions_parent(local_c) * field;            
        }
            
        wavecore::Matrix<double, dimension, dimension> strain_rate_tensor_impl(wavecore::Vector<double, dimension> /*local_coordinates*/) const {
            auto strain_rate_tensor_parent = derivatives_in_parent_domain({0,0}, nodal_velocities_);
            return wavecore::symmetric(wavecore::inverse(jacobian_matrix_impl({0,0})) * strain_rate_tensor_parent);
        }



    };

   
}
#endif
