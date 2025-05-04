/**
 * @file fire_dept.h
 * @brief Header file for the Fire Department module.
 *
 * This file contains the declaration of the initialization function for the
 * Fire Department module, which is responsible for managing fire department
 * units in the City Emergency Dispatch system.
 *
 * @date May 4, 2025
 * @author shayb
 */

#ifndef INC_FIRE_DEPT_H_
#define INC_FIRE_DEPT_H_

#include "FreeRTOS.h"
#include <stdint.h>

/**
 * @brief Initializes the Fire Department module.
 *
 * This function initializes the Fire Department module with the specified
 * number of fire department units. It sets up the necessary resources and
 * tasks for managing the fire department operations.
 *
 * @param numUnits The number of fire department units to initialize.
 * @return pdPASS if initialization is successful, otherwise an error code.
 */
BaseType_t FireDept_Init(uint8_t numUnits);

#endif /* INC_FIRE_DEPT_H_ */
