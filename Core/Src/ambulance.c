/**
 * @file ambulance.c
 * @brief Implementation of the Ambulance Department module.
 *
 * This module handles the initialization and task creation for the Ambulance Department.
 * It uses FreeRTOS for task management and queues for inter-task communication.
 *
 * @author shayb
 * @date April 17, 2025
 */

#include "ambulance.h"
#include "project_config.h"
#include "logging.h"
#include "resource_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/**
 * @brief Queue handle for the Ambulance Department.
 */
QueueHandle_t xAmbulanceQueue = NULL;

/**
 * @brief Task parameter storage for ambulance tasks.
 *
 * This array stores the parameters for each ambulance task instance.
 * The size is determined by the maximum number of ambulance units defined in the configuration.
 */
static ResourceTaskParams_t ambulanceTaskParams[RESOURCES_AMBULANCE];

/**
 * @brief Names for ambulance tasks.
 *
 * This array stores the unique names for each ambulance task instance.
 */
static char ambulanceTaskNames[RESOURCES_AMBULANCE][configMAX_TASK_NAME_LEN];

/**
 * @brief Initializes the Ambulance Department.
 *
 * This function initializes the Ambulance Department by creating tasks for each ambulance unit.
 * It ensures that the number of units does not exceed the configured maximum.
 *
 * @param numUnits The number of ambulance units to initialize.
 * @return pdPASS if all tasks are created successfully, pdFAIL otherwise.
 */
BaseType_t Ambulance_Init(uint8_t numUnits)
{
    BaseType_t xReturned = pdPASS;
    uint8_t i;

    printf("Initializing Ambulance Department with %d units...\r\n", numUnits);

    if (numUnits > RESOURCES_AMBULANCE)
    {
        printf("Requested more ambulance units (%d) than configured max (%d)!\r\n", numUnits, RESOURCES_AMBULANCE);
        numUnits = RESOURCES_AMBULANCE; // Clamp to max
        xReturned = pdFAIL;             // Indicate partial success/warning
    }

    if (xAmbulanceQueue == NULL)
    {
        printf("Ambulance Queue handle is NULL during init!\r\n");
        return pdFAIL; // Cannot proceed
    }

    for (i = 0; i < numUnits; ++i)
    {
        // Prepare parameters for this specific task instance
        ambulanceTaskParams[i].xDepartmentQueue = xAmbulanceQueue;

        // Create a unique name for this task instance
        snprintf(ambulanceTaskNames[i], configMAX_TASK_NAME_LEN, "Ambulance_%d", i + 1);

        // Create the task
        xReturned = xTaskCreate(
            ResourceUnit_Task,          // Generic task function
            ambulanceTaskNames[i],      // Unique name for the instance
            TASK_STACK_SIZE_DEPARTMENT, // Stack size from config
            &ambulanceTaskParams[i],    // Pointer to parameters for this instance
            TASK_PRIO_DEPT_LOW,         // Priority from config
            NULL);                      // Task handle (optional)

        if (xReturned != pdPASS)
        {
            printf("Failed to create Ambulance Task %d!\r\n", i + 1);
            return pdFAIL; // Stop initialization on first failure
        }
    }

    printf("Ambulance task creation complete.\r\n");
    return xReturned; // pdPASS if all tasks created ok (or pdFAIL if clamped/error)
}
