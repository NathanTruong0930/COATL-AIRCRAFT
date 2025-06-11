# COATL-AIRCRAFT

## Team 3 Members:
- Felix Moss
- Annika Boyd
- Eisa Alsharifi
- Nathan Truong

---

### Project: Electrostatic Dust Analyzer

This repository contains code and resources for the **Electrostatic Dust Analyzer** project developed as part of the PSU Capstone program. This project utilizes a QT PY ESP32 Development Board to interpret current measurements taken from dust particles for use in determining air conditions. 

---

### Key Features

1. Real-Time Detection of Charged Particles
Detects volcanic ash, mineral dust, and wildfire smoke using induced charge measurements.

2. Electrostatic Sensing Technology
Uses a Faraday tube and wire electrodes to sense small electrical currents from passing particles.

3. Wireless Data Transmission
The QT PY ESP32 Development Board transmits measurements via Wi-Fi to a web interface for real-time monitoring.

4. Onboard Signal Processing
Calculates induced charge and estimates particle size based on charge density assumptions.

5. Multi-Stage Gain Amplifier
Manually switchable gain stages (3.9×, 15×, 47×) allow for a wide range of signal amplitudes.

6. Web Interface for Visualization
Displays live data including voltage, charge, gain level, and particle size.

7. Compact, Low-Power Design
Operates on a 9V battery at ~1.35 W, suitable for eventual aircraft integration.

8. Lab-Tested with Real Volcanic Ash
Validated through controlled experiments using Mt. St. Helens ash samples.

---

### Repository Overview

- **412_Practicum:** Assignments and other in class documents
- **Bill_of_Materials:** List of components used in the project
- **Fusion_Files:** PCB box fusion files
- **Gantt_Chart:** Project Gantt chart
- **Kicad:** All project KiCAD files
- **LTSpice_Simulation:** All LTSpice files
- **Research_Documents:** Documents and notes used for research purposes
- **Team_Meeting_Notes:** Notes from team meetings and discussions
- **Testing_Documentation:** Documentation of tests run by the team
- **Weekly_Progress_Report:** Weekly log of progress by the team

---

### Notes

- The files present in the **KiCAD** folder are of older designs and are not of the final PCB enclosure, which was provided by our sponsor Josh Méndez.
- 

---

### License:

This project is licensed under the [GPL-3.0 license](LICENSE).