import numpy as np

from wavecore.systems import simulate_from_0_to
from wavecore.components import NodeComponents, ElementComponents, TimeComponents, World
from wavecore.builders import create_uniform_bar


def sine_pulse(t: float, pulse_width: float, amplitude: float )-> float :
    if t <= pulse_width:
        return np.sin(2 * np.pi / (2 * pulse_width) * t) * amplitude
    return 0.0

if __name__ == '__main__':

    E = 190
    rho = 8
    # Max velocity Load will create a peak of 150 MPa = 0.15 GPa. Use formula sigma = rho*c*v to find maximum v.``    
    c = np.sqrt(E/rho)
    max_stress = 0.15
    max_vel = max_stress / (c * rho)
    L = 1000 # Bar length

    n_elements = 100

    # Model a quarter of a Sine pulse
    pulse_width_time = (L * 0.25)/c
    end_time = L/c 

    bc = lambda t: sine_pulse(t, pulse_width=pulse_width_time, amplitude=max_vel)

    world = create_uniform_bar(length=L, num_elements=n_elements, area=1.0, density=rho, young_modulus=E)
    simulate_from_0_to(end_time, world=world, bc=bc)

 










