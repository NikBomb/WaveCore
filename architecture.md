# WaveCore architecture

WaveCore is a work in progress. The repository contains a tested finite-element
core, but not yet a complete solver or time integrator.

## ECS model

WaveCore separates component storage, relationships, and behavior:

```text
Archetypes own components.
Relations own indexes between entities.
Systems join and update archetype data.
```

An archetype is a homogeneous component store. It is not responsible for
knowing which systems use it, and it does not own references to other
archetypes.

## Archetypes

### NodeArchetype

Owns nodal components:

```text
coordinates
displacement
velocity
acceleration
mass
internal force
external force
```

### ElementArchetype<Element>

Owns element-level components for one element formulation. For `Quad4`, this
currently includes element properties such as thickness. Element/node
connectivity is deliberately not stored here; it is a relation.

The element formulation is stateless and reusable. The archetype stores data,
not one element object per entity. Element-level derived geometry such as
measure and characteristic length is stored here.

### MaterialArchetype<Material>

Owns shared material definitions and constitutive parameters. For example, a
linear-elastic material entry contains Young's modulus, Poisson's ratio, and
density. It does not contain per-element stress history.

### GaussPointArchetype<Element, Material>

Owns integration-point components:

```text
physical shape-function gradients
Jacobian and determinant
stress
constitutive history
```

The geometry type depends on the element formulation. The material state type
depends on the material model. Each Gauss point has independent state even
when many points use the same material definition.

For a one-point Quad4, there is one Gauss-point entity per element. For a
four-point element, there are four Gauss-point entities per element.

## Relations

Relations contain indexes only. They do not contain the component objects they
refer to.

```text
ElementNodeRelation
    element_id → node_ids[local_node]

ElementGaussPointRelation
    element_id → gauss_point_ids[local_gauss_point]

GaussPointMaterialRelation
    gauss_point_id → material_id
```

This is the finite-element equivalent of relational connectivity. It permits
the same element or material archetype to participate in multiple systems
without embedding relationships into component storage.

The relation indexes should be treated as entity handles or stable IDs at the
semantic level. Physical row indices may be used internally for dense storage,
compaction, or fast iteration.

## Systems

Systems perform queries and joins. Multiple systems may read or update the
same archetype.

```text
GeometrySystem
    reads NodeArchetype
    reads ElementArchetype
    reads ElementNodeRelation
    writes GaussPoint geometry components

MaterialUpdateSystem
    reads GaussPoint geometry components
    reads GaussPointMaterialRelation
    reads MaterialArchetype
    writes GaussPoint material-state components

ForceAssemblySystem
    reads GaussPoint stress and geometry
    reads ElementGaussPointRelation
    reads ElementNodeRelation
    writes nodal internal forces
```

The stress system does not link archetypes permanently. It resolves the
relations at execution time and performs the required joins.

## Stateless element and material formulations

`Quad4` is a reusable formulation and kernel. It owns no coordinates,
connectivity, geometry, velocity, or material history.

```cpp
element.refresh_geometry(nodes, connectivity, geometry);
element.gather_velocities(nodes, connectivity, velocities);
auto strain_rate = element.strain_rate_tensor(geometry, velocities);
auto forces = element.internal_force(stresses, properties, geometry);
```

A new element implements the element concept: traits, properties, geometry
state, quadrature, geometry refresh, strain-rate evaluation, and local-force
integration. A new material implements its material concept: parameters,
state type, initialization, update, and stress access.

Neither needs to know about relation storage, systems, or future execution
tiles.

## Data flow

The intended update sequence is:

1. The solver updates nodal components.
2. `GeometrySystem` follows element/node relations and refreshes Gauss-point
   geometry.
3. A kinematic operation gathers nodal velocities and computes strain rates.
4. `MaterialUpdateSystem` follows Gauss-point/material relations and updates
   constitutive states for `dt`.
5. `ForceAssemblySystem` reads updated stress and geometry, integrates local
   forces, follows element/node relations, and accumulates nodal forces.

Material updates mutate history. Geometry refresh and force assembly do not
advance constitutive state.

## Future execution tiles

Tiles are not part of ownership. They will be non-owning execution views over
archetype ranges or relation ranges:

```text
System query
    → contiguous range or tile
    → element/material kernel
```

Tile size and layout can later be tuned for CPU cache/SIMD, AMD wavefronts, or
GPU workgroups without changing the archetype or relation model.

## Current implementation

Implemented:

- stateless `Quad4` formulation;
- `IElementConcept` and `IMaterialConcept`;
- `ElementArchetype` component storage;
- `MaterialArchetype` material-definition storage;
- `GaussPointArchetype` geometry and constitutive-state storage;
- external element/node, element/Gauss-point, and Gauss-point/material
  relations;
- local force integration, generic systems, and shared-node force scattering; and
- component, numerical, archetype, and relation tests.

Not yet implemented:

- execution tiles;
- non-owning entry/debug views;
- runtime query/factory integration; and
- an end-to-end solver.
