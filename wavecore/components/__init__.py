"""
WaveCore component definitions.

Components are data-only containers. Systems operate on components
but do not own simulation data.
"""

from .nodes import NodeComponents
from .elements import ElementComponents
from .time import TimeComponents
from .world import World

__all__ = [
    "NodeComponents",
    "ElementComponents",
    "TimeComponents",
    "World",
]