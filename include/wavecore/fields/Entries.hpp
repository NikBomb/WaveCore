#pragma once

#include <cstddef>

namespace wavecore {
// Entries are contiguous and zero based. Never interchange enums between fields.
enum class CoordinateEntry2D { x, y, count };
enum class CoordinateEntry3D { x, y, z, count };
enum class DisplacementEntry2D { ux, uy, count };
enum class DisplacementEntry3D { ux, uy, uz, count };
enum class VelocityEntry2D { vx, vy, count };
enum class VelocityEntry3D { vx, vy, vz, count };
enum class AccelerationEntry2D { ax, ay, count };
enum class AccelerationEntry3D { ax, ay, az, count };
enum class ForceEntry2D { fx, fy, count };
enum class ForceEntry3D { fx, fy, fz, count };
// Symmetric tensors: reversed shear names alias the same entry. Shear remains
// tensor shear (half the engineering shear). Plane strain also retains szz.
enum class StressEntry2D { sxx, sxy, syy, count, syx = sxy };
enum class StressEntryPlaneStrain { sxx, sxy, syy, szz, count, syx = sxy };
enum class StressEntry3D { sxx, sxy, sxz, syy, syz, szz, count, syx = sxy, szx = sxz, szy = syz };
enum class StrainRateEntry2D { exx, exy, eyy, count, eyx = exy };
enum class StrainRateEntry3D { exx, exy, exz, eyy, eyz, ezz, count, eyx = exy, ezx = exz, ezy = eyz };
enum class JacobianEntry2D { jxx, jxy, jyx, jyy, count };
enum class Quad4GradientEntry { dx_n0, dx_n1, dx_n2, dx_n3, dy_n0, dy_n1, dy_n2, dy_n3, count };
enum class Quad4NodeEntry { n0, n1, n2, n3, count };
enum class Quad4PointEntry { g0, count };
enum class ConstraintEntry2D { x, y, count };
enum class ConstraintEntry3D { x, y, z, count };

class Quad4;
template <class Element> struct ElementRelationEntries;
template <> struct ElementRelationEntries<Quad4> {
    using Node = Quad4NodeEntry;
    using Point = Quad4PointEntry;
};

template <std::size_t Dimension> struct DimensionEntries;
template <> struct DimensionEntries<2> {
    using Coordinate = CoordinateEntry2D;
    using Displacement = DisplacementEntry2D;
    using Velocity = VelocityEntry2D;
    using Acceleration = AccelerationEntry2D;
    using Force = ForceEntry2D;
    using Stress = StressEntry2D;
    using StrainRate = StrainRateEntry2D;
    using Constraint = ConstraintEntry2D;
};
template <> struct DimensionEntries<3> {
    using Coordinate = CoordinateEntry3D;
    using Displacement = DisplacementEntry3D;
    using Velocity = VelocityEntry3D;
    using Acceleration = AccelerationEntry3D;
    using Force = ForceEntry3D;
    using Stress = StressEntry3D;
    using StrainRate = StrainRateEntry3D;
    using Constraint = ConstraintEntry3D;
};
} // namespace wavecore
