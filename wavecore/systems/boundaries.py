import numpy as np
from wavecore.components import NodeComponents


"""
Boundary system:

This system is responsible for updating the velocity or boundary conditions

"""


def apply_nodal_velocity_bc(nodes: NodeComponents, idx: int, time: float, timeFct: callable[[float], float]) -> None:
    """
    Apply velocity BC at node idx and time t,
    using function timeFct

    Writes
    ------
    nodes.v[idx] = timeFct(t)
    """

    nodes.v[idx] = timeFct(time)

def apply_nodal_disp_bc(nodes: NodeComponents, idx: int, time: float, timeFct: callable[[float], float]) -> None:
    """
    Apply dispacement BC at node idx and time t,
    using function timeFct

    Writes
    ------
    nodes.u[idx] = timeFct(t)
    """

    nodes.u[idx] = timeFct(time)


