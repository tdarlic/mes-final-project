# Making Embedded Systems Final Project

## 1. Application description
The project is digital barometer with algorithm that predicts the oncoming storms. Device sleeps with the screen off until one of the two things happen:
1. Device is picked up and handled
2. Storm is detected

Accelerometer and gyro will assist in detecting that the device was picked up and would also detect the orientation of the device. 
In case of storm the device would periodically light up a screen and would diplay a short warning about the oncoming storm.
The algorithm for detecting of storms would read the trend of the atmospheric pressure during last 6 or 12 hours and would interpret the results

## 2. Hardware description
Device is assembled out of DISCO-F429ZI development board and the small "shield" attached to it. The sheild is carrying the accelerometer and the barometer.
Following sensors are used in addition to the gyro already supplied by the board:
- [LPS28DFW](https://www.st.com/en/mems-and-sensors/lps28dfw.html#documentation) Dual full-scale, 1260 hPa and 4060 hPa, absolute digital output barometer with water-resistant package 
- [MMA8653FC](https://www.nxp.com/products/sensors/accelerometers/2g-4g-8g-low-g-10-bit-digital-accelerometer:MMA8653FC) 10-Bit Digital Accelerometer
- [MMA8652FC](https://www.nxp.com/products/sensors/accelerometers/2g-4g-8g-low-g-12-bit-digital-accelerometer:MMA8652FC) 12-Bit Digital Accelerometer

PCB was designed and ordered but were late to arrive so I had to etch my own PCB at home. As accelerometer and the barometer are usually QFN/DFN or some custom footprint that are
hard to etch and solder at home I used the brakeout board for the MMA8653FC that I had on hand but later I shifted to MMA8652FC once the board arrived. 
Because of this the code contains the version for both accelerometers. 

Below is the schematic of the final PCB that was used:
![Schematic](/documents/images/disco-board-pcb.png)

Images of the etched prototype board:
![etched-top](/documents/images/etched_top.jpg)
![etched-bottom](/documents/images/etched_bottom.jpg)

Images of the etched prototype board:
![PCB-large](/documents/images/PCB_large.jpg)
![PCB-part](/documents/images/PCB_part.jpg)

#### Hardware block diagram
![Hardware block diagram](/documents/images/hardware_block_diagram.png#gh-light-mode-only)
![Hardware block diagram](/documents/images/hardware_block_diagram.dark.png#gh-dark-mode-only)

## 3. Software description
Code uses the main control loop which handles the majority of the logic of the device. Below are main parts of the software:
1. Main code contained in `main.c`
    - The code in `main.c` has initially been generated by the STMCube code generation and was then completed with custom functions
2. HAL code generated by the STMCube code generating addon
    - All of the HAL code has been generated by STMCube program, the setup was carried out in the graphical interface
3. Software drivers:
    - [LPS28DFW](https://github.com/STMicroelectronics/STMems_Standard_C_drivers)(BSD 3-Clause License)
    - [MMA8653FC](https://www.nxp.com/design/sensor-developer-resources/sensor-sw-component-library/sensor-drivers-for-nxp-sensors:SENSOR-DRIVERS)(Licence: LA_OPT_NXP_Software_License v34 February 2022)
    - Graphics library [LVGL](https://github.com/lvgl/lv_port_stm32f429_disco)(MIT License)
    - Graphics library example code which was made to work for this board and then adjusted for the project
    - HAL (including generated code for the TFT LCD and touch controller)
    - Debug command line (Code taken from the Elicia White MES Week 5 assignment and adapted to match the requirements of the system):
        - Accelerometer test functions
        - Barometer test functions
        - Screen test functions
        - Logging and mocking procedure for storm prediction algorithm

4. State machine table for the digital barometer
[Google docs link](https://docs.google.com/spreadsheets/d/12yw3s-WZycBy4THHcHLK3jvAh2Erp7io5NsqDaDRv0c/edit?usp=sharing)
| States           | Action           | Events  |                  |                  |                 |                  |                 | 
|------------------|------------------|---------|------------------|------------------|-----------------|------------------|-----------------|
|                  |                  | Timer   | Move             | Tilt-Portrait    | Tilt-Landscpe   | Button           | Warning         | 
| ON               | initialize       | x       | x                | x                | x               | x                | x               | 
| SLEEP            | sleep            | MEASURE | DISPLAY_PRESSURE | DISPLAY_PRESSURE | DISPLAY_TREND   | DISPLAY_PRESSURE | DISPLAY_WARNING | 
| MEASURE          | measure          | ERROR   | DISPLAY_PRESSURE | DISPLAY_PRESSURE | DISPLAY_TREND   | DISPLAY_PRESSURE | DISPLAY_WARNING | 
| DISPLAY_PRESSURE | display_pressure | MEASURE | DISPLAY_PRESSURE | DISPLAY_PRESSURE | DISPLAY_TREND   | DISPLAY_PRESSURE | DISPLAY_WARNING | 
| DISPLAY_TREND    | display_trend    | MEASURE | DISPLAY_TREND    | DISPLAY_PRESSURE | DISPLAY_TREND   | DISPLAY_TREND    | DISPLAY_WARNING | 
| DISPLAY_WARNING  | display_warning  | MEASURE | DISPLAY_WARNING  | DISPLAY_WARNING  | DISPLAY_WARNING | DISPLAY_WARNING  | DISPLAY_WARNING | 
| ERROR            | reset            | ON      | x                | x                | x               | x                | x               | 

#### Description of the states
- ON               - Initial state, all variables are reset and device started
- SLEEP            - State in which device loops with screen off waiting for the timer or movement/input
- MEASURE          - Measuring, this can be done in the background with ADC trough the timer
- DISPLAY_PRESSURE - Standard display pressure mode in which the device is displaying pressure in portrait mode
- DISPLAY_TREND    - Display pressure trend (graph) in last 6 hours in portait mode
- DISPLAY_WARNING  - Warning: pressure trend has shown that the storm might be approacing
- ERROR            - Some of the parameters of pressures is out of bounds, reset the device

#### Description of the events
- Timer             - Timer has run out and the device will either take measurement or display value
- Move              - Movement has been detected and device will react to that by showing the screen
- Tilt-Portait      - Device was set in portait orientation so display standard screen with preesure value
- Tilt-Landscpe     - Device was set in landscape mode so show the graph with pressure trend
- Button            - Button was pressed - activate the screen
- Warning           - Pressure trend indicates that the storm is approaching
- Describe the parts you wrote in some detail (maybe 3-5 sentences per module)
- Describe code you re-used from other sources, including the licenses for those

## 4. Diagram(s) of the architecture
![Hierarchy of control diagram](/documents/images/hierarchy_of_control_diagram.png#gh-light-mode-only)
![Hierarchy of control diagram](/documents/images/hierarchy_of_control_diagram.dark.png#gh-dark-mode-only)

## 5. Build instructions

### 5.1 How to build the system 
#### Hardware
For putting hardware together a pcb was developed so it can hold the 
#### Software
### 5.2 How you debugged and tested the system

## 6. Future
### What would be needed to get this project ready for production
To be viable digital barometer following should be considered:
* Size of the screen needed for the project if screen needed at all
* Minimum viable MCU for the project
* Battery power required
* PCB design and development
* Hardware and software tests required
* Enclosure design and production
* CE and FCC testing
### How would you extend this project to do something more? Are there other features you’d like?
This project could be made much smaller as a keychain for example so that it just warns user of the possible storm without any screen. It could maybe vibrate or have just a few status leds. 
Or, we can go different direction and have this as a part of the weather station project with the larger screen (possible e-paper) and with additional sensors. Sensors could be made remote using
BLM or similar wireless technology.
### How would you go about adding them?
I would rather prefer this to be small wearable project which could be part of a keychain or would be installed on a small boat with long lasting battery and would just flash or vibrate to 
warn that the storm is approaching. With the modern mobile phones usability of this device is really limited in the current form. 

## 7. Grading
- Self assessment of the project: for each criteria, choose a score (1, 2, 3) and explain your reason for the score in 1-2 sentences.
- Have you gone beyond the base requirements? How so?
