/**
 * @file police.h
 * @brief Header file for the Police module.
 *
 * This file contains the declaration of the initialization function for the Police module.
 * It is part of the City Emergency Dispatch system.
 *
 * @date April 17, 2025
 * @author shayb
 */

#ifndef INC_POLICE_H_
#define INC_POLICE_H_

#include "FreeRTOS.h"
#include <stdint.h>

/**
 * @brief Initializes the Police module.
 *
 * This function initializes the Police module with the specified number of units.
 *
 * @param numUnits The number of police units to initialize.
 * @return pdPASS if initialization is successful, otherwise an error code.
 */
BaseType_t Police_Init(uint8_t numUnits);

#endif /* INC_POLICE_H_ */
