#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/elements/Quad4.hpp"

#include <doctest/doctest.h>

namespace {
struct FakeElement {};
}

TEST_CASE("Element concept describes stateless kernel operations") {
    static_assert(wavecore::IElementConcept<wavecore::Quad4>);
    static_assert(!wavecore::IElementConcept<FakeElement>);
    CHECK(wavecore::IElementConcept<wavecore::Quad4>);
}
