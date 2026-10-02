#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace wavecore {
template <class E>
concept FieldEntry = std::is_enum_v<E> && requires { E::count; };

// Views never own memory. Constness belongs to T, as with span<T>.
// Host owners create host views. A future device owner must supply accessible
// device allocations; copying these host pointers does not make a device view.
template <FieldEntry Entry, class T>
class FieldView {
public:
    static constexpr auto entry_count = static_cast<std::size_t>(Entry::count);
    using value_type = T;
    using entry_type = Entry;
    constexpr FieldView() = default;
    constexpr FieldView(std::array<T*, entry_count> pointers, std::size_t rows)
        : pointers_(pointers), rows_(rows) {}
    template <class U> requires std::convertible_to<U*, T*>
    constexpr FieldView(const FieldView<Entry, U>& other) : rows_(other.size()) {
        for (std::size_t i = 0; i < entry_count; ++i)
            pointers_[i] = other[static_cast<Entry>(i)].data();
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return rows_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return rows_ == 0; }
    [[nodiscard]] constexpr std::span<T> operator[](Entry entry) const noexcept {
        const auto i = static_cast<std::size_t>(entry);
        assert(i < entry_count);
        return {pointers_[i], rows_};
    }
    [[nodiscard]] constexpr FieldView subview(std::size_t first, std::size_t count) const {
        if (first > rows_ || count > rows_ - first)
            throw std::out_of_range("Field subview exceeds storage");
        auto pointers = pointers_;
        if (first != 0) for (auto& p : pointers) p += first;
        return {pointers, count};
    }
private:
    std::array<T*, entry_count> pointers_{};
    std::size_t rows_ = 0;
};

template <class T>
class ScalarView {
public:
    using value_type = T;
    constexpr ScalarView() = default;
    constexpr ScalarView(T* pointer, std::size_t rows) : pointer_(pointer), rows_(rows) {}
    template <class U> requires std::convertible_to<U*, T*>
    constexpr ScalarView(ScalarView<U> other) : pointer_(other.data()), rows_(other.size()) {}
    [[nodiscard]] constexpr T& operator[](std::size_t row) const noexcept {
        assert(row < rows_); return pointer_[row];
    }
    [[nodiscard]] constexpr T* data() const noexcept { return pointer_; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return rows_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return rows_ == 0; }
    [[nodiscard]] constexpr ScalarView subview(std::size_t first, std::size_t count) const {
        if (first > rows_ || count > rows_ - first)
            throw std::out_of_range("Scalar subview exceeds storage");
        return {first == 0 ? pointer_ : pointer_ + first, count};
    }
private:
    T* pointer_ = nullptr;
    std::size_t rows_ = 0;
};

// All components have the same logical row count. Structural operations may
// invalidate views: destruction/assignment/reallocation invalidates pointers,
// resize/clear invalidates captured extents. Reacquire after structural changes.
// Moves transfer allocations; prior views then depend on the destination owner.
template <FieldEntry Entry, class T = double>
class FieldStorage {
    static_assert(!std::is_const_v<T> && !std::same_as<T, bool>);
public:
    static constexpr auto entry_count = static_cast<std::size_t>(Entry::count);
    static_assert(entry_count > 0);
    using entry_type = Entry;
    using value_type = T;
    FieldStorage() = default;
    explicit FieldStorage(std::size_t rows) { resize(rows); }
    [[nodiscard]] std::size_t size() const noexcept { return arrays_[0].size(); }
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    void reserve(std::size_t rows) { for (auto& array : arrays_) array.reserve(rows); }
    void resize(std::size_t rows) {
        // Reserve all before changing logical sizes. A failed allocation can
        // invalidate views, but cannot leave mismatched component row counts.
        reserve(rows);
        const auto old = size();
        try { for (auto& array : arrays_) array.resize(rows); }
        catch (...) { for (auto& array : arrays_) array.resize(old); throw; }
    }
    void clear() noexcept { for (auto& array : arrays_) array.clear(); }
    void push_back(const std::array<T, entry_count>& row) {
        const auto old = size();
        // Preserve vector's geometric growth rather than reserve(size+1).
        if (old == arrays_[0].capacity()) reserve(old == 0 ? 1 : 2 * old);
        try { for (std::size_t i = 0; i < entry_count; ++i) arrays_[i].push_back(row[i]); }
        catch (...) { for (auto& array : arrays_) array.resize(old); throw; }
    }
    [[nodiscard]] std::span<T> operator[](Entry entry) noexcept { return view()[entry]; }
    [[nodiscard]] std::span<const T> operator[](Entry entry) const noexcept { return view()[entry]; }
    [[nodiscard]] FieldView<Entry, T> view() noexcept {
        std::array<T*, entry_count> pointers{};
        for (std::size_t i = 0; i < entry_count; ++i) pointers[i] = arrays_[i].data();
        return {pointers, size()};
    }
    [[nodiscard]] FieldView<Entry, const T> view() const noexcept {
        std::array<const T*, entry_count> pointers{};
        for (std::size_t i = 0; i < entry_count; ++i) pointers[i] = arrays_[i].data();
        return {pointers, size()};
    }
private:
    std::array<std::vector<T>, entry_count> arrays_;
};

template <class T = double>
class ScalarStorage {
    static_assert(!std::is_const_v<T> && !std::same_as<T, bool>);
public:
    using value_type = T;
    ScalarStorage() = default;
    explicit ScalarStorage(std::size_t rows) : values_(rows) {}
    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }
    [[nodiscard]] bool empty() const noexcept { return values_.empty(); }
    void resize(std::size_t rows) { values_.resize(rows); }
    void reserve(std::size_t rows) { values_.reserve(rows); }
    void clear() noexcept { values_.clear(); }
    void push_back(T value) { values_.push_back(std::move(value)); }
    template <class... Args> T& emplace_back(Args&&... args) {
        return values_.emplace_back(std::forward<Args>(args)...);
    }
    [[nodiscard]] T& operator[](std::size_t row) noexcept { return values_[row]; }
    [[nodiscard]] const T& operator[](std::size_t row) const noexcept { return values_[row]; }
    [[nodiscard]] T& at(std::size_t row) { return values_.at(row); }
    [[nodiscard]] const T& at(std::size_t row) const { return values_.at(row); }
    [[nodiscard]] ScalarView<T> view() noexcept { return {values_.data(), size()}; }
    [[nodiscard]] ScalarView<const T> view() const noexcept { return {values_.data(), size()}; }
private:
    std::vector<T> values_;
};
} // namespace wavecore
