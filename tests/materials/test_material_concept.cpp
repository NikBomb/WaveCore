// Include first and twice to verify self-containment and the include guard.
#include "wavecore/materials/IMaterialConcept.hpp"
#include "wavecore/materials/IMaterialConcept.hpp"

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/elements/Quad4.hpp"

#include <doctest/doctest.h>

namespace {

// Only declarations are needed to exercise the compile-time interface.
template <std::size_t Dimension>
struct Material {
    static constexpr std::size_t dimension = Dimension;
    using tensor_type = wavecore::MaterialTensor<Material>;
    struct state_type {
        tensor_type stress;
    };

    double density() const;
    state_type initial_state() const;
    void update(state_type&, const tensor_type&, double) const;
    const tensor_type& stress(const state_type&) const;
};

struct MissingTraits {};

struct MissingState {
    static constexpr std::size_t dimension = 2;
    using tensor_type = wavecore::Matrix<double, 2, 2>;
};

struct MissingTensor {
    static constexpr std::size_t dimension = 2;
    struct state_type {};
};

struct WrongTensor : Material<2> {
    using tensor_type = wavecore::Matrix<double, 3, 3>;
};

struct MutableDensity : Material<2> {
    double density();
};

struct WrongDensity : Material<2> {
    float density() const;
};

struct WrongInitialState : Material<2> {
    int initial_state() const;
};

struct MutableMaterialUpdate : Material<2> {
    void update(state_type&, const tensor_type&, double);
};

struct MutableStrainRate : Material<2> {
    void update(state_type&, tensor_type&, double) const;
};

struct WrongUpdateResult : Material<2> {
    bool update(state_type&, const tensor_type&, double) const;
};

struct MutableStressState : Material<2> {
    const tensor_type& stress(state_type&) const;
};

struct StressByValue : Material<2> {
    tensor_type stress(const state_type&) const;
};

struct MutableStressReference : Material<2> {
    tensor_type& stress(const state_type&) const;
};

} // namespace

TEST_CASE("Material concept accepts compatible 2D and 3D interfaces") {
    static_assert(wavecore::IMaterialConcept<Material<2>>);
    static_assert(wavecore::IMaterialConcept<Material<3>>);
    static_assert(std::same_as<wavecore::MaterialState<Material<2>>,
                               Material<2>::state_type>);
    static_assert(std::same_as<wavecore::MaterialTensor<Material<2>>,
                               wavecore::ElementMatrix<wavecore::Quad4>>);
}

TEST_CASE("Material concept rejects invalid traits") {
    static_assert(!wavecore::IMaterialConcept<MissingTraits>);
    static_assert(!wavecore::IMaterialConcept<MissingState>);
    static_assert(!wavecore::IMaterialConcept<MissingTensor>);
    static_assert(!wavecore::IMaterialConcept<Material<1>>);
    static_assert(!wavecore::IMaterialConcept<Material<4>>);
    static_assert(!wavecore::IMaterialConcept<WrongTensor>);
}

TEST_CASE("Material concept enforces operation types and const access") {
    static_assert(!wavecore::IMaterialConcept<MutableDensity>);
    static_assert(!wavecore::IMaterialConcept<WrongDensity>);
    static_assert(!wavecore::IMaterialConcept<WrongInitialState>);
    static_assert(!wavecore::IMaterialConcept<MutableMaterialUpdate>);
    static_assert(!wavecore::IMaterialConcept<MutableStrainRate>);
    static_assert(!wavecore::IMaterialConcept<WrongUpdateResult>);
    static_assert(!wavecore::IMaterialConcept<MutableStressState>);
    static_assert(!wavecore::IMaterialConcept<StressByValue>);
    static_assert(!wavecore::IMaterialConcept<MutableStressReference>);
}
