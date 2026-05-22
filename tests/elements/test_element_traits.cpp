#include "wavecore/elements/ElementTraits.hpp"
#include "wavecore/elements/Quad4.hpp"

#include <doctest/doctest.h>

namespace {

class FakeElement {};

} // namespace

TEST_CASE("Quad4 has defined element traits") {
    static_assert(wavecore::ElementTraitsDefined<wavecore::Quad4>);

    CHECK(wavecore::ElementTraits<wavecore::Quad4>::is_specialized);
    CHECK(wavecore::ElementTraits<wavecore::Quad4>::dimension == 2);
    CHECK(wavecore::ElementTraits<wavecore::Quad4>::nodes_per_element == 4);
    CHECK(wavecore::ElementTraits<wavecore::Quad4>::gauss_points == 1);
}

TEST_CASE("FakeElement does not have defined element traits") {
    static_assert(!wavecore::ElementTraitsDefined<FakeElement>);

    CHECK_FALSE(wavecore::ElementTraits<FakeElement>::is_specialized);
    CHECK(wavecore::ElementTraits<FakeElement>::dimension == 0);
    CHECK(wavecore::ElementTraits<FakeElement>::nodes_per_element == 0);
    CHECK(wavecore::ElementTraits<FakeElement>::gauss_points == 0);
}