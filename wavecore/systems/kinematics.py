from wavecore.components.elements import ElementComponents
from wavecore.components.nodes import NodeComponents

def update_element_length(elements: ElementComponents, nodes: NodeComponents) -> None:
    """
    Update current element lengths for an ordered 1D mesh.

    Mesh convention
    ---------------
    element e connects node e and node e + 1

    Current nodal coordinate
    ------------------------
    x_i = X_i + u_i

    Current element length
    ----------------------
    l_e = x_{e+1} - x_e

    Writes
    ------
    elements.l
    """
     
    elements.l[:] = nodes.x[1:] - nodes.x[:-1]


def update_element_density(elements: ElementComponents) -> None:
    """
    Updates the current density of the lements given the initial density and the current jacobian.

    Writes
    ------
    elements.rho
    """

    elements.rho[:] = elements.rho0 / elements.J




def update_node_current_position(nodes: NodeComponents) -> None:
    """
    Update current node coordinate from reference configuration and current displacement

    Writes
    ------
    nodes.x
    """

    nodes.x = nodes.X + nodes.u

def update_node_acceleration(nodes:NodeComponents) -> None:
    """
    Update current accelerations

    Writes
    ------
    nodes.a
    """

    nodes.a[:] = nodes.f/nodes.m

def update_element_jacobian(elements: ElementComponents) -> None:
    """
    Update element Jacobians

    Writes
    ------
    elements.J
    """

    elements.J[:] = elements.l / elements.L0

def update_element_rate_of_deformation(elements: ElementComponents, nodes: NodeComponents) -> None: 
    """
    Update element rate of deformation D

    Writes
    ------
    elements.D
    """

    elements.D[:] = (nodes.v[1:] - nodes.v[:-1]) / elements.l

def update_element_kinematics(
    elements: ElementComponents,
    nodes: NodeComponents
) -> None:
    """
    Update all 1D element kinematic quantities.

    Writes:
        nodes.x
        elements.l
        elements.J
        elements.rho
        elements.D
    """

    update_node_current_position(nodes)
    update_element_length(nodes=nodes, elements=elements)
    update_element_jacobian(elements)
    update_element_density(elements)
    update_element_rate_of_deformation(nodes=nodes, elements=elements)