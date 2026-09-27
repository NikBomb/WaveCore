#!/usr/bin/env python3
"""Render analytical, numerical, and error fields from a Chiappa snapshot CSV."""

import csv
import pathlib
import sys

import matplotlib.pyplot as plt
import numpy as np


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit("usage: plot_chiappa_snapshot.py INPUT.csv OUTPUT.png")
    input_path = pathlib.Path(sys.argv[1])
    output_path = pathlib.Path(sys.argv[2])
    with input_path.open(newline="") as stream:
        rows = list(csv.DictReader(stream))

    x = np.array([float(row["x"]) for row in rows])
    y = np.array([float(row["y"]) for row in rows])
    ux_num = np.array([float(row["numerical_ux"]) for row in rows])
    uy_num = np.array([float(row["numerical_uy"]) for row in rows])
    ux_ref = np.array([float(row["analytical_ux"]) for row in rows])
    uy_ref = np.array([float(row["analytical_uy"]) for row in rows])
    x_values = np.unique(x)
    y_values = np.unique(y)

    def field(values):
        result = np.empty((len(y_values), len(x_values)))
        for index, (xi, yi) in enumerate(zip(x, y)):
            result[np.where(y_values == yi)[0][0], np.where(x_values == xi)[0][0]] = values[index]
        return result

    fields = [field(ux_ref), field(ux_num), field(ux_num - ux_ref),
              field(uy_ref), field(uy_num), field(uy_num - uy_ref)]
    titles = ["Analytical $u_x$", "Numerical $u_x$", "$u_x$ error",
              "Analytical $u_y$", "Numerical $u_y$", "$u_y$ error"]
    vmax = max(np.max(np.abs(fields[0])), np.max(np.abs(fields[1])),
               np.max(np.abs(fields[3])), np.max(np.abs(fields[4])))
    error_vmax = max(np.max(np.abs(fields[2])), np.max(np.abs(fields[5])))

    figure, axes = plt.subplots(2, 3, figsize=(14, 8), constrained_layout=True)
    for index, (axis, values, title) in enumerate(zip(axes.flat, fields, titles)):
        limit = error_vmax if index in (2, 5) else vmax
        image = axis.imshow(values, origin="lower", extent=(0, 1, 0, 1),
                            cmap="RdBu_r", vmin=-limit, vmax=limit,
                            interpolation="nearest", aspect="equal")
        axis.set_title(title)
        axis.set_xlabel("x [m]")
        axis.set_ylabel("y [m]")
        figure.colorbar(image, ax=axis, shrink=0.8, label="displacement [m]")
    figure.suptitle("Chiappa bulk-wave benchmark at t = 7.1e-5 s")
    figure.savefig(output_path, dpi=180)


if __name__ == "__main__":
    main()
