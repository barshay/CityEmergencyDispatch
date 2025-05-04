/**
 * @file police.c
 * @brief Implementation of the Police Department tasks and initialization.
 *
 * This file contains the implementation of the Police Department's task creation
 * and initialization functions. It uses FreeRTOS for task management and includes
 * logging and resource management functionalities.
 *
 * @author shayb
 * @date April 17, 2025
 */

#include "police.h"
#include "project_config.h"
#include "logging.h"
#include "resource_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

QueueHandle_t xPoliceQueue = NULL;

// Task parameter storage for police tasks (file scope)
// Size needs to accommodate max possible units defined in config
static ResourceTaskParams_t policeTaskParams[RESOURCES_POLICE];
static char policeTaskNames[RESOURCES_POLICE][configMAX_TASK_NAME_LEN];

/**
 * @brief Initialization function for the Police Department.
 *
 * This function initializes the Police Department by creating the specified number
 * of police unit tasks. Each task is assigned a unique name and parameters.
 *
 * @param numUnits The number of police units to initialize.
 *                 If the requested number exceeds the configured maximum, it will
 *                 be clamped to the maximum value.
 *
 * @return pdPASS if all tasks are created successfully, otherwise pdFAIL.
 */
BaseType_t Police_Init(uint8_t numUnits)
{
    BaseType_t xReturned = pdPASS;
    uint8_t i;

    printf("Initializing Police Department with %d units...\r\n", numUnits);

    if (numUnits > RESOURCES_POLICE)
    {
        printf("Requested more police units (%d) than configured max (%d)!\r\n", numUnits, RESOURCES_POLICE);
        numUnits = RESOURCES_POLICE; // Clamp to max
        xReturned = pdFAIL;
    }
    if (xPoliceQueue == NULL)
    {
        printf("Police Queue handle is NULL during init!\r\n");
        return pdFAIL; // Cannot proceed
    }

    for (i = 0; i < numUnits; ++i)
    {
        // Prepare parameters for this specific task instance
        policeTaskParams[i].xDepartmentQueue = xPoliceQueue;

        // Create a unique name for this task instance
        snprintf(policeTaskNames[i], configMAX_TASK_NAME_LEN, "Police_%d", i + 1);

        // Create the task
        xReturned = xTaskCreate(
            ResourceUnit_Task,          // Generic task function
            policeTaskNames[i],         // Unique name for the instance
            TASK_STACK_SIZE_DEPARTMENT, // Stack size from config
            &policeTaskParams[i],       // Pointer to parameters for this instance
            TASK_PRIO_DEPT_LOW,         // Priority from config
            NULL);                      // Task handle (optional)

        if (xReturned != pdPASS)
        {
            printf("Failed to create Police Task %d!\r\n", i + 1);
            return pdFAIL; // Stop initialization on first failure
        }
    }
    printf("Police task creation complete.\r\n");
    return xReturned; // pdPASS if all tasks created ok (or pdFAIL if clamped/error)
}
