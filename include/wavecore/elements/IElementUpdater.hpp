#ifndef WAVECORE_IELEMENTUPDATER_HPP
#define WAVECORE_IELEMENTUPDATER_HPP
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
        void gather(this Self& self, std::span<typename Self::node_type>) noexcept {
            self.gather_impl();
        }
        
        private:
        
        [[nodiscard]] double measure_impl() const noexcept {
            return 0.0;
        }

        [[nodiscard]] double characteristic_length_impl() const noexcept {
            return 0.0;
        }
        
        template<typename Self>
        void gather_impl(std::span<typename Self::node_type>) {};

    };
}
#endif