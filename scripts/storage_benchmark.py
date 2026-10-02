#!/usr/bin/env python3
"""Time the unchanged Chiappa application in an isolated, instrumented copy.

prepare never edits the input source. run uses one warm-up and three measured
runs by default. Instrumentation overhead is included; compare builds using
the same instrumentation. Output CSVs remain the numerical comparison oracle.
"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess
from itertools import zip_longest
import math


TIMER = r'''#pragma once
#include <array>
#include <chrono>
#include <cstdio>
namespace storage_benchmark {
using Clock = std::chrono::steady_clock;
inline std::array<double, 16> seconds{};
inline constexpr std::array<const char*, 16> names{
 "wall", "setup", "initial_conditions_output", "observer_output",
 "element_geometry", "point_geometry", "strain_rates", "constitutive",
 "force_assembly", "force_clear", "mass", "acceleration",
 "half_velocity", "displacement", "constraints", "critical_timestep"};
struct Timer {
 std::size_t index; Clock::time_point start = Clock::now();
 explicit Timer(std::size_t i): index(i) {}
 ~Timer() { seconds[index] += std::chrono::duration<double>(Clock::now()-start).count(); }
};
struct Report {
 ~Report() {
  std::fprintf(stderr, "STORAGE_BENCHMARK {");
  for (std::size_t i=0;i<seconds.size();++i)
   std::fprintf(stderr, "%s\"%s\":%.9f", i ? "," : "", names[i], seconds[i]);
  std::fprintf(stderr, "}\n");
 }
};
}
'''


def prepare(source, destination):
    if destination.exists():
        raise SystemExit(f"Destination already exists: {destination}")
    destination.mkdir(parents=True)
    for name in ("CMakeLists.txt", "include", "src", "apps", "tests", "cmake"):
        src, dst = source / name, destination / name
        if src.is_dir():
            shutil.copytree(src, dst)
        else:
            shutil.copy2(src, dst)
    (destination / "include/storage_benchmark_timer.hpp").write_text(TIMER)
    path = destination / "include/wavecore/systems/FiniteElementSystems.hpp"
    text = path.read_text()
    names = {
        "refresh_element_geometry": 4, "refresh_gauss_point_geometry": 5,
        "compute_strain_rates": 6, "update_material": 7,
        "assemble_internal_forces": 8, "clear_nodal_forces": 9,
        "assemble_lumped_mass": 10, "compute_accelerations": 11,
        "first_half_velocity_update": 12, "second_half_velocity_update": 12,
        "update_nodal_displacements": 13, "apply_velocity_constraints": 14,
        "critical_timestep": 15,
    }
    for name, index in names.items():
        pattern = rf"((?:void|double) {name}\([\s\S]*?\)\s*(?:noexcept\s*)?\{{)"
        text, count = re.subn(pattern,
            rf"\1\n    storage_benchmark::Timer benchmark_timer{{{index}}};", text, count=1)
        if count != 1:
            raise SystemExit(f"Cannot instrument {name}")
    path.write_text('#include "storage_benchmark_timer.hpp"\n' + text)
    path = destination / "apps/chiappa_bulk_wave.cpp"
    text = path.read_text().replace('int main(int argc, char** argv) {', '''int main(int argc, char** argv) {
    storage_benchmark::Report benchmark_report;
    storage_benchmark::Timer benchmark_wall{0};
    const auto benchmark_setup_start = storage_benchmark::Clock::now();''')
    text = text.replace('initial_nodes) {',
        'initial_nodes) {\n        storage_benchmark::Timer benchmark_ic{2};', 1)
    text = text.replace('observed_nodes) {',
        'observed_nodes) {\n        storage_benchmark::Timer benchmark_output{3};', 1)
    text = text.replace('    wavecore::ExplicitDynamicsSystem::run(', '''    storage_benchmark::seconds[1] = std::chrono::duration<double>(
        storage_benchmark::Clock::now() - benchmark_setup_start).count();
    wavecore::ExplicitDynamicsSystem::run(''')
    path.write_text('#include "storage_benchmark_timer.hpp"\n' + text)


def run(executable, output, meshes, repeats, cpu):
    output.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, OMP_NUM_THREADS="1", OPENBLAS_NUM_THREADS="1", MKL_NUM_THREADS="1")
    records = []
    for mesh in meshes:
        for iteration in range(repeats + 1):
            prefix = output / f"n{mesh}-run{iteration}.csv"
            command = ["taskset", "-c", str(cpu), "/usr/bin/time", "-f",
                       "STORAGE_RSS_KIB %M", str(executable), str(prefix), str(mesh)]
            result = subprocess.run(command, text=True, capture_output=True, env=env, check=True)
            (output / f"n{mesh}-run{iteration}.log").write_text(result.stdout + result.stderr)
            timing = json.loads(re.search(r"STORAGE_BENCHMARK (\{.*\})", result.stderr)[1])
            timing["simulation"] = timing["wall"] - timing["setup"] - timing["initial_conditions_output"] - timing["observer_output"]
            timing["rss_kib"] = int(re.search(r"STORAGE_RSS_KIB (\d+)", result.stderr)[1])
            records.append({"mesh": mesh, "iteration": iteration, "warmup": iteration == 0, **timing})
            (output / "raw.json").write_text(json.dumps(records, indent=2))
            print(f"N={mesh} run={iteration} simulation={timing['simulation']:.3f}s wall={timing['wall']:.3f}s", flush=True)
    summary = {}
    for mesh in meshes:
        rows = [r for r in records if r["mesh"] == mesh and not r["warmup"]]
        summary[mesh] = {key: {"median": statistics.median(r[key] for r in rows),
                              "min": min(r[key] for r in rows),
                              "max": max(r[key] for r in rows)}
                         for key in rows[0] if key not in ("mesh", "iteration", "warmup")}
    (output / "summary.json").write_text(json.dumps(summary, indent=2))


def compare(before, after, rtol, atol):
    """Compare the four existing Chiappa outputs, including layout and row count."""
    report = {}
    for suffix in ("", ".initial.csv", ".history.csv", ".71us.csv"):
        a, b = Path(str(before) + suffix), Path(str(after) + suffix)
        maximum = 0.0
        rows = 0
        with a.open() as left, b.open() as right:
            lcsv, rcsv = csv.reader(left), csv.reader(right)
            if next(lcsv) != next(rcsv):
                raise SystemExit(f"CSV headers differ: {suffix or 'final'}")
            for rows, pair in enumerate(zip_longest(lcsv, rcsv), 1):
                lrow, rrow = pair
                if lrow is None or rrow is None or len(lrow) != len(rrow):
                    raise SystemExit(f"CSV dimensions differ: {suffix or 'final'} row {rows}")
                for column, (lval, rval) in enumerate(zip(lrow, rrow)):
                    x, y = float(lval), float(rval)
                    if not math.isfinite(x) or not math.isfinite(y) or not math.isclose(x,y,rel_tol=rtol,abs_tol=atol):
                        raise SystemExit(f"Numerical mismatch: {suffix or 'final'} row {rows} column {column}: {x} vs {y}")
                    maximum = max(maximum,abs(x-y))
        report[suffix or "final"] = {
            "rows": rows, "max_absolute_difference": maximum,
            "byte_identical": hashlib.sha256(a.read_bytes()).digest() == hashlib.sha256(b.read_bytes()).digest(),
        }
    print(json.dumps(report,indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="operation", required=True)
    prep = sub.add_parser("prepare")
    prep.add_argument("source", type=Path)
    prep.add_argument("destination", type=Path)
    bench = sub.add_parser("run")
    bench.add_argument("executable", type=Path)
    bench.add_argument("output", type=Path)
    bench.add_argument("--meshes", type=int, nargs="+", default=[160])
    bench.add_argument("--repeats", type=int, default=3)
    bench.add_argument("--cpu", type=int, default=min(os.sched_getaffinity(0)))
    comparison = sub.add_parser("compare")
    comparison.add_argument("before", type=Path)
    comparison.add_argument("after", type=Path)
    comparison.add_argument("--rtol", type=float, default=1e-11)
    comparison.add_argument("--atol", type=float, default=1e-14)
    args = parser.parse_args()
    if args.operation == "prepare":
        prepare(args.source.resolve(), args.destination.resolve())
    elif args.operation == "run":
        if args.repeats < 1:
            parser.error("--repeats must be positive")
        run(args.executable.resolve(), args.output.resolve(), args.meshes, args.repeats, args.cpu)
    else:
        compare(args.before,args.after,args.rtol,args.atol)


if __name__ == "__main__":
    main()
