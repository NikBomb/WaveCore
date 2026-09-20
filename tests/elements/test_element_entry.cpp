#include <limits>

#include <doctest/doctest.h>

#include "wavecore/elements/ElementEntry.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"

namespace {

// Storage-only fixture; does not implement a new quadrature formulation.
struct FourPointElement : wavecore::Quad4 {
    static constexpr std::size_t gauss_points = 4;
};

template <std::size_t Dimension>
struct InitialHistoryMaterial {
    static constexpr std::size_t dimension = Dimension;
    using tensor_type = wavecore::Matrix<double, Dimension, Dimension>;

    struct state_type {
        double history = 0.0;
        tensor_type stress{};
    };

    mutable std::size_t initialization_calls = 0;
    state_type initial_state() const {
        ++initialization_calls;
        return state_type{7.0};
    }
    double density() const { return 1.0; }
    void update(state_type&, const tensor_type&, double) const {}
    const tensor_type& stress(const state_type& state) const { return state.stress; }
};

template <class Element, class Material>
concept CanFormEntry = requires { typename wavecore::ElementEntry<Element, Material>; };

static_assert(CanFormEntry<wavecore::Quad4, InitialHistoryMaterial<2>>);
static_assert(!CanFormEntry<wavecore::Quad4, InitialHistoryMaterial<3>>);

} // namespace

TEST_CASE("Plane element properties require finite positive thickness") {
    CHECK(wavecore::PlaneElementProperties{0.25}.thickness() == 0.25);
    for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}) {
        CHECK_THROWS_AS(wavecore::PlaneElementProperties{invalid}, std::invalid_argument);
    }
}

TEST_CASE("Element entry owns connectivity and initializes elastic history") {
    const wavecore::LinearElasticPlaneStrain material{100.0, 0.25, 1.0};
    std::array<std::size_t, 4> connectivity{1, 3, 5, 6};
    wavecore::ElementEntry<wavecore::Quad4, wavecore::LinearElasticPlaneStrain>
        entry{connectivity, wavecore::PlaneElementProperties{0.5}, material};
    connectivity[0] = 99;
    CHECK(entry.connectivity[0] == 1);
    CHECK(entry.properties.thickness() == 0.5);
    REQUIRE(entry.states.size() == 1);
    CHECK(entry.states[0].stress(0, 0) == 0.0);
    CHECK(entry.states[0].stress_zz == 0.0);
}

TEST_CASE("Every integration point receives independent material-defined history") {
    using Material = InitialHistoryMaterial<2>;
    using Entry = wavecore::ElementEntry<FourPointElement, Material>;
    const Material material;
    Entry first{{0, 1, 2, 3}, wavecore::PlaneElementProperties{1.0}, material};
    Entry second{{4, 5, 6, 7}, wavecore::PlaneElementProperties{1.0}, material};
    CHECK(material.initialization_calls == 8);
    for (const auto& state : first.states) {
        CHECK(state.history == 7.0);
    }
    first.states[0].history = 42.0;
    CHECK(first.states[1].history == 7.0);
    CHECK(second.states[0].history == 7.0);
}
