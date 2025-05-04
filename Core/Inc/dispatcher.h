/*
 * @file dispatcher.h
 * @brief Header file for the dispatcher module.
 *
 * This file contains the declarations for the dispatcher module, including
 * initialization functions and module setup functions.
 *
 * @date April 17, 2025
 * @author shayb
 */

#ifndef INC_DISPATCHER_H_
#define INC_DISPATCHER_H_

#include "FreeRTOS.h"

/**
 * @brief Initializes the dispatcher module.
 *
 * This function sets up the dispatcher module, ensuring that all required
 * resources are properly initialized.
 *
 * @return BaseType_t Returns pdPASS if initialization is successful, otherwise pdFAIL.
 */
BaseType_t Dispatcher_Init(void);

/**
 * @brief Creates the necessary queues and semaphores for the dispatcher module.
 *
 * This function initializes all the FreeRTOS queues and semaphores required
 * for the dispatcher module to function correctly.
 */
void CreateQueuesAndSemaphores(void);

/**
 * @brief Initializes all modules managed by the dispatcher.
 *
 * This function ensures that all modules under the dispatcher are properly
 * initialized and ready for operation.
 */
void InitializeModules(void);

#endif /* INC_DISPATCHER_H_ */
