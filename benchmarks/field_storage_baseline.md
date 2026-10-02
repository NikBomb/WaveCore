# Field storage baseline and inventory

The storage migration is implemented. The user instructed continuation after
the baseline and explicitly stopped the 320×320 workload. The comparison uses
only the completed 160×160 baseline measurements. API and compatibility details
are documented in [numeric fields and views](../docs/field_storage.md).

## Preserved baseline

- Commit: `0eb6836d86253e2750d058ec08422d34dd0553ec`.
- Existing user edits: `README.md` and `architecture.md`; both preserved.
- Source snapshot: `build/storage-baseline/source`.
- Uninstrumented executable: `build/storage-baseline/release/apps/chiappa_bulk_wave`.
- Instrumented snapshot and executable: `build/storage-baseline/instrumented-source`
  and `build/storage-baseline/instrumented-release/apps/chiappa_bulk_wave`.
- Full machine/compiler/environment record and original diff:
  `build/storage-baseline/environment.json` and `user-changes.patch`.
- Compiler: GCC 14.2.0, C++23, Release (`-O3 -DNDEBUG`), sanitizers disabled,
  existing warning flags and warnings as errors enabled. No fast-math or native
  architecture flags added.
- CPU: AMD Ryzen 5 8600G, 6 cores / 12 threads, 16 MiB L3, frequency boost enabled.
- Serial execution, pinned to logical CPU 0 for repeated measurements;
  `OMP_NUM_THREADS=OPENBLAS_NUM_THREADS=MKL_NUM_THREADS=1`.
- Baseline tests: 50 cases and 646 assertions passed in the optimized build.

The baseline directories are ignored by Git. Preserve them until numerical
and performance comparisons are complete. The saved source excludes documentation
and artifacts, and contains all CMake inputs, library/application code, and tests.

## Simulation storage inventory

| Owner | Persistent fields | Current representation | Migration requirement |
| --- | --- | --- | --- |
| `NodeArchetype<D>` | Reference coordinates, displacement, velocity, acceleration, internal/external force, mass | `vector<Node<D>>`, six embedded `array<double,D>` and one double | Separate named component arrays per vector field; scalar mass |
| `ElementArchetype<Quad4>` | Thickness, measure, characteristic length, validity | Vectors of properties and geometry records | Scalar numeric fields and integer validity field |
| `GaussPointArchetype<Quad4,Material>` | Jacobian, physical gradients, determinant | Vector of geometry records | Named rectangular matrix component arrays, scalar determinant; retain Jacobian because it is publicly accessible |
| Same | Strain-rate tensor | Vector of 2×2 matrices | Three symmetric component arrays; preserve tensor shear convention |
| Same | Constitutive history: 2×2 stress, `stress_zz` | Vector of material state records | Material-specific field mapping, preserving independent history per point and initial-state semantics |
| `MaterialArchetype<LinearElasticPlaneStrain>` | Density, shear modulus, Lamé lambda | Vector of material objects with three doubles | Numeric parameter fields need a material adapter; preserve validation and shared material definitions |
| `ElementNodeRelation` | Four node rows per Quad4 element | Vector of fixed arrays of `size_t` | Four named integer arrays; preserve order and shared-node references |
| `ElementGaussPointRelation` | Gauss-point rows per element | Vector of fixed arrays of `size_t` | Named fixed-width integer fields for each formulation |
| `GaussPointMaterialRelation` | Material row per point | Vector of `size_t` | Scalar integer field |
| Dynamics constraints | One flag per node/direction | Vector of fixed arrays of bool | Numeric flag arrays (avoid `vector<bool>` proxy storage) |

All current relation values are storage rows, despite some local names ending
in `_id`. There is no separate persistent entity-ID allocator or row-remapping
table. Tests must include shared nodes, permuted relation rows, and multiple
materials rather than assuming every relation is the identity mapping.

`Node<3>` exists, but the only element/material simulation implementation is
2D Quad4 with linear elastic plane strain. There is no plane-stress element or
3D constitutive implementation, GPU allocation/transfer mechanism, or execution
tile implementation to migrate.

## Required compatibility decisions

The current public API returns genuine mutable references to node arrays,
Gauss-point geometry/state records, material objects, and element properties.
`NodeArchetype::values()` returns `std::span<Node<D>>`; application callbacks
and element concepts accept these spans. Component arrays across many entities
cannot simultaneously expose contiguous owning `Node` records. The migration
uses node/record proxies and matching kernel overloads. Explicit owning-record
references and concrete span assumptions require source changes. Legacy span
callbacks use temporary snapshots allocated once per run; direct view callbacks
avoid copying. Snapshot callback lifetimes differ from live aliases and are
documented in the API guide. No duplicate canonical AoS simulation storage remains.

