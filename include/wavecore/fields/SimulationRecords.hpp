#pragma once

#include <cstdint>
#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldRow.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"

namespace wavecore {
// Explicit adapters keep formulation layouts out of generic numeric storage.
// Additional formulations register their own adapters rather than using AoS fallback.
template <class Element> class ElementGeometryFields;
template <> class ElementGeometryFields<Quad4> {
public:
    template <class T, class Flag> struct Reference {
        T& measure; T& characteristic_length; Flag& valid;
        operator Quad4::element_geometry_type() const { return {measure,characteristic_length,valid != 0}; }
        Reference& operator=(const Quad4::element_geometry_type& value) requires (!std::is_const_v<T>) {
            measure=value.measure; characteristic_length=value.characteristic_length; valid=value.valid; return *this;
        }
        Reference& operator=(const Reference& other) requires (!std::is_const_v<T>) {
            return *this=static_cast<Quad4::element_geometry_type>(other);
        }
    };
    ScalarStorage<> measure, characteristic_length;
    ScalarStorage<std::uint8_t> valid;
    void add() { measure.push_back(0.0); characteristic_length.push_back(0.0); valid.push_back(0); }
    auto at(std::size_t row) { return Reference<double,std::uint8_t>{measure.at(row),characteristic_length.at(row),valid.at(row)}; }
    auto at(std::size_t row) const { return Reference<const double,const std::uint8_t>{measure.at(row),characteristic_length.at(row),valid.at(row)}; }
};

template <class Element> class PointGeometryFields;
template <> class PointGeometryFields<Quad4> {
public:
    template <class T> struct Reference {
        MatrixRow<JacobianEntry2D,T,2,2> jacobian;
        MatrixRow<Quad4GradientEntry,T,2,4> gradients;
        T& jacobian_determinant;
        operator Quad4::geometry_point_type() const { return {jacobian,gradients,jacobian_determinant}; }
        Reference& operator=(const Reference& other) requires (!std::is_const_v<T>) {
            return *this=static_cast<Quad4::geometry_point_type>(other);
        }
        Reference& operator=(const Quad4::geometry_point_type& value) requires (!std::is_const_v<T>) {
            jacobian = value.jacobian; gradients = value.gradients;
            jacobian_determinant = value.jacobian_determinant; return *this;
        }
    };
    FieldStorage<JacobianEntry2D> jacobian;
    FieldStorage<Quad4GradientEntry> gradients;
    ScalarStorage<> determinant;
    template <class T> struct View {
        FieldView<JacobianEntry2D,T> jacobian;
        FieldView<Quad4GradientEntry,T> gradients;
        ScalarView<T> determinant;
        [[nodiscard]] Reference<T> at(std::size_t row) const {
            if (row >= determinant.size()) throw std::out_of_range("Geometry row exceeds storage");
            return {{jacobian,row},{gradients,row},determinant[row]};
        }
    };
    [[nodiscard]] View<double> view() noexcept { return {jacobian.view(),gradients.view(),determinant.view()}; }
    [[nodiscard]] View<const double> view() const noexcept { return {jacobian.view(),gradients.view(),determinant.view()}; }
    [[nodiscard]] std::size_t size() const { return determinant.size(); }
    void add() { jacobian.push_back({}); gradients.push_back({}); determinant.push_back(0.0); }
    auto at(std::size_t row) {
        return Reference<double>{{jacobian.view(),row},{gradients.view(),row},determinant.at(row)};
    }
    auto at(std::size_t row) const {
        return Reference<const double>{{jacobian.view(),row},{gradients.view(),row},determinant.at(row)};
    }
};

template <class Material> class MaterialStateFields;
template <> class MaterialStateFields<LinearElasticPlaneStrain> {
public:
    template <class T> struct Reference {
        SymmetricMatrixRow<StressEntryPlaneStrain,T,2> stress;
        T& stress_zz;
        operator LinearElasticPlaneStrain::state_type() const { return {stress,stress_zz}; }
        Reference& operator=(const Reference& other) requires (!std::is_const_v<T>) {
            return *this=static_cast<LinearElasticPlaneStrain::state_type>(other);
        }
        Reference& operator=(const LinearElasticPlaneStrain::state_type& value) requires (!std::is_const_v<T>) {
            stress = value.stress; stress_zz = value.stress_zz; return *this;
        }
    };
    FieldStorage<StressEntryPlaneStrain> stress;
    template <class T> struct View {
        FieldView<StressEntryPlaneStrain,T> stress;
        [[nodiscard]] Reference<T> at(std::size_t row) const {
            if (row >= stress.size()) throw std::out_of_range("Material state row exceeds storage");
            return {{stress,row},stress[StressEntryPlaneStrain::szz][row]};
        }
    };
    [[nodiscard]] View<double> view() noexcept { return {stress.view()}; }
    [[nodiscard]] View<const double> view() const noexcept { return {stress.view()}; }
    void add(const LinearElasticPlaneStrain::state_type& state) {
        stress.push_back({state.stress(0,0),state.stress(0,1),state.stress(1,1),state.stress_zz});
    }
    auto at(std::size_t row) {
        if (row >= stress.size()) throw std::out_of_range("Material state row exceeds storage");
        return Reference<double>{{stress.view(),row},stress[StressEntryPlaneStrain::szz][row]};
    }
    auto at(std::size_t row) const {
        if (row >= stress.size()) throw std::out_of_range("Material state row exceeds storage");
        return Reference<const double>{{stress.view(),row},stress[StressEntryPlaneStrain::szz][row]};
    }
};
} // namespace wavecore
