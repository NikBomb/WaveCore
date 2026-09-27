# WaveCore

WaveCore is an experimental explicit finite-element code for elastodynamics,
wave propagation, and future fracture-mechanics research.

The repository currently contains a tested finite-element core, not a complete
solver. Mesh construction, time integration, material updates, and global
force assembly are not yet connected into an end-to-end simulation.

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

The tests cover concept conformance, material initialization, stateless
element kernels, geometry and Jacobian calculations, force integration,
thickness scaling, shared-node scattering, archetype storage, and relation
indexes.

## Status and next steps

The separate archetypes, external relations, stateless element kernel, and
generic geometry/material/force systems are implemented. The next work is to
add non-owning entry views, improve entity/index handling, and introduce
execution tiles after the storage and system boundaries are stable.
