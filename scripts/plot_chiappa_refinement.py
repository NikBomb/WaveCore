"""Compare two mesh histories with the analytical probe solution."""
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

coarse = np.genfromtxt(sys.argv[1], delimiter=',', names=True)
fine = np.genfromtxt(sys.argv[2], delimiter=',', names=True)
fig, axes = plt.subplots(2, 1, figsize=(12, 7), sharex=True, layout='constrained')
for ax, component in zip(axes, ('ux', 'uy')):
    ax.plot(fine['time']*1e6, fine['analytical_'+component]*1e6,
            color='black', label='Analytical (80 x 80 series)', linewidth=1.8)
    for label, data, color in [('160 x 160', coarse, 'tab:orange'), ('320 x 320', fine, 'tab:blue')]:
        reference = data['analytical_'+component]
        error = data['numerical_'+component] - reference
        peak = np.max(np.abs(reference))
        print(label, component, 'relative L2 %', 100*np.linalg.norm(error)/np.linalg.norm(reference),
              'peak-normalized RMS %', 100*np.sqrt(np.mean(error**2))/peak,
              'peak-normalized max %', 100*np.max(np.abs(error))/peak)
        ax.plot(data['time']*1e6, data['numerical_'+component]*1e6,
                color=color, label=label, linewidth=1)
    ax.set_ylabel(component+' [micrometres]')
    ax.grid(alpha=.25)
    ax.legend()
axes[-1].set_xlabel('Time [microseconds]')
fig.suptitle('Mesh refinement: Chiappa probe at (0.25 m, 0.25 m)')
fig.savefig(sys.argv[3], dpi=180)
