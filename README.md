# 3D Spatial LIDAR-Mapping System

## Project Overview
This project implements a cost-effective, portable LiDAR (Light Detection and Ranging) system for indoor 3D mapping. A Pololu VL53L1X Time-of-Flight (ToF) sensor is mounted on a 28BYJ-48 stepper motor and controlled by a Texas Instruments MSP-EXP432E401Y microcontroller. The system collects distance measurements in the y-z plane at fixed angular increments and transmits them over UART to a PC, where a MATLAB script converts the data into a 3D point cloud visualization.

### Key Features
- 360° spatial scanning in the y-z plane with 32 measurements per revolution (11.25° step increments)
- Manual x-axis displacement between scans for full 3D reconstruction
- On-board status LEDs and push buttons for scan control
- Real-time data transmission over UART at 115200 bps

## Components Used
### Hardware
| Component                         | Description                                                 |
|-----------------------------------|-------------------------------------------------------------|
| **MSP-EXP432E401Y Microcontroller** | ARM Cortex-M4F, 32-bit CPU, 1 MB flash, 256 kB SRAM, I2C/UART |
| **28BYJ-48 Stepper Motor**          | Unipolar 4-phase, 5.625° stride angle (32 steps/rev used)   |
| **ULN2003 Stepper Motor Driver**    | 5–12 V input, 4-channel driver for stepper coils             |
| **Pololu VL53L1X ToF Sensor**       | 940 nm Class 1 VCSEL, up to 4 m range, I2C interface         |

### Software
| Software         | Purpose                              |
|------------------|--------------------------------------|
| **Keil MDK (C)** | Microcontroller firmware development |
| **MATLAB**       | Serial data processing and 3D visualization |
