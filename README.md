# City Emergency Dispatch

## Overview

City Emergency Dispatch is a real-time embedded system project designed to manage emergency services such as police, fire department, and ambulance dispatch. The project is implemented using STM32 microcontrollers and FreeRTOS for real-time task management.

## Features

- Real-time task scheduling using FreeRTOS.
- Modular design for handling different emergency services.
- Logging and debugging support.
- Configurable project settings for STM32F7 series microcontrollers.

## Project Structure

```
CityEmergencyDispatch/
├── Core/
│   ├── Inc/        # Header files
│   ├── Src/        # Source files
├── Drivers/        # STM32 HAL drivers
├── Middlewares/    # Third-party libraries (e.g., FreeRTOS)
├── cmake/          # CMake configuration files
├── build/          # Build artifacts (ignored in version control)
├── README.md       # Project documentation
├── .gitignore      # Git ignore rules
└── CMakeLists.txt  # CMake build script
```

## Prerequisites

- STM32CubeIDE or STM32CubeMX for project configuration.
- GNU Arm Embedded Toolchain for building the project.
- STM32CubeProgrammer for flashing the firmware.
- CMake and Ninja build system.

## Build and Flash Instructions

1. **Configure the Project**:

   ```bash
   cmake -S . -B build/Debug -G Ninja
   ```

2. **Build the Project**:

   ```bash
   cmake --build build/Debug
   ```

3. **Flash the Firmware**:
   Use the provided VS Code task:

   - `CubeProg: Flash project (SWD)`

   Or run the following command:

   ```bash
   STM32_Programmer_CLI --connect port=swd --download build/Debug/CityEmergencyDispatch.elf -hardRst -rst --start
   ```

## Project Configuration

The project is configured using STM32CubeMX with the following setup:

### Microcontroller

- **Model**: STM32F756ZGTx
- **Package**: LQFP144
- **Clock Configuration**:
  - System Clock: 72 MHz (PLLCLK)
  - HSE: 8 MHz (External Oscillator)
  - USB Clock: 48 MHz

### Peripherals

- **FreeRTOS**:
  - CMSIS V2 interface enabled.
- **Timers**:
  - TIM1: Used as the time base for the system.
  - TIM2: Configured with a prescaler of 7199 and a period of 99.
- **Random Number Generator (RNG)**: Enabled.
- **USART3**:
  - Mode: Asynchronous.
  - TX: PD8, RX: PD9.

### Development Board

- **Board**: NUCLEO-F756ZG

### Toolchain

- **Compiler**: GNU Arm Embedded Toolchain (GCC)
- **Build System**: CMake with Ninja

This configuration ensures optimal performance for real-time embedded applications.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- STM32CubeMX and STM32 HAL for peripheral configuration.
- FreeRTOS for real-time task management.
- GNU Arm Embedded Toolchain for cross-compilation.
