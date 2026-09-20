#include <doctest/doctest.h>

#include "wavecore/elements/IElementConcept.hpp"
#include "wavecore/elements/Quad4.hpp"

namespace {

struct WrongPointCount : wavecore::Quad4 {
    static constexpr std::size_t gauss_points = 2;
};

struct MissingQuadrature : wavecore::Quad4 {
    void quadrature() const = delete;
};

static_assert(wavecore::IElementConcept<wavecore::Quad4>);
static_assert(!wavecore::IElementConcept<WrongPointCount>);
static_assert(!wavecore::IElementConcept<MissingQuadrature>);

} // namespace

TEST_CASE("Quad4 quadrature integrates constants and linear parent fields") {
    // Quadrature is available before gathering any nodal data.
    const wavecore::Quad4 element;
    const auto points = element.quadrature();
    REQUIRE(points.size() == 1);
    CHECK(points[0].coordinates(0) == 0.0);
    CHECK(points[0].coordinates(1) == 0.0);
    CHECK(points[0].weight == 4.0);

    double integral = 0.0;
    for (const auto& point : points) {
        const double field = 2.0 + 3.0 * point.coordinates(0)
                                 - 5.0 * point.coordinates(1);
        integral += field * point.weight;
    }
    CHECK(integral == doctest::Approx(8.0));
}

TEST_CASE("Quad4 quadrature integrates physical area on a skewed element") {
    std::array<wavecore::Node2D, 4> nodes{
        wavecore::Node2D{{0.0, 0.0}}, wavecore::Node2D{{2.0, 0.0}},
        wavecore::Node2D{{3.0, 3.0}}, wavecore::Node2D{{1.0, 3.0}}
    };
    const std::array<std::size_t, 4> connectivity{0, 1, 2, 3};
    wavecore::Quad4 element;
    element.gather(nodes, connectivity);

    double area = 0.0;
    for (const auto& point : element.quadrature()) {
        area += point.weight * element.jacobian_determinant(point.coordinates);
    }
    CHECK(area == doctest::Approx(6.0));
    CHECK(area == doctest::Approx(element.measure()));
}
