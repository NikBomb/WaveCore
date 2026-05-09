"""
Advance system, 
in charge of stepping the solution through time
"""

import numpy as np
from wavecore.components import NodeComponents, TimeComponents

def advance_nodes_velocity(nodes: NodeComponents, time: TimeComponents) -> None:
    """
    Advance node velocities.
    Note that constant dt is assumed 

    Writes
    ------
    nodes.v

    """
    nodes.v[:] += nodes.a * 0.5 * time.dt_current

def advance_nodes_displacement(nodes: NodeComponents, time: TimeComponents) -> None:
    """
    Advance node displacements.

    Writes
    nodes.u 
    """

    nodes.u[:] += nodes.v + nodes.a*time.dt_current

