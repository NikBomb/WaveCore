"""
Mass systems for the 1D updated-Lagrangian bar.

The solver uses a lumped nodal mass vector. For the Lagrangian
formulation, nodal mass is computed from reference element mass
and remains constant during the simulation.

For element e:

    m_e = rho0_e * A0_e * L0_e

and for a two-node linear element:

    m_node[e]     += 0.5 * m_e
    m_node[e + 1] += 0.5 * m_e
"""

from __future__ import annotations

from wavecore.components import NodeComponents, ElementComponents


def clear_node_mass(nodes: NodeComponents) -> None:
    """
    Reset lumped nodal masses.

    Writes
    ------
    nodes.m
    """

    nodes.m.fill(0.0)


def update_lumped_node_mass(
    nodes: NodeComponents,
    elements: ElementComponents,
) -> None:
    """
    Assemble lumped nodal masses for the ordered 1D bar.

    Ordered 1D convention:
        element e connects node e and node e + 1

    Element mass:
        m_e = rho0_e * A0_e * L0_e

    Lumped contribution:
        m[e]     += 0.5 * m_e
        m[e + 1] += 0.5 * m_e

    Reads
    -----
    elements.rho0
    elements.A0
    elements.L0

    Writes
    ------
    nodes.m
    """

    clear_node_mass(nodes)

    element_mass = elements.rho0 * elements.A0 * elements.L0

    nodes.m[:-1] += 0.5 * element_mass
    nodes.m[1:] += 0.5 * element_mass