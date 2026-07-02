#include "wavecore/elements/Quad4.hpp"
#include "wavecore/mesh/Node.hpp"
#include "wavecore/utils/Matrix.hpp"

#include <doctest/doctest.h>

#include <array>

TEST_CASE("Quad4 jacobian matrix tests") {
    SUBCASE("Unit square jacobian matrix") {
        // Create a unit square element with nodes at:
        // 0: (0,0), 1: (1,0), 2: (1,1), 3: (0,1)
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0: bottom-left
            wavecore::Node2D{{1.0, 0.0}}, // node 1: bottom-right
            wavecore::Node2D{{1.0, 1.0}}, // node 2: top-right
            wavecore::Node2D{{0.0, 1.0}}  // node 3: top-left
        };

        wavecore::Quad4 element{};
        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        // Test at the center point (0,0) - this is where the Gauss point is located
        wavecore::Vector<double, 2> local_coords;
        auto jacobian = element.jacobian_matrix(local_coords);

        // For a unit square, we need to compute the actual Jacobian matrix
        // The derivative matrix at (0,0) in parent domain:
        // dN1_dcsi = -0.25*(1-eta) = -0.25*(1-0) = -0.25
        // dN2_dcsi =  0.25*(1-eta) =  0.25*(1-0) =  0.25  
        // dN3_dcsi =  0.25*(1+eta) =  0.25*(1+0) =  0.25
        // dN4_dcsi = -0.25*(1+eta) = -0.25*(1+0) = -0.25
        // 
        // dN1_deta = -0.25*(1-csi) = -0.25*(1-0) = -0.25
        // dN2_deta = -0.25*(1+csi) = -0.25*(1+0) = -0.25
        // dN3_deta =  0.25*(1+csi) =  0.25*(1+0) =  0.25
        // dN4_deta =  0.25*(1-csi) =  0.25*(1-0) =  0.25
        //
        // Derivative matrix at (0,0):
        // [-0.25,  0.25,  0.25, -0.25]
        // [-0.25, -0.25,  0.25,  0.25]
        //
        // Coordinates matrix (unit square):
        // [0, 1, 1, 0]  (x coordinates)
        // [0, 0, 1, 1]  (y coordinates)
        //
        // Jacobian = derivative_matrix * coordinates_matrix
        // Jacobian[0][0] = (-0.25)*0 + (0.25)*1 + (0.25)*1 + (-0.25)*0 = 0.5
        // Jacobian[0][1] = (-0.25)*0 + (0.25)*0 + (0.25)*1 + (-0.25)*1 = 0
        // Jacobian[1][0] = (-0.25)*0 + (-0.25)*1 + (0.25)*1 + (0.25)*0 = 0
        // Jacobian[1][1] = (-0.25)*0 + (-0.25)*0 + (0.25)*1 + (0.25)*1 = 0.5
        //
        // So the Jacobian should be:
        // [0.5, 0]
        // [0, 0.5]
        CHECK(jacobian(0,0) == doctest::Approx(0.5));
        CHECK(jacobian(0,1) == doctest::Approx(0.0));
        CHECK(jacobian(1,0) == doctest::Approx(0.0));
        CHECK(jacobian(1,1) == doctest::Approx(0.5));
    }

    SUBCASE("Scaled square jacobian matrix") {
        // Create a scaled square element with nodes at:
        // 0: (0,0), 1: (2,0), 2: (2,3), 3: (0,3)
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0
            wavecore::Node2D{{2.0, 0.0}}, // node 1
            wavecore::Node2D{{2.0, 3.0}}, // node 2
            wavecore::Node2D{{0.0, 3.0}}  // node 3
        };

        wavecore::Quad4 element{};
        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        // Test at the center point (0,0)
        wavecore::Vector<double, 2> local_coords;
        auto jacobian = element.jacobian_matrix(local_coords);

        // For this rectangle, we need to compute the actual Jacobian matrix
        // The derivative matrix at (0,0) in parent domain:
        // dN1_dcsi = -0.25*(1-eta) = -0.25*(1-0) = -0.25
        // dN2_dcsi =  0.25*(1-eta) =  0.25*(1-0) =  0.25  
        // dN3_dcsi =  0.25*(1+eta) =  0.25*(1+0) =  0.25
        // dN4_dcsi = -0.25*(1+eta) = -0.25*(1+0) = -0.25
        // 
        // dN1_deta = -0.25*(1-csi) = -0.25*(1-0) = -0.25
        // dN2_deta = -0.25*(1+csi) = -0.25*(1+0) = -0.25
        // dN3_deta =  0.25*(1+csi) =  0.25*(1+0) =  0.25
        // dN4_deta =  0.25*(1-csi) =  0.25*(1-0) =  0.25
        //
        // Derivative matrix at (0,0):
        // [-0.25,  0.25,  0.25, -0.25]
        // [-0.25, -0.25,  0.25,  0.25]
        //
        // Coordinates matrix (rectangle with nodes at (0,0), (2,0), (2,3), (0,3)):
        // [0, 2, 2, 0]  (x coordinates)
        // [0, 0, 3, 3]  (y coordinates)
        //
        // Jacobian = derivative_matrix * coordinates_matrix
        // Jacobian[0][0] = (-0.25)*0 + (0.25)*2 + (0.25)*2 + (-0.25)*0 = 1
        // Jacobian[0][1] = (-0.25)*0 + (0.25)*0 + (0.25)*3 + (-0.25)*3 = 0
        // Jacobian[1][0] = (-0.25)*0 + (-0.25)*2 + (0.25)*2 + (0.25)*0 = 0
        // Jacobian[1][1] = (-0.25)*0 + (-0.25)*0 + (0.25)*3 + (0.25)*3 = 1.5
        //
        // So the Jacobian should be:
        // [1, 0]
        // [0, 1.5]
        CHECK(jacobian(0,0) == doctest::Approx(1.0));
        CHECK(jacobian(0,1) == doctest::Approx(0.0));
        CHECK(jacobian(1,0) == doctest::Approx(0.0));
        CHECK(jacobian(1,1) == doctest::Approx(1.5));
    }

    SUBCASE("Parallelogram jacobian matrix") {
        // Create a parallelogram element with nodes at:
        // 0: (0,0), 1: (2,0), 2: (3,1), 3: (1,1)
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0
            wavecore::Node2D{{2.0, 0.0}}, // node 1
            wavecore::Node2D{{3.0, 1.0}}, // node 2
            wavecore::Node2D{{1.0, 1.0}}  // node 3
        };

        wavecore::Quad4 element{};
        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        // Test at the center point (0,0)
        wavecore::Vector<double, 2> local_coords;
        auto jacobian = element.jacobian_matrix(local_coords);

         // For this parallelogram, we need to compute the Jacobian matrix manually
         // The derivative matrix at (0,0) in parent domain:
         // dN1_dcsi = -0.25*(1-eta) = -0.25*(1-0) = -0.25
         // dN2_dcsi =  0.25*(1-eta) =  0.25*(1-0) =  0.25  
         // dN3_dcsi =  0.25*(1+eta) =  0.25*(1+0) =  0.25
         // dN4_dcsi = -0.25*(1+eta) = -0.25*(1+0) = -0.25
         // 
         // dN1_deta = -0.25*(1-csi) = -0.25*(1-0) = -0.25
         // dN2_deta = -0.25*(1+csi) = -0.25*(1+0) = -0.25
         // dN3_deta =  0.25*(1+csi) =  0.25*(1+0) =  0.25
         // dN4_deta =  0.25*(1-csi) =  0.25*(1-0) =  0.25
         //
         // Derivative matrix at (0,0):
         // [-0.25,  0.25,  0.25, -0.25]
         // [-0.25, -0.25,  0.25,  0.25]
         //
         // Coordinates matrix (parallelogram with nodes at (0,0), (2,0), (3,1), (1,1)):
         // [0, 2, 3, 1]  (x coordinates)
         // [0, 0, 1, 1]  (y coordinates)
         //
         // Jacobian = derivative_matrix * coordinates_matrix
         // Jacobian[0][0] = (-0.25)*0 + (0.25)*2 + (0.25)*3 + (-0.25)*1 = 1
         // Jacobian[0][1] = (-0.25)*0 + (0.25)*0 + (0.25)*1 + (-0.25)*1 = 0
         // Jacobian[1][0] = (-0.25)*0 + (-0.25)*2 + (0.25)*3 + (0.25)*1 = 0.5
         // Jacobian[1][1] = (-0.25)*0 + (-0.25)*0 + (0.25)*1 + (0.25)*1 = 0.5
         //
         // So the Jacobian should be:
         // [1, 0]
         // [0.5, 0.5]
         CHECK(jacobian(0,0) == doctest::Approx(1.0));
         CHECK(jacobian(0,1) == doctest::Approx(0.0));
         CHECK(jacobian(1,0) == doctest::Approx(0.5));
         CHECK(jacobian(1,1) == doctest::Approx(0.5));
    }

    SUBCASE("Jacobian determinant calculation") {
        // Create a unit square element
        std::array<wavecore::Node2D, 4> nodes{
            wavecore::Node2D{{0.0, 0.0}}, // node 0: bottom-left
            wavecore::Node2D{{1.0, 0.0}}, // node 1: bottom-right
            wavecore::Node2D{{1.0, 1.0}}, // node 2: top-right
            wavecore::Node2D{{0.0, 1.0}}  // node 3: top-left
        };

        wavecore::Quad4 element{};
        std::array<std::size_t, 4> connectivity{0, 1, 2, 3};

        element.gather(nodes, connectivity);

        // Test at the center point (0,0)
        wavecore::Vector<double, 2> local_coords;
        auto jacobian = element.jacobian_matrix(local_coords);
        
         // Determinant of unit square Jacobian matrix should be 1
         double det = wavecore::determinant(jacobian);
         // For our calculated Jacobian [0.5, 0; 0, 0.5], determinant = 0.5 * 0.5 - 0 * 0 = 0.25
         CHECK(det == doctest::Approx(0.25));
    }
}