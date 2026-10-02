#pragma once

#include <cstdint>
#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldRow.hpp"

namespace wavecore {
template <std::size_t Dimension, class T = const std::uint8_t>
struct ConstraintView {
    using Entry = typename DimensionEntries<Dimension>::Constraint;
    FieldView<Entry,T> fields;
    [[nodiscard]] std::size_t size() const noexcept { return fields.size(); }
    [[nodiscard]] auto operator[](std::size_t row) const noexcept {
        return ArrayRow<Entry,T>{fields,row};
    }
};
template <std::size_t Dimension>
class ConstraintMasks {
public:
    using Entry = typename DimensionEntries<Dimension>::Constraint;
    void add(const std::array<std::uint8_t,Dimension>& flags) { fields_.push_back(flags); }
    [[nodiscard]] std::size_t size() const noexcept { return fields_.size(); }
    [[nodiscard]] auto operator[](std::size_t row) const noexcept { return view()[row]; }
    [[nodiscard]] ConstraintView<Dimension> view() const noexcept { return {fields_.view()}; }
    [[nodiscard]] ConstraintView<Dimension,std::uint8_t> view() noexcept { return {fields_.view()}; }
private:
    FieldStorage<Entry,std::uint8_t> fields_;
};
} // namespace wavecore
