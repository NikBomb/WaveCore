# WaveCore

WaveCore is an experimental explicit finite-element code for elastodynamics,
wave propagation, and future fracture-mechanics research.

The repository currently contains a tested finite-element core, not a complete
solver. The application is still a placeholder; mesh construction, time
integration, material updates, and global force assembly are not yet connected
into an end-to-end simulation.

## Architecture: typed ECS-style material blocks

The central architectural idea is a typed block that behaves like an ECS
archetype.

`MaterialElementBlock<Element, Material>` groups elements that share the same
element formulation and material model. The block owns the data columns for
that group:

```text
MaterialElementBlock
├── one material definition
├── connectivity[element]
├── properties[element]
├── geometry[element][gauss point]
├── nodal velocities[element]
└── material states[element][gauss point]
```

The important consequence is that an element is not an owning object. `Quad4`
is a reusable, stateless formulation and kernel. It receives caller-owned data
and can process every element in a block:

```cpp
element.refresh_geometry(nodes, connectivity, geometry);
element.gather_velocities(nodes, connectivity, velocities);
auto strain_rate = element.strain_rate_tensor(geometry, velocities);
auto forces = element.internal_force(stresses, properties, geometry);
```

This provides ECS-style data ownership without requiring a general-purpose
entity registry. Blocks are homogeneous batches, which keeps material states
typed, avoids per-element allocations, and provides a natural boundary for
CPU vectorization or future GPU kernels. The current storage is contiguous by
column; it can later be changed to a flatter SoA or AoSoA layout without
changing the formulation model.

## Current components

- `Node<Dimension>` stores nodal coordinates, displacement, velocity,
  acceleration, mass, and forces.
- `Quad4` implements a two-dimensional four-node quadrilateral with one centre
  Gauss point.
- `LinearElasticPlaneStrain` implements isotropic small-strain plane-strain
  elasticity.
- `PlaneElementProperties` stores validated positive element thickness.
- `MaterialElementBlock` owns grouped element data and material history.
- `scatter_force` accumulates local element forces into shared nodes.
- C++ concepts validate element and material interfaces at compile time.

Geometry state is derived data: physical shape-function gradients, Jacobians,
and Jacobian determinants are rebuilt from nodal geometry. Material state is
persistent constitutive history and is initialized through the material.
Stress is part of the material state and is read after the material update for
force assembly.

## Constitutive and assembly flow

The intended block-level sequence is:

1. Refresh block geometry when nodal positions change.
2. Gather current nodal velocities.
3. Compute strain rates using the block geometry state.
4. Update each integration-point material state.
5. Read updated stress from each material state.
6. Integrate local internal forces.
7. Scatter and accumulate those forces into the nodes.

Material updates mutate history. Geometry refresh and force assembly do not
advance material history.

## Repository layout

```text
include/wavecore/blocks/       Typed material blocks
include/wavecore/elements/     Element formulations and properties
include/wavecore/materials/    Material concepts and constitutive models
include/wavecore/mesh/         Nodes and force scattering
tests/                         Component and numerical tests
apps/                          Placeholder executable
```

See [architecture.md](architecture.md) for the longer design description and
current limitations.

## Building and testing

The project uses CMake and requires a C++23 compiler.

```bash
cmake --preset gcc14-debug
cmake --build --preset gcc14-debug
ctest --preset gcc14-debug --output-on-failure
```

The test suite covers concept conformance, material-state initialization,
stateless element kernels, geometry and Jacobian calculations, internal-force
integration, thickness scaling, and shared-node force scattering.

## Status and next steps

The typed block and stateless element kernel are implemented. The next major
work is to add block-level material-update and force-assembly kernels, then
introduce the small runtime block interface needed to process heterogeneous
element/material blocks from a solver.
