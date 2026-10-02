#pragma once

#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldRow.hpp"
#include "wavecore/mesh/Node.hpp"

namespace wavecore {
template <std::size_t Dimension, class T = double> struct NodeView;

template <std::size_t Dimension, class T>
struct NodeReference {
    static constexpr std::size_t dimension = Dimension;
    using Entries = DimensionEntries<Dimension>;
    NodeView<Dimension,T> fields;
    std::size_t row;
    [[nodiscard]] auto coordinates() const {
        return ArrayRow<typename Entries::Coordinate,const double>{fields.coordinates, row};
    }
    [[nodiscard]] auto displacement() const { return ArrayRow<typename Entries::Displacement,T>{fields.displacement,row}; }
    [[nodiscard]] auto velocity() const { return ArrayRow<typename Entries::Velocity,T>{fields.velocity,row}; }
    [[nodiscard]] auto acceleration() const { return ArrayRow<typename Entries::Acceleration,T>{fields.acceleration,row}; }
    [[nodiscard]] auto internal_force() const { return ArrayRow<typename Entries::Force,T>{fields.internal_force,row}; }
    [[nodiscard]] auto external_force() const { return ArrayRow<typename Entries::Force,T>{fields.external_force,row}; }
    [[nodiscard]] double mass() const { return fields.mass[row]; }
    void set_mass(double value) const requires (!std::is_const_v<T>) { fields.mass[row] = value; }
    [[nodiscard]] typename Node<Dimension>::Vector current_coordinates() const {
        return fields.current_coordinates(row);
    }
    operator Node<Dimension>() const {
        Node<Dimension> value{coordinates()};
        value.displacement() = displacement(); value.velocity() = velocity();
        value.acceleration() = acceleration(); value.internal_force() = internal_force();
        value.external_force() = external_force(); value.set_mass(mass());
        return value;
    }
    void assign(const Node<Dimension>& value) const requires (!std::is_const_v<T>) {
        ArrayRow<typename Entries::Coordinate,T>{fields.coordinates,row} = value.coordinates();
        displacement() = value.displacement(); velocity() = value.velocity();
        acceleration() = value.acceleration(); internal_force() = value.internal_force();
        external_force() = value.external_force(); set_mass(value.mass());
    }
    NodeReference& operator=(const Node<Dimension>& value) requires (!std::is_const_v<T>) {
        assign(value); return *this;
    }
    NodeReference& operator=(const NodeReference& other) requires (!std::is_const_v<T>) {
        assign(static_cast<Node<Dimension>>(other)); return *this;
    }
};

template <std::size_t Dimension, class T>
struct NodeView {
    static constexpr auto dimension = Dimension;
    using Entries = DimensionEntries<Dimension>;
    FieldView<typename Entries::Coordinate,T> coordinates;
    FieldView<typename Entries::Displacement,T> displacement;
    FieldView<typename Entries::Velocity,T> velocity;
    FieldView<typename Entries::Acceleration,T> acceleration;
    FieldView<typename Entries::Force,T> internal_force, external_force;
    ScalarView<T> mass;
    [[nodiscard]] std::size_t size() const noexcept { return mass.size(); }
    [[nodiscard]] typename Node<Dimension>::Vector current_coordinates(std::size_t row) const noexcept {
        typename Node<Dimension>::Vector result{};
        for (std::size_t d=0;d<Dimension;++d)
            result[d]=coordinates[static_cast<typename Entries::Coordinate>(d)][row]+
                      displacement[static_cast<typename Entries::Displacement>(d)][row];
        return result;
    }
    [[nodiscard]] NodeReference<Dimension,T> operator[](std::size_t row) const noexcept { return {*this,row}; }
    [[nodiscard]] NodeReference<Dimension,T> at(std::size_t row) const {
        if (row >= size()) throw std::out_of_range("Node row exceeds storage");
        return (*this)[row];
    }
    [[nodiscard]] NodeView<Dimension,const double> read_only() const noexcept {
        return {coordinates,displacement,velocity,acceleration,internal_force,external_force,mass};
    }
    struct Iterator {
        const NodeView* view; std::size_t row;
        auto operator*() const { return (*view)[row]; }
        Iterator& operator++() { ++row; return *this; }
        bool operator==(const Iterator&) const = default;
    };
    [[nodiscard]] Iterator begin() const { return {this,0}; }
    [[nodiscard]] Iterator end() const { return {this,size()}; }
};
template <std::size_t D, class T>
[[nodiscard]] auto read_only_nodes(NodeView<D,T> nodes) { return nodes.read_only(); }
template <class NodeType, std::size_t Extent>
[[nodiscard]] auto read_only_nodes(std::span<NodeType,Extent> nodes) {
    return std::span<const std::remove_const_t<NodeType>>(nodes.data(), nodes.size());
}
template <class NodeType, std::size_t Count>
[[nodiscard]] auto read_only_nodes(const std::array<NodeType,Count>& nodes) {
    return std::span<const NodeType>{nodes};
}
} // namespace wavecore
