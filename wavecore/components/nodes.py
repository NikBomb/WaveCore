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