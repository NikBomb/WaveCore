# Numeric fields and views

`FieldStorage<Entry,T>` owns one contiguous `std::vector<T>` for every unique
named entry. The entries are columns; the index within each column is an entity
storage row. All columns in one field have the same number of rows.

```cpp
#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldStorage.hpp"

wavecore::FieldStorage<wavecore::StressEntry2D,double> stress(100);
stress[wavecore::StressEntry2D::sxx][17] = 3.0;
auto view = stress.view();                 // pointers and extent; no value copy
view[wavecore::StressEntry2D::sxy][17] = 4.0;

const auto& owner = stress;
auto read_only = owner.view();             // FieldView<StressEntry2D,const double>
double xx = read_only[wavecore::StressEntry2D::sxx][17];
// stress[wavecore::VelocityEntry2D::vx][17] does not compile.

wavecore::ScalarStorage<double> density(100);
density[17] = 7800.0;
auto scalar_view = density.view();         // ScalarView<double>
double rho = scalar_view[17];
```

Both owners provide `size()`, `empty()`, `resize()`, `reserve()`, `clear()`,
`push_back()` and const/mutable `view()`. `FieldStorage::push_back()` accepts
one fixed-width array of component values. `ScalarStorage` also provides
`at()` and `emplace_back()`. Field selectors return spans; callers cannot resize
one component independently. Views provide `size()`, `empty()`, indexing and
`subview(first,count)`; they neither resize nor own their memory.

An entry enum is scoped, zero based and contiguous, with `count` denoting the
number of unique columns. Add shared entry definitions in `fields/Entries.hpp`.
Value types include floating point and integer types. Use `uint8_t` for numeric
flags; `bool` owners are rejected to avoid `vector<bool>` bit proxies.

Stress and strain-rate fields store symmetric tensors. Reversed shear names
are enum aliases: `sxy` and `syx` select exactly the same memory. A symmetric
matrix row also maps `(0,1)` and `(1,0)` to the same component. In 2D there are
three in-plane components. `StressEntryPlaneStrain` includes an independent
fourth `szz` column; `StressEntry2D` is the in-plane set for plane stress.
The 3D sets have six symmetric components. These entry sets do not introduce
new plane-stress or 3D constitutive physics. Tensor shear remains half the
engineering shear. Jacobian and gradient fields retain all matrix entries.

`ArrayRow`, `MatrixRow` and `SymmetricMatrixRow` provide familiar indexing over
one row. They are temporary non-owning proxies. Conversion to an array or
matrix makes an explicit value copy; assigning a value or another mutable row
copies components into the destination. Copy construction of a row proxy
keeps referring to the same row.

## Simulation ownership

| Owner | Fields |
| --- | --- |
| `NodeArchetype<D>` | Coordinates, displacement, velocity, acceleration, internal/external force: named component arrays; mass: scalar array |
| `ElementArchetype` | Measure, characteristic length and validity: scalar arrays; thickness: scalar array of the existing validated wrapper |
| `GaussPointArchetype` | Jacobian and physical gradients: named matrix entries; determinant: scalar array; strain rate: symmetric component arrays; independent constitutive stress/history: material adapter fields |
| Relations | Each fixed connectivity slot: named `size_t` array; Gauss-point material binding: scalar `size_t` array |
| `ConstraintMasks<D>` | Named `uint8_t` arrays for directional flags |

Relations contain storage rows. There is currently no separate entity-ID
allocator or remapping table. Reordering entities requires updating relations;
never assume element, point, material, and node row numbers coincide. Shared
nodes are still accumulated into in connectivity order. Material history is
advanced only in the material-update pass.

`NodeArchetype::values()` returns `NodeView<D>`; const owners return
`NodeView<D,const double>`. It exposes the component field views directly:

```cpp
auto nodes = node_archetype.values();
nodes.velocity[wavecore::VelocityEntry2D::vx][node_row] = 1.0;
nodes.mass[node_row] = 2.0;
nodes[node_row].displacement()[0] = 0.01;    // row proxy, same underlying storage
```

`GaussPointArchetype` provides `geometry_view()`, `state_view()`, `stress_view()`
and `strain_rate_view()`, including read-only variants. Relations provide
read-only `view()` members. Systems cache the views used by a pass; force
scattering accepts just a mutable force field view. Kernels still use small
local `Node`, `Matrix`, connectivity and quadrature values where useful.

Element and material record mappings are explicit adapters in
`fields/SimulationRecords.hpp`; relation slot mappings use
`ElementRelationEntries<Element>`. New formulations/materials must register
adapters describing their numeric fields. There is no runtime reflection or
fallback that silently stores arbitrary constitutive records as AoS.

## Lifetime and compatibility

Each view stores pointers and an extent; constructing or copying it does not
allocate or copy field values. Constness is encoded in its value type, as with
`span<const T>`. A const view object over mutable `T` can still modify data;
use `FieldView<Entry,const T>` or `ScalarView<const T>` for read-only access.

The owner must outlive all its views and row proxies. Destruction, assignment
and reallocation invalidate pointers. Resize/clear invalidates captured row
extents. Reacquire views after structural operations, including entity growth;
an allocation failure can also invalidate old pointers even if sizes are rolled
back. Moving an owner transfers its allocation: existing views depend on the
destination owner's lifetime, and views of a move-assignment destination's old
allocation become invalid. Subviews share these rules. Concurrent mutation
requires caller synchronization.

Archetype accessors now return row proxies rather than owning-record references,
and nodal ranges are views rather than contiguous `span<Node>` objects. Code
that explicitly binds `Node&`, geometry/state references or concrete span types
must use proxies/views or request an explicit value copy. Range loops should
use `auto&&` for mutable rows. These are necessary source API changes for
component storage; numerical behavior and output formats are preserved.

The dynamics driver supports existing initial-condition and observer callbacks
typed as `span<Node>`/`span<const Node>` through a reusable temporary snapshot.
It allocates that snapshot once per run, copies initial-condition edits back,
and fills it at output events for legacy observers. Such callback spans are
valid only during the callback and are not live aliases of canonical fields.
Direct view callbacks avoid this compatibility cost; the Chiappa application
uses them. Retain a value copy when an observer needs data after its callback.

## Exceptions and device boundary

Shared material definitions remain validated configuration objects held in
`ScalarStorage<Material>`. They are separate from mutable Gauss-point history.
`PlaneElementProperties` contains only the scalar thickness and remains a
validated scalar wrapper. Output schedules, strings, analytical-series
configuration, variable-length topology, and local kernel temporaries do not
belong in persistent numeric field storage. No execution tiles, ECS framework,
integrator ordering or scheduling changes were introduced.

There is no GPU backend in this repository. CPU owners produce host views;
ordinary host-vector pointers are not device allocations. The pointer-and-extent
view layout is suitable for a future device owner supplying GPU-accessible
memory and corresponding device accessor annotations. CPU views are not
advertised as GPU-executable, and GPU performance is unmeasured.

See [the benchmark report](../benchmarks/field_storage_baseline.md) for measured
correctness/performance, limitations and reproduction commands.
