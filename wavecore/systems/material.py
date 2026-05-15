"""
Material systems for the 1D updated-Lagrangian bar.

The first material model is an incremental 1D linear elastic
stress update:

    delta_eps = D * dt
    sigma_new = sigma_old + E * delta_eps

where D is the spatial velocity gradient computed by the
kinematics system.
"""

from wavecore.components import ElementComponents, TimeComponents


def update_linear_elastic_stress(
    elements: ElementComponents,
    time: TimeComponents,
) -> None:
    """
    Update 1D Cauchy stress using an incremental linear elastic law.

    Reads
    -----
    elements.D
        Spatial velocity gradient / rate of deformation.

    elements.E
        Young's modulus.

    time.dt
        Current timestep.

    Writes
    ------
    elements.sigma
        Updated Cauchy stress.

    Formula
    -------
        delta_eps = D * dt
        sigma += E * delta_eps
    """

    elements.sigma[:] += elements.E * elements.D * time.dt_current