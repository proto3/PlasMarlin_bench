# PlasMarlin Test Bench

This repository contains code for PlasMarlin bench test.

The bench test is composed of :
    - The PlasMarlin hardware itself (Mega + RAMPS 1.4).
    - A second Arduino Mega that plays scenarios on inputs.
    - 4 Arduino micro that display stepper positions and the emulated analog plasma voltage.
    - A 8 channels Saleae analyzer.
    - A main board to link everything while giving access to some input through push buttons.
