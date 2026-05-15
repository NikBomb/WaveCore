"""
Timestep systems for explicit 1D elastodynamics.

The stable timestep is estimated element-by-element using

    dt_e = l_e / c_e

where

    c_e = sqrt(E_e / rho_e)

The accepted timestep is

    dt_next = safety_factor * min(dt_e)
"""


import numpy as np

from wavecore.components import ElementComponents, TimeComponents


def update_element_wave_speed(elements: ElementComponents) -> None:
    """
    Update current elastic wave speeds.

    In 1D linear elasticity:

        c_e = sqrt(E_e / rho_e)

    Writes
    ------
    elements.c
    """

    elements.c[:] = np.sqrt(elements.E / elements.rho)


def update_element_critical_timestep(elements: ElementComponents) -> None:
    """
    Update element critical timesteps.

        dt_e = l_e / c_e

    Writes
    ------
    elements.dt_e
    """

    elements.dt_e[:] = elements.l / elements.c


def update_next_timestep(elements: ElementComponents, time: TimeComponents) -> None:
    """
    Update accepted timestep for the next increment.

        dt = safety_factor * dt_crit

    Writes
    ------
    time.dt
    """
    dt = float(np.min(elements.dt_e))
    time.dt_next = time.safety_factor * dt


def update_timestep(
    elements: ElementComponents,
    time: TimeComponents,
) -> None:
    """
    Update all timestep-related quantities.

    Writes
    ------
    elements.c
    elements.dt_e
    time.dt_crit
    time.dt_next
    """

    update_element_wave_speed(elements)
    update_element_critical_timestep(elements)
    update_next_timestep(time)


def initialize_timestep(
    elements: ElementComponents,
    time: TimeComponents,
) -> None:
    """
    Initialize the first explicit timestep.

    This should be called after the kinematics system has initialized:

        elements.l
        elements.J
        elements.rho

    Writes
    ------
    time.dt
    time.dt_next
    time.dt_crit
    time.is_initialized
    """

    update_timestep(elements, time)

    time.is_initialized = True

    time.validate()

def advance_time(time_component: TimeComponents):
    """
    Advance the time, and update current timestep

    Writes
    ------

    time.t
    time.dt_current
    time.dt_next = 0.0
    """
    time_component.t += time_component.dt_next
    time_component.dt_current = time_component.dt_next
    time_component.dt_next = 0.0