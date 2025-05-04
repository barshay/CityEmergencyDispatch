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

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- STM32CubeMX and STM32 HAL for peripheral configuration.
- FreeRTOS for real-time task management.
- GNU Arm Embedded Toolchain for cross-compilation.
