#ifndef WAVECORE_IELEMENTUPDATER_HPP
#define WAVECORE_IELEMENTUPDATER_HPP

namespace wavecore {
    struct IElementUpdater {
        template <typename Self>
        [[nodiscard]] double measure(this Self&& self) noexcept {
            return self.measure_impl();
        }

        template<typename Self>
        [[nodiscard]] double characteristic_length(this const Self&& self) noexcept {
            return self.characteristic_length_impl();
        }
        
        private:
        
        [[nodiscard]] double measure_impl() const noexcept {
            return 0.0;
        }

        [[nodiscard]] double characteristic_length_impl() const noexcept {
            return 0.0;
        }


    };
}
#endif