The user confirmed `xy` and `yx` represent the same symmetric tensor component.
They now alias one shear column in persistent fields. Local matrices and
standalone material operations retain their original value types and formulas.
Plane strain also retains `stress_zz`: zero out-of-plane strain does not mean
zero out-of-plane stress. Plane-stress and 3D named entry sets are provided,
without introducing new constitutive physics.

Geometry currently uses `Node::current_coordinates()` (`X + displacement`) and
is refreshed each timestep. This supersedes the historical geometry note in
`AGENTS.md`; preserve the actual implemented formulation and update order.

## Reproduction

From the repository root, with the preserved baseline source present:

```sh
cmake -S build/storage-baseline/source -B build/storage-baseline/release \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14 \
  -DWAVECORE_ENABLE_SANITIZERS=OFF \
  -DFETCHCONTENT_SOURCE_DIR_DOCTEST="$PWD/build/gcc14-debug/_deps/doctest-src"
cmake --build build/storage-baseline/release -j2
ctest --test-dir build/storage-baseline/release --output-on-failure

# prepare requires a destination that does not already exist
python3 scripts/storage_benchmark.py prepare \
  build/storage-baseline/source build/storage-baseline/instrumented-source
cmake -S build/storage-baseline/instrumented-source \
  -B build/storage-baseline/instrumented-release \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14 \
  -DWAVECORE_ENABLE_SANITIZERS=OFF -DWAVECORE_ENABLE_TESTS=OFF
cmake --build build/storage-baseline/instrumented-release -j2
python3 scripts/storage_benchmark.py run \
  build/storage-baseline/instrumented-release/apps/chiappa_bulk_wave \
  build/storage-baseline/measurements --meshes 160 --repeats 3 --cpu 0
```

For the refactored build, prepare a new isolated instrumented source directory
from the working repository, use identical CMake flags, and run with the same
mesh, CPU, environment and repeat count:

```sh
cmake -S . -B build/storage-refactor/release \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14 \
  -DWAVECORE_ENABLE_SANITIZERS=OFF \
  -DFETCHCONTENT_SOURCE_DIR_DOCTEST="$PWD/build/gcc14-debug/_deps/doctest-src"
cmake --build build/storage-refactor/release -j2
ctest --test-dir build/storage-refactor/release --output-on-failure

python3 scripts/storage_benchmark.py prepare . build/storage-refactor/measured-source
cmake -S build/storage-refactor/measured-source -B build/storage-refactor/measured-release \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14 \
  -DWAVECORE_ENABLE_SANITIZERS=OFF -DWAVECORE_ENABLE_TESTS=OFF
cmake --build build/storage-refactor/measured-release -j2
python3 scripts/storage_benchmark.py run \
  build/storage-refactor/measured-release/apps/chiappa_bulk_wave \
  build/storage-refactor/results --meshes 160 --repeats 3 --cpu 0

python3 scripts/storage_benchmark.py compare \
  build/storage-baseline/reference/nodal_160.csv \
  build/storage-refactor/results/n160-run1.csv
```

The preparation destinations must be new directories. For reruns, choose new
directory names or reuse the already preserved executables. The local doctest
path uses the repository's existing cached dependency; a fresh environment can
omit that option and let CMake fetch the pinned doctest version (network needed).
To reconstruct baseline source on a fresh checkout without changing the working
tree:

```sh
mkdir -p build/storage-baseline/source
git archive 0eb6836d86253e2750d058ec08422d34dd0553ec | tar -x -C build/storage-baseline/source
```

For the final sanitizer check, use the same configuration with
`-DCMAKE_BUILD_TYPE=Debug -DWAVECORE_ENABLE_SANITIZERS=ON` and run
`LSAN_OPTIONS=detect_leaks=0 ctest --test-dir build/storage-refactor/sanitize --output-on-failure`.
ASan and UBSan were enabled; leak detection was disabled in this sandbox.

## Measurement method

Run the full existing 160×160 Chiappa case to 470 microseconds,
including the existing history and snapshot schedule. Each workload has one
discarded warm-up and three measured repetitions. Record median and min/max;
this small sample is descriptive, not a confidence interval. Measurements run
sequentially to avoid benchmark processes competing with one another.

RAII timers are inserted only in the isolated instrumented source. All phase
times are inclusive of timer overhead. Nodal integration comprises acceleration,
both velocity updates, displacement updates and constraints; force clearing is
reported separately. Setup means application mesh/material/relation construction.
Initial-condition output and observer output (including analytical evaluation)
have separate timers. Simulation is application wall time minus setup and
those two callback costs; it includes driver initialization, mass assembly,
critical-timestep selection and event scheduling. File destruction/close costs
outside callbacks remain in the residual. Process startup is excluded from the
application wall timer. Maximum RSS comes from `/usr/bin/time` and includes the
whole process; it is not exact field-storage allocation size.

