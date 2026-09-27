#ifndef WAVECORE_QUAD4_HPP
#define WAVECORE_QUAD4_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include <stdexcept>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/elements/PlaneElementProperties.hpp"
#include "wavecore/elements/QuadraturePoint.hpp"
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"

namespace wavecore {

class Quad4 {
public:
    using node_type = Node2D;
    using properties_type = PlaneElementProperties;
    static constexpr std::size_t dimension = 2;
    static constexpr std::size_t gauss_points = 1;
    static constexpr std::size_t nodes_per_element = 4;
    static constexpr std::size_t num_edges = 4;

    struct geometry_point {
        Matrix<double, dimension, dimension> jacobian{};
        Matrix<double, dimension, nodes_per_element> gradients{};
        double jacobian_determinant = 0.0;
    };
    using geometry_point_type = geometry_point;
    using geometry_state_type = std::array<geometry_point, gauss_points>;
    using nodal_velocity_type = Matrix<double, nodes_per_element, dimension>;

    [[nodiscard]] constexpr std::array<QuadraturePoint<dimension>, gauss_points>
    quadrature() const noexcept { return {{{{0.0, 0.0}, 4.0}}}; }

    void refresh_geometry(std::span<const node_type> nodes,
                          std::span<const std::size_t, nodes_per_element> connectivity,
                          geometry_state_type& geometry) const {
        Matrix<double, nodes_per_element, dimension> coordinates{};
        for (std::size_t inode = 0; inode < nodes_per_element; ++inode)
            for (std::size_t d = 0; d < dimension; ++d)
                coordinates(inode, d) = nodes[connectivity[inode]].coordinates()[d];

        const auto points = quadrature();
        for (std::size_t gp = 0; gp < gauss_points; ++gp) {
            auto& point = geometry[gp];
            point.jacobian = derivatives_in_parent_domain(points[gp].coordinates, coordinates);
            point.jacobian_determinant = determinant(point.jacobian);
            if (!std::isfinite(point.jacobian_determinant) || point.jacobian_determinant <= 0.0)
                throw std::domain_error("Quad4 geometry requires a finite positive Jacobian");
            point.gradients = inverse(point.jacobian) *
                              derivatives_shape_functions_parent(points[gp].coordinates);
        }
    }

    void gather_velocities(std::span<const node_type> nodes,
                           std::span<const std::size_t, nodes_per_element> connectivity,
                           nodal_velocity_type& velocities) const noexcept {
        for (std::size_t inode = 0; inode < nodes_per_element; ++inode)
            for (std::size_t d = 0; d < dimension; ++d)
                velocities(inode, d) = nodes[connectivity[inode]].velocity()[d];
    }

    [[nodiscard]] Matrix<double, dimension, dimension>
    strain_rate_tensor(const geometry_state_type& geometry,
                       const nodal_velocity_type& velocities,
                       std::size_t gp = 0) const {
        return symmetric(geometry.at(gp).gradients * velocities);
    }

    [[nodiscard]] std::array<Vector<double, dimension>, nodes_per_element>
    internal_force(ElementStressSpan<Quad4> stresses, const properties_type& properties,
                   const geometry_state_type& geometry) const {
        std::array<Vector<double, dimension>, nodes_per_element> forces{};
        const auto points = quadrature();
        for (std::size_t gp = 0; gp < gauss_points; ++gp) {
            const auto& point = geometry.at(gp);
            if (!std::isfinite(point.jacobian_determinant) || point.jacobian_determinant <= 0.0)
                throw std::domain_error("Quad4 internal force requires a finite positive Jacobian");
            const auto stress_gradients = stresses[gp] * point.gradients;
            const double weight = points[gp].weight * point.jacobian_determinant *
                                  properties.thickness();
            for (std::size_t node = 0; node < nodes_per_element; ++node)
                for (std::size_t d = 0; d < dimension; ++d)
                    forces[node](d) += stress_gradients(d, node) * weight;
        }
        return forces;
    }

private:
    [[nodiscard]] static Matrix<double, dimension, nodes_per_element>
    derivatives_shape_functions_parent(const Vector<double, dimension>& local) noexcept {
        const auto csi = local(0), eta = local(1);
        Matrix<double, dimension, nodes_per_element> d{};
        d(0, 0) = -0.25 * (1.0 - eta); d(0, 1) = 0.25 * (1.0 - eta);
        d(0, 2) = 0.25 * (1.0 + eta);  d(0, 3) = -0.25 * (1.0 + eta);
        d(1, 0) = -0.25 * (1.0 - csi); d(1, 1) = -0.25 * (1.0 + csi);
        d(1, 2) = 0.25 * (1.0 + csi);  d(1, 3) = 0.25 * (1.0 - csi);
        return d;
    }

    template <std::size_t Columns>
    [[nodiscard]] static Matrix<double, dimension, dimension>
    derivatives_in_parent_domain(const Vector<double, dimension>& local,
                                 const Matrix<double, Columns, dimension>& field) noexcept {
        return derivatives_shape_functions_parent(local) * field;
    }
};

static_assert(IElementConcept<Quad4>);

} // namespace wavecore

#endif
