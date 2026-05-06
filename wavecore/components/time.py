from dataclasses import dataclass
from numpy.typing import NDArray
import numpy as np

FloatArray = NDArray[np.float64]

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

