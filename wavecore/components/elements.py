from dataclasses import dataclass
from numpy.typing import NDArray
import numpy as np

FloatArray = NDArray[np.float64]


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
    J: FloatArray          # Jacobian
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
