#pragma once
#include "wavecore/mesh/NodeView.hpp"
namespace wavecore {
template <std::size_t Dimension>
class NodeArchetype {
public:
    using node_type = Node<Dimension>;
    using Entries = DimensionEntries<Dimension>;
    [[nodiscard]] std::size_t size() const noexcept { return mass_.size(); }
    [[nodiscard]] std::size_t add_node(const node_type& node) {
        const auto row = size();
        try {
            coordinates_.push_back(node.coordinates()); displacement_.push_back(node.displacement());
            velocity_.push_back(node.velocity()); acceleration_.push_back(node.acceleration());
            internal_force_.push_back(node.internal_force()); external_force_.push_back(node.external_force());
            mass_.push_back(node.mass());
        } catch (...) {
            coordinates_.resize(row); displacement_.resize(row); velocity_.resize(row);
            acceleration_.resize(row); internal_force_.resize(row); external_force_.resize(row);
            mass_.resize(row); throw;
        }
        return row;
    }
    [[nodiscard]] auto node(std::size_t row) { return values().at(row); }
    [[nodiscard]] auto node(std::size_t row) const { return values().at(row); }
    [[nodiscard]] NodeView<Dimension> values() noexcept {
        return {coordinates_.view(),displacement_.view(),velocity_.view(),acceleration_.view(),
                internal_force_.view(),external_force_.view(),mass_.view()};
    }
    [[nodiscard]] NodeView<Dimension,const double> values() const noexcept {
        return {coordinates_.view(),displacement_.view(),velocity_.view(),acceleration_.view(),
                internal_force_.view(),external_force_.view(),mass_.view()};
    }
private:
    FieldStorage<typename Entries::Coordinate> coordinates_;
    FieldStorage<typename Entries::Displacement> displacement_;
    FieldStorage<typename Entries::Velocity> velocity_;
    FieldStorage<typename Entries::Acceleration> acceleration_;
    FieldStorage<typename Entries::Force> internal_force_, external_force_;
    ScalarStorage<> mass_;
};
} // namespace wavecore
