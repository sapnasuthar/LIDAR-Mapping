# 3D Spatial LIDAR-Mapping System

## Project Overview
#### This project implements a cost-effective, portable LiDAR (Light Detection and Ranging) system for indoor 3D mapping. A Pololu VL53L1X Time-of-Flight (ToF) sensor is mounted on a 28BYJ-48 stepper motor and controlled by a Texas Instruments MSP-EXP432E401Y microcontroller. The system collects distance measurements in the y-z plane at fixed angular increments and transmits them over UART to a PC, where a MATLAB script converts the data into a 3D point cloud visualization.
