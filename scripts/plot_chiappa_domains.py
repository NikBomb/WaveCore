"""Render matched-time nodal fields on two meshes, with common color scales.

Usage: plot_chiappa_domains.py SNAPSHOT160.csv SNAPSHOT320.csv OUTPUT_DIRECTORY
The snapshot CSVs include their actual time, avoiding hard-coded plot labels.
"""
import pathlib
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

paths = [pathlib.Path(p) for p in sys.argv[1:3]]
output = pathlib.Path(sys.argv[3])
output.mkdir(parents=True, exist_ok=True)
data = [np.genfromtxt(p, delimiter=',', names=True) for p in paths]
time = float(data[0]['time'][0])
assert all(np.allclose(d['time'], time, rtol=0, atol=1e-14) for d in data)
meshes = []
for d in data:
    x, y = np.unique(d['x']), np.unique(d['y'])
    assert len(d) == len(x)*len(y)
    assert np.allclose(d['x'], np.tile(x, len(y)))
    assert np.allclose(d['y'], np.repeat(y, len(x)))
    meshes.append((x, y))


def draw(ax, index, values, limit, title):
    x, y = meshes[index]
    artist = ax.pcolormesh(x, y, values.reshape(len(y), len(x))*1e6,
                          shading='nearest', cmap='RdBu_r', vmin=-limit, vmax=limit,
                          rasterized=True)
    ax.set(xlim=(0, 1), ylim=(0, 1), xlabel='x [m]', ylabel='y [m]', title=title)
    ax.set_aspect('equal')
    return artist


limits = {}
for component in ('ux', 'uy'):
    field = max(np.max(np.abs(d[key+'_'+component])) for d in data
                for key in ('numerical', 'analytical'))*1e6
    error = max(np.max(np.abs(d['numerical_'+component]-d['analytical_'+component]))
                for d in data)*1e6
    limits[component] = field, error

fig, axes = plt.subplots(2, 5, figsize=(19, 8), layout='constrained')
for row, component in enumerate(('ux', 'uy')):
    columns = [(1, data[1]['analytical_'+component], 'Analytical', False)]
    for index, d in enumerate(data):
        n = len(meshes[index][0])-1
        columns.append((index, d['numerical_'+component], f'{n} x {n}', False))
    for index, d in enumerate(data):
        n = len(meshes[index][0])-1
        columns.append((index, d['numerical_'+component]-d['analytical_'+component],
                        f'{n} x {n} error', True))
    for ax, (index, values, title, error) in zip(axes[row], columns):
        artist = draw(ax, index, values, limits[component][int(error)], component+' | '+title)
        fig.colorbar(artist, ax=ax, shrink=.7, label='micrometres')
fig.suptitle(f'Direct nodal IC | domain comparison at {time*1e6:g} microseconds\n'
             'Common displacement and error scales within each component; no plot smoothing')
fig.savefig(output/'domain_comparison.png', dpi=180)
plt.close(fig)

for index, d in enumerate(data):
    n = len(meshes[index][0])-1
    fig, axes = plt.subplots(2, 3, figsize=(12, 8), layout='constrained')
    for row, component in enumerate(('ux', 'uy')):
        ref, num = d['analytical_'+component], d['numerical_'+component]
        for col, (values, title) in enumerate(zip((ref, num, num-ref),
                                                 ('Analytical', 'Numerical', 'Error'))):
            artist = draw(axes[row, col], index, values, limits[component][int(col==2)],
                          component+' | '+title)
            fig.colorbar(artist, ax=axes[row, col], shrink=.75, label='micrometres')
    fig.suptitle(f'{n} x {n} elements | direct nodal IC | t = {time*1e6:g} microseconds')
    fig.savefig(output/f'domain_{n}.png', dpi=180)
    plt.close(fig)
    error = np.column_stack([d['numerical_'+c]-d['analytical_'+c] for c in ('ux', 'uy')])
    ref = np.column_stack([d['analytical_'+c] for c in ('ux', 'uy')])
    print(n, 'snapshot relative nodal L2 [%]', 100*np.linalg.norm(error)/np.linalg.norm(ref))
