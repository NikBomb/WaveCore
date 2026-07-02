#ifndef wavecore_IELEMENT_HPP
#define wavecore_IELEMENT_HPP
#include <cstddef>
#include <span>
#include "wavecore/utils/Matrix.hpp"

namespace wavecore {
    struct IElement {

        template <typename Self>
        [[nodiscard]] double measure(this const Self& self) noexcept {
            return self.measure_impl();
        }

        template<typename Self>
        [[nodiscard]] double characteristic_length(this const Self& self) noexcept {
            return self.characteristic_length_impl();
        }
        
        template<typename Self>
        void gather(this Self& self, std::span<typename Self::node_type> nodes,
                    std::span<const std::size_t, Self::nodes_per_element> connectivity) noexcept {
            self.gather_impl(nodes, connectivity);
        }

        template<typename Self>
         [[nodiscard]] wavecore::Matrix<double, Self::dimension, Self::dimension> jacobian_matrix(this const Self& self, const wavecore::Vector<double, Self::dimension>& local_c) noexcept {
            return self.jacobian_matrix_impl(local_c);
         }
         
          template<typename Self>
          [[nodiscard]] double jacobian_determinant(this const Self& self, const wavecore::Vector<double, Self::dimension>& local_c) noexcept {
              return self.jacobian_determinant_impl(local_c);
          }
          
          private:

         [[nodiscard]] double measure_impl() const noexcept {
             return 0.0;
         }

         [[nodiscard]] double characteristic_length_impl() const noexcept {
             return 0.0;
         }
         
        template<typename Self>
          [[nodiscard]] wavecore::Matrix<double, Self::dimension, Self::dimension> jacobian_matrix_impl(const wavecore::Vector<double, Self::dimension>& /*local_c*/){
              return wavecore::Matrix<double, Self::dimension, Self::dimension>();
          }

         template<typename Self>
         [[nodiscard]] double jacobian_determinant_impl(const wavecore::Vector<double, Self::dimension>&/*local_c*/) noexcept {
             return 0.0;
         }


        template<typename Self>
        void gather_impl(std::span<const typename Self::node_type>, 
                         std::span<const std::size_t, Self::nodes_per_element>) noexcept {};

    };
}
#endif