Raw values, logs, output CSVs and summary statistics are stored in
`build/storage-baseline/measurements`. Verify timer instrumentation by comparing
all four output CSVs with the uninstrumented executable. Use these saved output
CSVs as numerical oracles after migration; inspect any differences rather than
loosening tolerances automatically. GPU performance is unmeasured. The discarded 320×320 warm-up is not part of
the comparison; its measured repetitions were stopped at the user’s request.

## Measured results

One warm-up and three measured full runs per build. Times are seconds,
shown as median **[minimum–maximum]**. Positive change means more time/memory.

| Operation | Baseline | Refactored | Change |
| --- | ---: | ---: | ---: |
| Application wall | 74.637 [74.456–74.713] | 74.457 [74.346–74.831] | -0.24% |
| Simulation (setup/output excluded) | 50.657 [50.397–50.667] | 50.692 [50.669–51.112] | +0.07% |
| Mesh/material/relation setup | 0.006 [0.006–0.007] | 0.006 [0.006–0.006] | -4.46% |
| Initial conditions and IC output | 0.012 [0.011–0.012] | 0.011 [0.011–0.011] | -2.72% |
| Observers, analytical evaluation and output | 24.039 [23.952–24.041] | 23.701 [23.661–23.748] | -1.40% |
| Element geometry | 11.933 [11.888–11.981] | 11.129 [11.128–11.295] | -6.74% |
| Gauss-point geometry | 8.726 [8.667–8.742] | 11.159 [11.074–11.161] | +27.89% |
| Strain-rate computation | 5.847 [5.639–5.961] | 5.110 [5.063–5.393] | -12.61% |
| Constitutive update | 2.960 [2.938–2.962] | 3.252 [3.227–3.272] | +9.86% |
| Internal-force integration and assembly | 8.319 [8.214–8.362] | 8.368 [8.244–8.433] | +0.60% |
| Force clearing | 0.962 [0.960–0.964] | 0.430 [0.424–0.439] | -55.31% |
| Acceleration | 1.210 [1.206–1.211] | 2.427 [2.419–2.427] | +100.63% |
| Both velocity half updates | 1.864 [1.861–1.868] | 0.757 [0.755–0.763] | -59.41% |
| Displacement update | 0.926 [0.923–0.929] | 0.433 [0.433–0.459] | -53.19% |
| Constraints | 3.227 [3.225–3.239] | 3.090 [3.089–3.099] | -4.26% |
| Nodal integration (four phases combined) | 7.228 [7.226–7.233] | 6.707 [6.704–6.740] | -7.22% |
| Critical timestep | 4.647 [4.627–4.653] | 4.547 [4.541–4.550] | -2.16% |
| Initial mass assembly | 0.00010 [0.00010–0.00011] | 0.00009 [0.00008–0.00009] | -16.19% |
| Peak RSS (MiB) | 15.656 [15.648–15.656] | 13.828 [13.828–13.863] | -11.68% |

The simulation median increased 0.07%, smaller than the within-build timing
ranges (0.53% baseline and 0.87% refactored). Wall-time ranges overlap. Three
repetitions do not establish statistical equivalence or support a CPU speedup
claim. Peak process RSS is lower.
RSS includes setup peaks, allocator behavior and the whole process, rather
than only live numeric fields.

Phase regressions remain in Gauss-point geometry, constitutive updates, and
acceleration. These kernels reconstruct local records and access separate
component streams; attributing the costs to those accesses is an inference
from the code and phase timings. Hardware-counter profiling was not performed.
The first refactor
warm-up took 59.87 s for simulation and 18.08 s for Gauss-point geometry.
Phase timing and inspection of the optimized binary led to caching geometry
and state views outside entity loops, direct nodal coordinate access, and
force-only scattering. These kept the same pass ordering and arithmetic; the
final median change is smaller than the observed within-build timing variation.
No physics,
benchmark duration, output schedule, or optimization flags were changed.

Baseline: **50 tests / 646 assertions**. Refactored: **56 tests / 694 assertions**,
all passed in Release and ASan/UBSan builds. All sixteen refactored output files
(four each for warm-up and three repetitions) are byte-identical to the
uninstrumented baseline, including 4,701 history samples and 25,921 rows per
domain/IC file. Maximum observed numeric difference is zero; the comparison
script also supports `rtol=1e-11`, `atol=1e-14` for investigating future builds.

The preserved measured refactor source/executable are
`build/storage-refactor/measured-source` and
`build/storage-refactor/measured-release/apps/chiappa_bulk_wave`.
Raw records, output CSVs and per-run logs are in `build/storage-refactor/results`.
[Machine-readable results](field_storage_results.json) contain the summary
statistics. The final benchmark ran after compilation and sanitizer tests
finished. 320×320 and GPU performance are not reported.
