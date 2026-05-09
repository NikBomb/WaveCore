"""
System that implements the explicit dynamic integration by Belytschko.

Pag 333. Box 6.1

1. Initial Conditions and Initialization:
    a. set v0, sigma_0 and initial values of other material state variables;
       d0 = 0, n = 0, t=0; 
    b.   compute M                                                               -> update_lumped_node_mass
2. getforce                                                                      -> update_element_kinematics; update_timestep 
   a. Initialization: fn = 0; delta_t_crit = 0
   b. compute global external nodal forces                                       -> To do
   c. Loop over elements
    aa. Gather element modal displacement and velocities
    bb. feint = 0
    cc. Loop over quadrature points
        aaa. Compute measures of deformation: D, F, E                            -> update_element_rate_of_deformation; update_element_jacobian
        bbb. compute stress sigma_n by constitutive equation                     -> update update_linear_elastic_stress
        ccc. fint,e, n += Bt* sigman * wq * J                                    -> update_nodes_internal_forces (is scattered automatically)
    dd. Compute external element forces on node                                  -> To do
    ee. compute fen = feext - fe_int                                             -> update_nodes_forces
    ff. compute dt_crit                                                          -> update_timestep
    gg. scatter
   d. Update timestep
3. Compute accelerations
4. time update t_n+1 = t_n + deltaTn+1/2; t_n+1/2 = 0.5(t_n + t_n+1)
5. First update nodal velocities: v_n+1/2 = v_n + (t_n+1/2 - t_n)*a_n           -> simplified as 0.5*dt_current*a_n
6. Enforce velocity BC 
7. Update Nodal displacements
8. getforce
9. compute a_n+1
10. Second update partial velocities: v_n+1 = v_n+1/2 + (t_n+1 - t_n-1/2)*a_n+1 -> Keep using half dt_current
11. Energy Balance                                                              -> To do
12. update counter
13. Output; if simulation not complete go to 4.

"""

# At the fist timestep we need all the state. 
# Given that the 1d simulation is driven by velocity, 
# The displacement, stress etc is zero but we need to
# compute the timestep.

from wavecore.components import NodeComponents, ElementComponents, TimeComponents
from wavecore.systems.boundaries import apply_nodal_velocity_bc
from wavecore.systems.timestep import update_next_timestep

def applied_velocity_BC_prepare(nodes: NodeComponents, elements: ElementComponents, bc: callable[[float],float]) :
    """
    Computes step 1 to 4 of box above, at timestep 0
    
    """

    apply_nodal_velocity_bc(nodes, 0.0, 0.0, bc )



    


