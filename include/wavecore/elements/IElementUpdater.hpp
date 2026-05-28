#ifndef WAVECORE_IELEMENTUPDATER_HPP
#define WAVECORE_IELEMENTUPDATER_HPP
#include <cstddef>
#include <span>

namespace wavecore {
    struct IElementUpdater {
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
        
        private:
        
        [[nodiscard]] double measure_impl() const noexcept {
            return 0.0;
        }

        [[nodiscard]] double characteristic_length_impl() const noexcept {
            return 0.0;
        }
        
        template<typename Self>
        void gather_impl(std::span<const typename Self::node_type>, 
                         std::span<const std::size_t, Self::nodes_per_element>) noexcept {};

    };
}
#endif