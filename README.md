# Seat Heater Control System

Implementation of a seat heater control system for the front two seats in the car (driver seat and passenger seat).

## Table of Contents
- [Introduction](#introduction)
- [System Overview](#system-overview)
- [System Features](#system-features)
- [Functionality](#functionality)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Introduction

The Seat Heater Control System is designed to regulate the temperature of the front two seats in a car, providing comfort to the occupants. It consists of buttons for input, temperature sensors, heating elements, and a shared screen to display system status.

## System Overview

Each seat includes:
1. Button for temperature input.
2. Temperature sensor to monitor temperature.
3. Heating element for temperature control.
4. Shared screen to display system status.

## System Features

1. Heating levels: Off, Low (25°C), Medium (30°C), High (35°C).
2. Temperature control within desired range ±3°C.
3. Diagnostics for temperature sensor failures.
4. All data displayed on car screen.
5. Control buttons in car console and steering wheel.

## Functionality

1. User selects heating level: Off, Low, Medium, High.
2. Heater intensity adjusted based on temperature differential.
3. Temperature sensor connected to ADC.
4. Current temperature, heating level, and heater state displayed on screen.
5. Failure detection halts temperature control and activates red LED.

## Requirements

1. Implemented with Tiva C.
2. Utilizes FreeRTOS for task management.
3. Modules: GPIO, UART, GPTM, ADC.
4. Diagnostics stored in RAM.
5. Runtime measurements with GPTM.

## Installation

- Clone the repository: `git clone https://github.com/Youssef-Abouzeid/Seat-Heater-Control-System`
- Follow installation instructions in project documentation.

## Usage

1. Ensure all hardware components are connected properly.
2. Compile and flash the software onto the Tiva C controller.
3. Follow user manual for operating the seat heater control system.

## Contributing

Contributions are welcome! Please follow the contribution guidelines outlined in the project repository.

## License

This project is licensed under the [MIT License](LICENSE).
