#include "wavecore/mesh/Node.hpp"

#include <doctest/doctest.h>

TEST_CASE("2D node initializes fields to zero") {
    const wavecore::Node2D node{};

    CHECK(node.x.size() == 2);
    CHECK(node.u.size() == 2);
    CHECK(node.v.size() == 2);
    CHECK(node.a.size() == 2);
    CHECK(node.f_int.size() == 2);
    CHECK(node.f_ext.size() == 2);

    for (std::size_t d = 0; d < 2; ++d) {
        CHECK(node.x[d] == doctest::Approx(0.0));
        CHECK(node.u[d] == doctest::Approx(0.0));
        CHECK(node.v[d] == doctest::Approx(0.0));
        CHECK(node.a[d] == doctest::Approx(0.0));
        CHECK(node.f_int[d] == doctest::Approx(0.0));
        CHECK(node.f_ext[d] == doctest::Approx(0.0));
    }

    CHECK(node.mass == doctest::Approx(0.0));
}

TEST_CASE("3D node initializes fields to zero") {
    const wavecore::Node3D node{};

    CHECK(node.x.size() == 3);
    CHECK(node.u.size() == 3);
    CHECK(node.v.size() == 3);
    CHECK(node.a.size() == 3);
    CHECK(node.f_int.size() == 3);
    CHECK(node.f_ext.size() == 3);

    for (std::size_t d = 0; d < 3; ++d) {
        CHECK(node.x[d] == doctest::Approx(0.0));
        CHECK(node.u[d] == doctest::Approx(0.0));
        CHECK(node.v[d] == doctest::Approx(0.0));
        CHECK(node.a[d] == doctest::Approx(0.0));
        CHECK(node.f_int[d] == doctest::Approx(0.0));
        CHECK(node.f_ext[d] == doctest::Approx(0.0));
    }

    CHECK(node.mass == doctest::Approx(0.0));
}

TEST_CASE("2D node coordinates can be initialized") {
    wavecore::Node2D node{};

    node.x[0] = 1.0;
    node.x[1] = 2.0;

    CHECK(node.x[0] == doctest::Approx(1.0));
    CHECK(node.x[1] == doctest::Approx(2.0));
}

TEST_CASE("3D node coordinates can be initialized") {
    wavecore::Node3D node{};

    node.x[0] = 1.0;
    node.x[1] = 2.0;
    node.x[2] = 3.0;

    CHECK(node.x[0] == doctest::Approx(1.0));
    CHECK(node.x[1] == doctest::Approx(2.0));
    CHECK(node.x[2] == doctest::Approx(3.0));
}