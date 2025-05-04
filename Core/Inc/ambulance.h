/**
 * @file ambulance.h
 * @brief Header file for the Ambulance module.
 *
 * This file contains the function prototypes and necessary includes for the
 * Ambulance module, which is responsible for managing ambulance units in the
 * City Emergency Dispatch system.
 *
 * @date April 17, 2025
 * @author shayb
 */

#ifndef INC_AMBULANCE_H_
#define INC_AMBULANCE_H_

#include "FreeRTOS.h"
#include <stdint.h>

/**
 * @brief Initializes the Ambulance module.
 *
 * This function initializes the Ambulance module with the specified number of
 * ambulance units. It sets up the necessary resources and prepares the module
 * for operation.
 *
 * @param numUnits The number of ambulance units to initialize.
 * @return pdPASS if initialization is successful, otherwise an error code.
 */
BaseType_t Ambulance_Init(uint8_t numUnits);

#endif /* INC_AMBULANCE_H_ */
