from dataclasses import dataclass
from numpy.typing import NDArray
import numpy as np
from .nodes import NodeComponents
from .elements import ElementComponents
from .time import TimeComponents

FloatArray = NDArray[np.float64]

@dataclass(slots=True)
class World:
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