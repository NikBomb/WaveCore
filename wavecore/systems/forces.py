"""
Forces system for a 1d updated Lagrangian bar. 

For now the solver assumes no external forces, as 
the BCs are applied though displacement, velocity Bcs.

"""


from wavecore.components import NodeComponents, ElementComponents
import numpy as np

def clear_internal_forces(nodes: NodeComponents) -> None :
    """
    Clear the internal forces. Used at every timestep
    
    Writes
    ------

    nodes.f_int
    """

    nodes.f_int.fill(0.0)



def update_nodes_internal_force(nodes: NodeComponents, elements: ElementComponents) -> None:
    """
    Update the nodal internal forces from the element stress.

    In 1D the stress at the GP of the element is constant

    f_int_element = [-sigma*A, sigma*A]

    Writes
    ------
    nodes.f_int
    """
    clear_internal_forces(nodes)
    
    force = elements.sigma * elements.A0

    nodes.f_int[:-1] += - force
    nodes.f_int[1:]  +=   force


def update_nodes_residual_force(nodes: NodeComponents) -> None:
    """
    Update nodal residual force

    f = fext - fint

    At this moment fext is 0

    Writes
    ------

    nodes.f 
    """

    nodes.f[:] = -nodes.f_int

def update_forces(elements: ElementComponents, nodes: NodeComponents) -> None:
    """
    Single algorithm that takes the current stress state and updates the nodal internal forces

    Writes:

    nodes.fint
    nodes.f
    """

    update_nodes_internal_force(nodes=nodes, elements=elements)
    update_nodes_residual_force(nodes = nodes)

