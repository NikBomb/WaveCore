#ifndef WAVECORE_MESH_NODE_HPP
#define WAVECORE_MESH_NODE_HPP

#include <array>
#include <cstddef>

namespace wavecore {

template <std::size_t Dimension>
class Node {
    static_assert(
        Dimension == 2 || Dimension == 3,
        "WaveCore currently supports only 2D and 3D nodes"
    );

public:
    using Vector = std::array<double, Dimension>;

    Node() = default;

    explicit Node(const Vector& coordinates)
        : coordinates_{coordinates}
    {
    }

    [[nodiscard]] const Vector& coordinates() const noexcept {
        return coordinates_;
    }

    [[nodiscard]] Vector& displacement() noexcept {
        return displacement_;
    }

    [[nodiscard]] const Vector& displacement() const noexcept {
        return displacement_;
    }

    [[nodiscard]] Vector& velocity() noexcept {
        return velocity_;
    }

    [[nodiscard]] const Vector& velocity() const noexcept {
        return velocity_;
    }

    [[nodiscard]] Vector& acceleration() noexcept {
        return acceleration_;
    }

    [[nodiscard]] const Vector& acceleration() const noexcept {
        return acceleration_;
    }

    [[nodiscard]] double mass() const noexcept {
        return mass_;
    }

    void set_mass(const double mass) noexcept {
        mass_ = mass;
    }

    [[nodiscard]] Vector& internal_force() noexcept {
        return internal_force_;
    }

    [[nodiscard]] const Vector& internal_force() const noexcept {
        return internal_force_;
    }

    [[nodiscard]] Vector& external_force() noexcept {
        return external_force_;
    }

    [[nodiscard]] const Vector& external_force() const noexcept {
        return external_force_;
    }

private:
    Vector coordinates_{};
    Vector displacement_{};
    Vector velocity_{};
    Vector acceleration_{};

    double mass_ = 0.0;

    Vector internal_force_{};
    Vector external_force_{};
};

using Node2D = Node<2>;
using Node3D = Node<3>;

} // namespace wavecore

#endif // WAVECORE_MESH_NODE_HPP