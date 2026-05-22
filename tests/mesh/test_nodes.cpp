#include "wavecore/mesh/Node.hpp"

#include <doctest/doctest.h>

TEST_CASE("2D node initializes fields to zero") {
    const wavecore::Node2D node{};

    CHECK(node.coordinates().size() == 2);
    CHECK(node.displacement().size() == 2);
    CHECK(node.velocity().size() == 2);
    CHECK(node.acceleration().size() == 2);
    CHECK(node.internal_force().size() == 2);
    CHECK(node.external_force().size() == 2);

    for (std::size_t d = 0; d < 2; ++d) {
        CHECK(node.coordinates()[d] == doctest::Approx(0.0));
        CHECK(node.displacement()[d] == doctest::Approx(0.0));
        CHECK(node.velocity()[d] == doctest::Approx(0.0));
        CHECK(node.acceleration()[d] == doctest::Approx(0.0));
        CHECK(node.internal_force()[d] == doctest::Approx(0.0));
        CHECK(node.external_force()[d] == doctest::Approx(0.0));
    }

    CHECK(node.mass() == doctest::Approx(0.0));
}

TEST_CASE("2D node coordinates can be initialized") {
    const wavecore::Node2D node{{1.0, 2.0}};

    CHECK(node.coordinates()[0] == doctest::Approx(1.0));
    CHECK(node.coordinates()[1] == doctest::Approx(2.0));
}

TEST_CASE("2D node mutable fields can be updated through accessors") {
    wavecore::Node2D node{{1.0, 2.0}};

    node.displacement()[0] = 0.1;
    node.displacement()[1] = 0.2;

    node.velocity()[0] = 3.0;
    node.velocity()[1] = 4.0;

    node.set_mass(10.0);

    CHECK(node.displacement()[0] == doctest::Approx(0.1));
    CHECK(node.displacement()[1] == doctest::Approx(0.2));

    CHECK(node.velocity()[0] == doctest::Approx(3.0));
    CHECK(node.velocity()[1] == doctest::Approx(4.0));

    CHECK(node.mass() == doctest::Approx(10.0));
}

TEST_CASE("3D node coordinates can be initialized") {
    const wavecore::Node3D node{{1.0, 2.0, 3.0}};

    CHECK(node.coordinates()[0] == doctest::Approx(1.0));
    CHECK(node.coordinates()[1] == doctest::Approx(2.0));
    CHECK(node.coordinates()[2] == doctest::Approx(3.0));
}