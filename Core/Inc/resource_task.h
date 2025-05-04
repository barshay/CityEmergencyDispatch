/**
 * @file resource_task.h
 * @brief Header file for resource task management in the City Emergency Dispatch system.
 *
 * This file contains the definitions and prototypes for managing resource tasks,
 * including the task parameter structure, helper functions, and task function prototypes.
 *
 * @date April 23, 2025
 * @author shayb
 */

#ifndef INC_RESOURCE_TASK_H_
#define INC_RESOURCE_TASK_H_

#include "FreeRTOS.h" // For QueueHandle_t
#include "queue.h"

/**
 * @brief Task Parameter Structure for resource tasks.
 *
 * This structure defines the parameters required for resource tasks, including
 * the shared queue handle and the department type.
 */
typedef struct
{
    QueueHandle_t xDepartmentQueue; /**< Handle of the SHARED queue this task reads from. */
    uint8_t departmentType;         /**< Type of the department (e.g., police, fire, ambulance). */
} ResourceTaskParams_t;

/**
 * @brief Get a random task duration in ticks.
 *
 * This helper function generates a random duration in ticks for resource tasks.
 *
 * @return Random duration in ticks.
 */
uint32_t GetRandomTaskDurationTicks(void);

/**
 * @brief Generic Resource Unit Task function.
 *
 * This function serves as the entry point for resource unit tasks. It processes
 * events from the shared queue and performs actions based on the department type.
 *
 * @param pvParameters Pointer to the task parameters (ResourceTaskParams_t).
 */
void ResourceUnit_Task(void *pvParameters);

#endif /* INC_RESOURCE_TASK_H_ */
