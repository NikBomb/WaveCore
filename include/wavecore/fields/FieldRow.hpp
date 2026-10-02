#pragma once

#include <algorithm>
#include <iterator>
#include "wavecore/fields/FieldStorage.hpp"
#include "wavecore/utils/Matrix.hpp"

namespace wavecore {
// A row is a proxy over components, not a contiguous array or an owning value.
template <FieldEntry Entry, class T>
class ArrayRow {
public:
    static constexpr auto width = FieldView<Entry, T>::entry_count;
    using array_type = std::array<std::remove_const_t<T>, width>;
    constexpr ArrayRow(FieldView<Entry, T> field, std::size_t row) : field_(field), row_(row) {}
    ArrayRow(const ArrayRow&) = default;
    ArrayRow& operator=(const ArrayRow& other) requires (!std::is_const_v<T>) {
        return *this = static_cast<array_type>(other);
    }
    [[nodiscard]] constexpr T& operator[](std::size_t entry) const noexcept {
        return field_[static_cast<Entry>(entry)][row_];
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return width; }
    operator array_type() const {
        array_type result{};
        for (std::size_t i = 0; i < width; ++i) result[i] = (*this)[i];
        return result;
    }
    ArrayRow& operator=(const array_type& value) requires (!std::is_const_v<T>) {
        for (std::size_t i = 0; i < width; ++i) (*this)[i] = value[i];
        return *this;
    }
    void fill(std::remove_const_t<T> value) const requires (!std::is_const_v<T>) {
        for (std::size_t i = 0; i < width; ++i) (*this)[i] = value;
    }
    [[nodiscard]] bool operator==(const array_type& other) const {
        for (std::size_t i = 0; i < width; ++i) if ((*this)[i] != other[i]) return false;
        return true;
    }
    struct Iterator {
        const ArrayRow* row; std::size_t entry;
        T& operator*() const { return (*row)[entry]; }
        Iterator& operator++() { ++entry; return *this; }
        bool operator==(const Iterator&) const = default;
    };
    [[nodiscard]] Iterator begin() const { return {this, 0}; }
    [[nodiscard]] Iterator end() const { return {this, width}; }
private:
    FieldView<Entry, T> field_;
    std::size_t row_;
};

template <FieldEntry Entry, class T, std::size_t Rows, std::size_t Columns>
class MatrixRow {
    static_assert(Rows * Columns == FieldView<Entry, T>::entry_count);
public:
    using matrix_type = Matrix<std::remove_const_t<T>, Rows, Columns>;
    MatrixRow(FieldView<Entry, T> field, std::size_t row) : field_(field), row_(row) {}
    MatrixRow(const MatrixRow&) = default;
    MatrixRow& operator=(const MatrixRow& other) requires (!std::is_const_v<T>) {
        return *this = static_cast<matrix_type>(other);
    }
    [[nodiscard]] T& operator()(std::size_t i, std::size_t j) const noexcept {
        return field_[static_cast<Entry>(i * Columns + j)][row_];
    }
    operator matrix_type() const {
        matrix_type result{};
        for (std::size_t i = 0; i < Rows; ++i)
            for (std::size_t j = 0; j < Columns; ++j) result(i,j) = (*this)(i,j);
        return result;
    }
    MatrixRow& operator=(const matrix_type& value) requires (!std::is_const_v<T>) {
        for (std::size_t i = 0; i < Rows; ++i)
            for (std::size_t j = 0; j < Columns; ++j) (*this)(i,j) = value(i,j);
        return *this;
    }
private:
    FieldView<Entry,T> field_;
    std::size_t row_;
};

template <FieldEntry Entry, class T, std::size_t Dimension>
class SymmetricMatrixRow {
public:
    using matrix_type = Matrix<std::remove_const_t<T>, Dimension, Dimension>;
    SymmetricMatrixRow(FieldView<Entry,T> field, std::size_t row) : field_(field), row_(row) {}
    SymmetricMatrixRow(const SymmetricMatrixRow&) = default;
    SymmetricMatrixRow& operator=(const SymmetricMatrixRow& other) requires (!std::is_const_v<T>) {
        return *this = static_cast<matrix_type>(other);
    }
    [[nodiscard]] T& operator()(std::size_t i, std::size_t j) const noexcept {
        if (i > j) std::swap(i,j);
        const auto entry = i * Dimension - i * (i + 1) / 2 + j;
        return field_[static_cast<Entry>(entry)][row_];
    }
    operator matrix_type() const {
        matrix_type result{};
        for (std::size_t i=0;i<Dimension;++i)
            for (std::size_t j=0;j<Dimension;++j) result(i,j)=(*this)(i,j);
        return result;
    }
    SymmetricMatrixRow& operator=(const matrix_type& value) requires (!std::is_const_v<T>) {
        for (std::size_t i=0;i<Dimension;++i)
            for (std::size_t j=i;j<Dimension;++j) (*this)(i,j)=value(i,j);
        return *this;
    }
private:
    FieldView<Entry,T> field_;
    std::size_t row_;
};
} // namespace wavecore
