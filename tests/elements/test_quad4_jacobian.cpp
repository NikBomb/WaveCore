#include <array>
#include <doctest/doctest.h>
#include "wavecore/elements/Quad4.hpp"

TEST_CASE("Quad4 refreshes the physical Jacobian") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0, 0}}, wavecore::Node2D{{2, 0}},
        wavecore::Node2D{{3, 1}}, wavecore::Node2D{{1, 1}}};
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4::geometry_state_type geometry{};
    wavecore::Quad4{}.refresh_geometry(nodes, connectivity, geometry);
    CHECK(geometry[0].jacobian(0, 0) == doctest::Approx(1));
    CHECK(geometry[0].jacobian(0, 1) == doctest::Approx(0));
    CHECK(geometry[0].jacobian(1, 0) == doctest::Approx(.5));
    CHECK(geometry[0].jacobian(1, 1) == doctest::Approx(.5));
}
