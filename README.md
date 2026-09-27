# WaveCore

WaveCore is an experimental explicit finite-element code for elastodynamics,
wave propagation, and future fracture-mechanics research.

The repository currently contains a tested finite-element core and the first
explicit dynamics systems, including reusable simulation orchestration and
scheduled output callbacks. A mesh loader is not yet implemented.

## Architecture: ECS-style archetypes and relations

WaveCore uses a typed, data-oriented ECS model. Archetypes own components;
relations own indexes between entities; systems join and update the data.

```text
NodeArchetype
ElementArchetype<Element>
MaterialArchetype<Material>
GaussPointArchetype<Element, Material>

ElementNodeRelation
ElementGaussPointRelation
GaussPointMaterialRelation
```

The archetypes have separate responsibilities:

- `NodeArchetype` owns nodal components such as coordinates, velocity, and
  forces.
- `ElementArchetype<Element>` owns element-level components such as
  properties.
- `MaterialArchetype<Material>` owns shared constitutive definitions and
  parameters.
- `GaussPointArchetype<Element, Material>` owns integration-point geometry
  and material history, including stress.

Connectivity is not embedded in the element archetype. It is relational data:

```text
ElementNodeRelation
    element_id → node_ids[local_node]

ElementGaussPointRelation
    element_id → gauss_point_ids[local_gauss_point]

GaussPointMaterialRelation
    gauss_point_id → material_id
```

This keeps archetypes as component stores and lets systems join them without
creating ownership dependencies between archetypes.

## Stateless formulations

`Quad4` is a reusable element formulation and kernel. It owns no per-element
coordinates, connectivity, geometry, velocity, or material history. It
operates on caller-owned components:

```cpp
element.refresh_geometry(nodes, connectivity, geometry);
element.gather_velocities(nodes, connectivity, velocities);
auto strain_rate = element.strain_rate_tensor(geometry, velocities);
auto forces = element.internal_force(stresses, properties, geometry);
```

Material types similarly define constitutive behavior and state layout without
owning the states. A new element or material implements its concept; the
archetype and relation infrastructure remains generic.

## Systems

Systems perform the joins and update components. For example:

```text
GeometrySystem
    NodeArchetype + ElementArchetype + ElementNodeRelation
    → GaussPoint geometry components

MaterialUpdateSystem
    GaussPoint components + MaterialArchetype
    → GaussPoint material-state components

ForceAssemblySystem
    GaussPoint stress/geometry + ElementNodeRelation
    → nodal internal forces
```

An archetype may be read or updated by multiple systems. The archetype does
not know which systems use it.

The stress and constitutive history belong to Gauss-point entities, not shared
material definitions. One material definition may therefore serve many
Gauss points while every Gauss point retains independent history.

## Current components

- `Node<Dimension>` provides two- and three-dimensional nodal value types.
- `Quad4` implements a two-dimensional four-node quadrilateral with one centre
  Gauss point.
- `LinearElasticPlaneStrain` implements isotropic small-strain plane-strain
  elasticity.
- `ElementArchetype`, `MaterialArchetype`, and `GaussPointArchetype` provide
  typed component storage.
- Relation classes provide external element/node, element/Gauss-point, and
  Gauss-point/material indexes.
- `scatter_force` accumulates local forces into shared nodes.

## Repository layout

```text
include/wavecore/archetypes/  Typed component stores
include/wavecore/relations/   External relationship indexes
include/wavecore/elements/    Element formulations and properties
include/wavecore/materials/   Material concepts and constitutive models
include/wavecore/mesh/        Nodes and force scattering
tests/                        Component and numerical tests
apps/                         Placeholder executable
```

See [architecture.md](architecture.md) for the detailed ownership and system
model.

## Building and testing

The project uses CMake and requires a C++23 compiler.

```bash
cmake --preset gcc14-debug
cmake --build --preset gcc14-debug
ctest --preset gcc14-debug --output-on-failure
```

The Chiappa bulk-wave comparison can be generated with:

```bash
LSAN_OPTIONS=detect_leaks=0 build/gcc14-debug/apps/chiappa_bulk_wave \
    artifacts/chiappa_bulk/chiappa_snapshot.csv 160
MPLCONFIGDIR=/tmp/wavecore-mpl python3 scripts/plot_chiappa_snapshot.py \
    artifacts/chiappa_bulk/chiappa_snapshot.csv.71us.csv \
    artifacts/chiappa_bulk/chiappa_snapshot.png
```

The default mesh is 160 x 160; pass 320 for the refined comparison. Each run
saves the domain at 71 microseconds, a point history through 470 microseconds,
the final domain, and the direct nodal initial velocities. See
[the comparison instructions](artifacts/chiappa_bulk/NODAL_COMPARISON.md)
for matched-time domain and history plots for both meshes.

The one-point `Quad4` run is a diagnostic comparison.
It intentionally has no hourglass control and should not yet be treated as a
production-accuracy result.

The tests cover concept conformance, material initialization, stateless
element kernels, geometry and Jacobian calculations, force integration,
thickness scaling, shared-node scattering, archetype storage, and relation
indexes.

## Status and next steps

The separate archetypes, external relations, stateless element kernel,
generic geometry/material/force systems, lumped-mass leapfrog systems, and the
Chiappa analytical bulk-wave benchmark are implemented.
`ExplicitDynamicsSystem::run` handles initialization and time integration;
the application supplies the mesh/material data, IC, BC, and final time.
Optional policies set a timestep cap and output schedule. The system triggers
output events; application callbacks decide what and how to write.
Current BC support is limited to stationary homogeneous constraints.
Next steps include general loads and prescribed motion, improved entity/index
handling, and execution tiles once the system boundaries are stable.
