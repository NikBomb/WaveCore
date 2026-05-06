"""
WaveCore components.

This module defines the data layour for a 1D updated Lagrangian
explicit finite element  solver using a structure of arrays style.

Entity convention
-------------------
Node entities:
    node_id = 0, 1, ..., num_nodes-1

Element entities:
    element_id = 0, 1,..., num_elements - 1 

For the ordered 1D mesh:
  Element e connects node_id = element_id  and node_id = element_id + 1
  num_elements = num_nodes - 1

"""

from dataclasses import dataclass
from numpy.typing import NDArray
import numpy as np

FloatArray = NDArray[np.float64]

@dataclass(slots=True)
class NodeComponents:
    X : FloatArray              # Nodal reference coordinate
    u:  FloatArray              # Nodal displacements
    v:  FloatArray              # Nodal velocities
    a:  FloatArray              # Nodal accelerations
    m: FloatArray               # Nodal mass
    f_int: FloatArray           # Nodal internal forces
    f_ext: FloatArray           # Nodal external forces
    f: FloatArray               # Nodal residual force (f_ext - f_int)

    @property
    def num_nodes(self) -> int:
        return self.X.size
    
    def validate(self) -> None:
        n = self.num_nodes

        arrays = {
            "u": self.u,
            "v": self.v,
            "a": self.a,
            "m": self.m,
            "f_int": self.f_int,
            "f_ext": self.f_ext,
            "f": self.f,
        }

        for name, array in arrays.items():
            if array.shape != (n,):
                raise ValueError(
                    f"Node array '{name}' has shape {array.shape}, "
                    f"expected {(n,)}."
                )

        if np.any(self.m <= 0.0):
            raise ValueError("All nodal lumped masses must be positive.")
        

@dataclass(slots=True)
class ElementComponents:
    """
    Components attached to element entities.

    All arrays have shape:

        (num_elements,)
    """

    L0: FloatArray         # Reference element length
    A0: FloatArray         # Reference area
    rho0: FloatArray       # Reference density
    E: FloatArray          # Young's modulus

    sigma: FloatArray      # Current Cauchy stress
    l: FloatArray          # Current element length
    rho: FloatArray        # Current density
    D: FloatArray          # Spatial velocity gradient / rate of deformation
    dt_e: FloatArray       # Element critical timestep

    @property
    def num_elements(self) -> int:
        return self.L0.size

    def validate(self) -> None:
        n = self.num_elements

        arrays = {
            "A0": self.A0,
            "rho0": self.rho0,
            "E": self.E,
            "sigma": self.sigma,
            "l": self.l,
            "rho": self.rho,
            "D": self.D,
            "dt_e": self.dt_e,
        }

        for name, array in arrays.items():
            if array.shape != (n,):
                raise ValueError(
                    f"Element array '{name}' has shape {array.shape}, "
                    f"expected {(n,)}."
                )

        if np.any(self.L0 <= 0.0):
            raise ValueError("All reference element lengths must be positive.")

        if np.any(self.A0 <= 0.0):
            raise ValueError("All element areas must be positive.")

        if np.any(self.rho0 <= 0.0):
            raise ValueError("All reference densities must be positive.")

        if np.any(self.E <= 0.0):
            raise ValueError("All Young's moduli must be positive.")


@dataclass(slots=True)
class TimeComponents:
    """
    Global simulation time state.
    """

    t: float
    dt: float
    step: int
    dt_safety_factor: float
    is_initialized: bool = False

    def validate(self) -> None:
        if self.dt <= 0.0 and self.is_initialized:
            raise ValueError("Time step dt must be positive.")

        if self.dt_safety_factor > 1.0:
            raise ValueError("Safety factor for dt cannot be higher than 1.0.")
        
        if self.dt_safety_factor <= 0:
            raise ValueError("Safety factor must be positive and bigger than 0.")
        
        if self.step < 0:
            raise ValueError("Step counter must be non-negative.")


@dataclass(slots=True)
class Components:
    """
    Full component database for the WaveCore 1D solver.
    """

    nodes: NodeComponents
    elements: ElementComponents
    time: TimeComponents

    def validate(self) -> None:
        self.nodes.validate()
        self.elements.validate()
        self.time.validate()

        expected_num_elements = self.nodes.num_nodes - 1

        if self.elements.num_elements != expected_num_elements:
            raise ValueError(
                "For an ordered 1D mesh, expected "
                f"num_elements = num_nodes - 1 = {expected_num_elements}, "
                f"got {self.elements.num_elements}."
            )
        

def create_uniform_bar_components(
    *,
    length: float,
    num_elements: int,
    area: float,
    density: float,
    young_modulus: float,
    safety_factor: float = 0.8,
) -> Components:
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
    dt = safety_factor * np.min(dt_e)

    # Lumped nodal mass
    m = np.zeros(num_nodes, dtype=np.float64)

    element_mass = rho0 * A0 * L0

    m[:-1] += 0.5 * element_mass
    m[1:] += 0.5 * element_mass

    nodes = NodeComponents(
        X=X,
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
    )

    time = TimeComponents(
        t=0.0,
        dt=dt,
        step=0,
    )

    components = Components(
        nodes=nodes,
        elements=elements,
        time=time,
    )

    components.validate()

    return components