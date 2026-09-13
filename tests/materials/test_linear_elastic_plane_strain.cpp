#include "wavecore/materials/LinearElasticPlaneStrain.hpp"

#include <doctest/doctest.h>
#include <limits>

namespace {
using Material = wavecore::LinearElasticPlaneStrain;
static_assert(wavecore::IMaterialConcept<Material>);
} // namespace

TEST_CASE("Plane strain material starts stress free") {
    const Material material(1000.0, 0.25, 2500.0);
    const auto state = material.initial_state();
    CHECK(material.density() == 2500.0);
    CHECK(material.stress(state) == Material::tensor_type{});
    CHECK(state.stress_zz == 0.0);
    CHECK(&material.stress(state) == &state.stress);
}

TEST_CASE("Plane strain uniaxial strain produces constrained transverse stresses") {
    // E = 1000, nu = 0.25: constrained axial modulus = 1200, coupling = 400.
    const Material material(1000.0, 0.25, 2500.0);
    auto state = material.initial_state();
    const Material::tensor_type rate{{0.02, 0.0}, {0.0, 0.0}};
    material.update(state, rate, 0.5);
    const auto& stress = material.stress(state);
    CHECK(stress(0, 0) == doctest::Approx(12.0));
    CHECK(stress(1, 1) == doctest::Approx(4.0));
    CHECK(stress(0, 1) == 0.0);
    CHECK(stress(1, 0) == 0.0);
    CHECK(state.stress_zz == doctest::Approx(4.0));
}

TEST_CASE("Plane strain shear uses tensor shear and preserves symmetry") {
    const Material material(1000.0, 0.25, 2500.0);
    auto state = material.initial_state();
    const Material::tensor_type rate{{0.0, 0.03}, {0.03, 0.0}};
    material.update(state, rate, 0.5);
    CHECK(material.stress(state)(0, 1) == doctest::Approx(12.0));
    CHECK(material.stress(state)(1, 0) == doctest::Approx(12.0));
    CHECK(material.stress(state)(0, 0) == 0.0);
    CHECK(material.stress(state)(1, 1) == 0.0);
    CHECK(state.stress_zz == 0.0);
}

TEST_CASE("Plane strain updates accumulate and reverse independently per state") {
    const Material material(1000.0, 0.25, 2500.0);
    auto state = material.initial_state();
    auto other = material.initial_state();
    const Material::tensor_type rate{{0.02, 0.03}, {0.03, -0.01}};
    material.update(state, rate, 0.25);
    material.update(state, rate, 0.75);
    material.update(other, rate, 1.0);
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            CHECK(material.stress(state)(i, j) ==
                  doctest::Approx(material.stress(other)(i, j)));
        }
    }
    CHECK(state.stress_zz == doctest::Approx(other.stress_zz));

    const auto before = state;
    material.update(state, rate, 0.0);
    material.update(state, Material::tensor_type{}, 1.0);
    CHECK(state.stress == before.stress);
    CHECK(state.stress_zz == before.stress_zz);

    const Material::tensor_type reverse{{-0.02, -0.03}, {-0.03, 0.01}};
    material.update(state, reverse, 1.0);
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            CHECK(material.stress(state)(i, j) == doctest::Approx(0.0));
        }
    }
    CHECK(state.stress_zz == doctest::Approx(0.0));
    CHECK(other.stress == before.stress);
    CHECK(other.stress_zz == before.stress_zz);
}

TEST_CASE("Plane strain material rejects invalid parameters and time steps") {
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (const double value : {0.0, -1.0, inf, nan}) {
        CHECK_THROWS_AS(Material(value, 0.25, 2500.0), std::invalid_argument);
        CHECK_THROWS_AS(Material(1000.0, 0.25, value), std::invalid_argument);
    }
    for (const double value : {-1.0, -2.0, 0.5, 1.0, inf, nan}) {
        CHECK_THROWS_AS(Material(1000.0, value, 2500.0), std::invalid_argument);
    }
    CHECK_NOTHROW(Material(1000.0, 0.0, 2500.0));
    CHECK_NOTHROW(Material(1000.0, -0.25, 2500.0));

    const Material material(1000.0, 0.25, 2500.0);
    auto state = material.initial_state();
    const Material::tensor_type rate{{0.02, 0.03}, {0.03, -0.01}};
    material.update(state, rate, 0.5);
    const auto before = state;
    for (const double dt : {-1.0, inf, nan}) {
        CHECK_THROWS_AS(material.update(state, rate, dt), std::invalid_argument);
        CHECK(state.stress == before.stress);
        CHECK(state.stress_zz == before.stress_zz);
    }
}
