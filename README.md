# Vehicle Sorting Platform

A rotating IoT platform that classifies toy vehicles by length (light or heavy) and turns them toward the matching exit. It reports live state to a web dashboard.

*Making & Hacking mini-project, Team 4, Tecnun – Universidad de Navarra, May 2026.*

## How it works

- **Detect.** A VEML7700 lux sensor sits at the centre of the 100 mm platform. When a car rolls on, the lux reading drops sharply, and that is the trigger.
- **Classify.** Two VL53L0X laser distance sensors, 70 mm apart, look up from the platform. A small car (50 mm) covers at most one of them, so it is **light**. A large car (100 mm) covers both, so it is **heavy**. Gating on lux filters out noise.
- **Sort.** An SG90 servo drives a 90° bevel-gear stage that rotates the platform toward the right lane, then returns it to centre.
- **Report.** Two ESP8266 boards (a sensor board and an actuator board) talk over Wi-Fi to a FastAPI backend with SQLite. The backend decides the servo's target angle. A React dashboard shows live sensor state, the history of processed vehicles and manual controls.

### Engineering details

- **I²C address clash.** Both VL53L0X sensors boot at the same address. The firmware holds one in reset through its XSHUT pin, re-addresses the other to `0x30`, then releases the first and sets it to `0x31`.
- **Mechanics.** Servo → horizontal Ø5 mm shaft → POM bevel gears → vertical shaft → platform. The shafts run on 4 radial bearings plus an axial bearing that carries the platform's weight. The parts are 3D-printed in PLA, with a laser-cut MDF electronics box.

## This repository

```
firmware/sensor_board/sensor_board.ino   ESP8266: lux + 2x distance, posts to /datos and /vehiculo
firmware/sensor_board/secrets.example.h  Wi-Fi + backend settings (copy to secrets.h)
```

The commit history keeps the demo version, then a fix for two logic bugs in the classification step: an `if` without braces, and `=` used where `==` was meant.

The actuator board, the FastAPI backend and the React frontend were written by Alejandro Sempere and live in his repository. *(Link to be added.)*

### Build

1. In the Arduino IDE, install the ESP8266 board package plus the DFRobot_VL53L0X, DFRobot_VEML7700 and Adafruit Unified Sensor libraries.
2. Copy `secrets.example.h` to `secrets.h` and fill in your Wi-Fi network and backend URL.
3. Flash the sketch to a NodeMCU/ESP8266. D5 and D6 drive the XSHUT pins of the two distance sensors.

## Team

| Member | Contribution |
|---|---|
| Alejandro Sempere | Electronics and software (firmware, backend, web app), final integration |
| Carlos Tesoriero | Initial solution design (sensor choice and detection algorithm), electronics and software, assembly, presentation |
| Clemens Wiltschko | Mechanical design: servo housing, platform, printing and assembly |
| Martin Picaza | Mechanism planning, turning platform and model cars, assembly |
| Daniel Ullán | Gearbox housing, base, laser-cut box, drawings |
