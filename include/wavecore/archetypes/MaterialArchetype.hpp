#ifndef WAVECORE_MATERIAL_ARCHETYPE_HPP
#define WAVECORE_MATERIAL_ARCHETYPE_HPP

#include <cstddef>
#include <utility>
#include <vector>

#include "wavecore/materials/IMaterialConcept.hpp"

namespace wavecore {

template <IMaterialConcept Material>
class MaterialArchetype {
public:
    using material_type = Material;

    [[nodiscard]] std::size_t size() const noexcept { return materials_.size(); }

    [[nodiscard]] std::size_t add_material(Material material) {
        const auto index = materials_.size();
        materials_.push_back(std::move(material));
        return index;
    }

    [[nodiscard]] Material& material(std::size_t index) { return materials_.at(index); }
    [[nodiscard]] const Material& material(std::size_t index) const { return materials_.at(index); }

private:
    std::vector<Material> materials_;
};

} // namespace wavecore

#endif
