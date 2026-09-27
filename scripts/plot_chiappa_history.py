"""Plot the benchmark probe history; report explicitly normalized errors."""
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

data = np.genfromtxt(sys.argv[1], delimiter=',', names=True)
fig, axes = plt.subplots(2, 1, figsize=(11, 7), sharex=True, layout='constrained')
for ax, component in zip(axes, ('ux', 'uy')):
    numerical = data['numerical_' + component]
    analytical = data['analytical_' + component]
    error = numerical - analytical
    peak = np.max(np.abs(analytical))
    print(component, 'relative L2 [%]:', 100*np.linalg.norm(error)/np.linalg.norm(analytical),
          'peak-normalized RMS [%]:', 100*np.sqrt(np.mean(error**2))/peak,
          'peak-normalized maximum [%]:', 100*np.max(np.abs(error))/peak)
    ax.plot(data['time']*1e6, analytical*1e6, label='Analytical (80 x 80 series)', color='blue')
    ax.plot(data['time']*1e6, numerical*1e6, label='WaveCore (160 x 160 elements)', color='red', linewidth=1)
    ax.set_ylabel(component + ' [micrometres]')
    ax.grid(alpha=.25)
    ax.legend()
axes[-1].set_xlabel('Time [microseconds]')
fig.suptitle('Chiappa bulk wave: displacement at (0.25 m, 0.25 m)')
fig.savefig(sys.argv[2], dpi=180)
