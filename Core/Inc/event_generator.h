/**
 * @file event_generator.h
 * @brief Header file for the Event Generator module.
 *
 * This file contains the declaration of the initialization function for the
 * Event Generator module, which is responsible for generating events in the
 * system. It utilizes FreeRTOS for task management.
 *
 * @date Created on: Apr 17, 2025
 * @author shayb
 */

#ifndef INC_EVENT_GENERATOR_H_
#define INC_EVENT_GENERATOR_H_

#include "FreeRTOS.h"

/**
 * @brief Initializes the Event Generator module.
 *
 * This function sets up the necessary resources and tasks for the Event
 * Generator module to operate. It should be called during system
 * initialization.
 *
 * @return pdPASS if initialization is successful, otherwise an error code.
 */
BaseType_t EventGenerator_Init(void);

#endif /* INC_EVENT_GENERATOR_H_ */
