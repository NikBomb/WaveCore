#ifndef WAVECORE_NODE_ARCHETYPE_HPP
#define WAVECORE_NODE_ARCHETYPE_HPP

#include <cstddef>
#include <span>
#include <utility>
#include <vector>

#include "wavecore/mesh/Node.hpp"

namespace wavecore {

template <std::size_t Dimension>
class NodeArchetype {
public:
    using node_type = Node<Dimension>;

    [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }

    [[nodiscard]] std::size_t add_node(node_type node) {
        const auto index = nodes_.size();
        nodes_.push_back(std::move(node));
        return index;
    }

    [[nodiscard]] node_type& node(std::size_t index) { return nodes_.at(index); }
    [[nodiscard]] const node_type& node(std::size_t index) const { return nodes_.at(index); }
    [[nodiscard]] std::span<node_type> values() noexcept { return nodes_; }
    [[nodiscard]] std::span<const node_type> values() const noexcept { return nodes_; }

private:
    std::vector<node_type> nodes_;
};

} // namespace wavecore

#endif
