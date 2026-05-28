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

TEST_CASE("Quad4 measure tests") {
    SUBCASE("unit square has measure one") {
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0: bottom-left
            wavecore::Node2D{{1.0, 0.0}}, // node 1: bottom-right
            wavecore::Node2D{{1.0, 1.0}}, // node 2: top-right
            wavecore::Node2D{{0.0, 1.0}}  // node 3: top-left
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(1.0));
    }

    SUBCASE("axis-aligned rectangle has measure width times height") {
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0
            wavecore::Node2D{{2.0, 0.0}}, // node 1
            wavecore::Node2D{{2.0, 3.0}}, // node 2
            wavecore::Node2D{{0.0, 3.0}}  // node 3
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(6.0));
    }

    SUBCASE("translated rectangle has unchanged measure") {
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{10.0, -4.0}}, // node 0
            wavecore::Node2D{{13.0, -4.0}}, // node 1
            wavecore::Node2D{{13.0,  1.0}}, // node 2
            wavecore::Node2D{{10.0,  1.0}}  // node 3
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(15.0));
    }

    SUBCASE("parallelogram has correct measure") {
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0
            wavecore::Node2D{{2.0, 0.0}}, // node 1
            wavecore::Node2D{{3.0, 1.0}}, // node 2
            wavecore::Node2D{{1.0, 1.0}}  // node 3
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(2.0));
    }

    SUBCASE("distorted convex quadrilateral has correct measure") {
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0
            wavecore::Node2D{{2.0, 0.0}}, // node 1
            wavecore::Node2D{{2.5, 1.0}}, // node 2
            wavecore::Node2D{{0.0, 1.0}}  // node 3
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(2.25));
    }

    SUBCASE("nontrivial connectivity has correct measure") {
        std::array<wavecore::Node2D, 8> nodes{
            wavecore::Node2D{{100.0, 100.0}}, // unused
            wavecore::Node2D{{0.0,   0.0}},   // bottom-left
            wavecore::Node2D{{42.0,  42.0}},  // unused
            wavecore::Node2D{{2.0,   0.0}},   // bottom-right
            wavecore::Node2D{{-1.0, -1.0}},   // unused
            wavecore::Node2D{{2.0,   3.0}},   // top-right
            wavecore::Node2D{{0.0,   3.0}},   // top-left
            wavecore::Node2D{{9.0,   9.0}}    // unused
        };

        wavecore::Quad4 element{};

        std::array<std::size_t, 4> connectivity{1, 3, 5, 6};

        element.gather(nodes, connectivity);

        CHECK(element.measure() == doctest::Approx(6.0));
    }
}
