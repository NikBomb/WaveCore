from wavecore.components import ElementComponents
from wavecore.components import World
from wavecore.components import TimeComponents
from wavecore.components import NodeComponents
import numpy as np


def create_uniform_bar(
    *,
    length: float,
    num_elements: int,
    area: float,
    density: float,
    young_modulus: float,
    safety_factor: float = 0.8,
) -> World:
    """
    Create component arrays for a uniform 1D bar.

    The initial timestep is computed from the initial undeformed
    configuration using:

        dt = safety_factor * min_e(l_e / c_e)

    with:

        c_e = sqrt(E_e / rho_e)
    """

    if length <= 0.0:
        raise ValueError("Bar length must be positive.")

    if num_elements <= 0:
        raise ValueError("num_elements must be positive.")

    if area <= 0.0:
        raise ValueError("Area must be positive.")

    if density <= 0.0:
        raise ValueError("Density must be positive.")

    if young_modulus <= 0.0:
        raise ValueError("Young's modulus must be positive.")

    if not (0.0 < safety_factor <= 1.0):
        raise ValueError("safety_factor must be in the interval (0, 1].")

    num_nodes = num_elements + 1

    X = np.linspace(0.0, length, num_nodes, dtype=np.float64)

    u = np.zeros(num_nodes, dtype=np.float64)
    v = np.zeros(num_nodes, dtype=np.float64)
    a = np.zeros(num_nodes, dtype=np.float64)

    f_int = np.zeros(num_nodes, dtype=np.float64)
    f_ext = np.zeros(num_nodes, dtype=np.float64)
    f = np.zeros(num_nodes, dtype=np.float64)

    L0 = X[1:] - X[:-1]

    A0 = np.full(num_elements, area, dtype=np.float64)
    rho0 = np.full(num_elements, density, dtype=np.float64)
    E = np.full(num_elements, young_modulus, dtype=np.float64)

    sigma = np.zeros(num_elements, dtype=np.float64)

    # Initial current quantities
    l = L0.copy()
    rho = rho0.copy()
    D = np.zeros(num_elements, dtype=np.float64)

    c = np.sqrt(E / rho)
    dt_e = l / c
    dt_next = safety_factor * np.min(dt_e)
    J = l * 0.5

    # Lumped nodal mass
    m = np.zeros(num_nodes, dtype=np.float64)

    element_mass = rho0 * A0 * L0

    m[:-1] += 0.5 * element_mass
    m[1:] += 0.5 * element_mass


    nodes = NodeComponents(
        X=X,
        x=X.copy(),
        u=u,
        v=v,
        a=a,
        m=m,
        f_int=f_int,
        f_ext=f_ext,
        f=f,
    )

    elements = ElementComponents(
        L0=L0,
        A0=A0,
        rho0=rho0,
        E=E,
        sigma=sigma,
        l=l,
        rho=rho,
        D=D,
        dt_e=dt_e,
        J=J
    )

    time = TimeComponents(
        t=0.0,
        dt_next=dt_next,
        dt_current = dt_next,
        dt_safety_factor= safety_factor,
        step=0,
    )

    world = World(
        nodes=nodes,
        elements=elements,
        time=time,
    )

    world.validate()

    return world