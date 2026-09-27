# Chiappa direct-nodal-initial-condition comparison

Both meshes use the same direct nodal prescription: `vx = 1 m/s` for
reference coordinates in the closed square `[0.45, 0.55] x [0.45, 0.55]`;
zero velocity elsewhere, and `vy = 0` everywhere. Patch-edge nodes receive
the full value. Displacement and stress start at zero. No Fourier-series
initialization, smoothing, projection, or averaging is used.

The initial CSVs record every node's assigned velocity: 289 excited nodes
on the 160 mesh and 1089 on the 320 mesh. These are nodal data; ordinary
element shape functions still enter FEM strain and force calculations.

Both meshes use one-point Quad4, unit thickness, plane strain, lumped mass,
no damping or hourglass control, and dt = 1e-8 s. Normal displacement and
velocity are constrained on each outer edge; tangential motion is free.

Each run produces:

- `nodal_N.csv.initial.csv`: initial nodal coordinates and velocities.
- `nodal_N.csv.71us.csv`: the domain at 71 microseconds, near the paper's
  rounded snapshot time (not the final state).
- `nodal_N.csv.history.csv`: the probe at (0.25, 0.25) through 470 microseconds.
- `nodal_N.csv`: final domain at 470 microseconds.

Snapshot files carry their actual time. Analytical values use 80 x 80 modes.
Domain plots use common color scales across meshes for each component, and
separate common scales for errors. History errors are computed over the full
time interval; field errors are discrete nodal norms at the snapshot time.

Reproduction from the repository root (release build recommended):

```sh
cmake -S . -B build/chiappa-release -DCMAKE_BUILD_TYPE=Release -DWAVECORE_ENABLE_TESTS=OFF -DWAVECORE_ENABLE_SANITIZERS=OFF
cmake --build build/chiappa-release --target chiappa_bulk_wave -j2
build/chiappa-release/apps/chiappa_bulk_wave artifacts/chiappa_bulk/nodal_160.csv 160
build/chiappa-release/apps/chiappa_bulk_wave artifacts/chiappa_bulk/nodal_320.csv 320
MPLCONFIGDIR=/tmp/wavecore-mpl python3 scripts/plot_chiappa_domains.py artifacts/chiappa_bulk/nodal_160.csv.71us.csv artifacts/chiappa_bulk/nodal_320.csv.71us.csv artifacts/chiappa_bulk/nodal_comparison
MPLCONFIGDIR=/tmp/wavecore-mpl python3 scripts/plot_chiappa_refinement.py artifacts/chiappa_bulk/nodal_160.csv.history.csv artifacts/chiappa_bulk/nodal_320.csv.history.csv artifacts/chiappa_bulk/nodal_comparison/history.png
```
