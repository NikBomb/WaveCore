"""
WaveCore

A small explicit elastodynamics engine for wave propagation,
updated-Lagrangian finite elements, and fracture mechanics
experiments.

The initial implementation focuses on a 1D updated-Lagrangian
bar using a structure-of-arrays ECS-style architecture.
"""

from wavecore.components import (
    NodeComponents,
    ElementComponents,
    TimeComponents,
    World,
)

from wavecore.builders import create_uniform_bar


__all__ = [
    "NodeComponents",
    "ElementComponents",
    "TimeComponents",
    "World",
    "create_uniform_bar"
]