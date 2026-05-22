#include "wavecore/elements/IElement.hpp"
#include "wavecore/elements/Quad4.hpp"

#include <doctest/doctest.h>

namespace {

class FakeElement {};

} // namespace

TEST_CASE("Quad4 conforms concept") {
    static_assert(wavecore::IElement<wavecore::Quad4>);
}

TEST_CASE("Fake element does not conform concept") {
    static_assert(!wavecore::IElement<FakeElement>);
}