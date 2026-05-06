"""
WaveCore component definitions.

Components are data-only containers. Systems operate on components
but do not own simulation data.
"""

from wavecore.components.nodes import NodeComponents
from wavecore.components.elements import ElementComponents
from wavecore.components.time import TimeComponents
from wavecore.components.world import World

__all__ = [
    "NodeComponents",
    "ElementComponents",
    "TimeComponents",
    "World",
]