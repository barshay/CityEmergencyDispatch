/**
 * @file fire_dept.c
 * @brief Implementation of the Fire Department module for the City Emergency Dispatch system.
 *
 * This module handles the initialization and task creation for the Fire Department resources.
 * It uses FreeRTOS for task management and queues for inter-task communication.
 *
 * @author shayb
 * @date April 17, 2025
 */

#include "fire_dept.h"
#include "project_config.h"
#include "logging.h"
#include "resource_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/**
 * @brief Queue handle for Fire Department tasks.
 *
 * This queue is used for communication between Fire Department tasks and other modules.
 */
QueueHandle_t xFireDeptQueue = NULL;

/**
 * @brief Task parameters for Fire Department tasks.
 *
 * This array stores the parameters for each Fire Department task instance.
 * The size of the array is determined by the maximum number of Fire Department resources
 * defined in the project configuration.
 */
static ResourceTaskParams_t fireDeptTaskParams[RESOURCES_FIRE_DEPT];

/**
 * @brief Task names for Fire Department tasks.
 *
 * This array stores the unique names for each Fire Department task instance.
 * The size of the array is determined by the maximum number of Fire Department resources
 * defined in the project configuration.
 */
static char fireDeptTaskNames[RESOURCES_FIRE_DEPT][configMAX_TASK_NAME_LEN];

/**
 * @brief Initializes the Fire Department module.
 *
 * This function initializes the Fire Department module by creating the required number of tasks
 * and setting up the necessary parameters. It ensures that the number of tasks does not exceed
 * the maximum configured limit.
 *
 * @param numUnits The number of Fire Department units to initialize.
 * @return pdPASS if all tasks are created successfully, pdFAIL otherwise.
 */
BaseType_t FireDept_Init(uint8_t numUnits)
{
    BaseType_t xReturned = pdPASS;
    uint8_t i;

    printf("Initializing Fire Department with %d units...\r\n", numUnits);
    if (numUnits > RESOURCES_FIRE_DEPT)
    {
        printf("Requested more fire dept units (%d) than configured max (%d)!\r\n", numUnits, RESOURCES_FIRE_DEPT);
        numUnits = RESOURCES_FIRE_DEPT;
        xReturned = pdFAIL;
    }
    if (xFireDeptQueue == NULL)
    {
        printf("Fire Department Queue handle is NULL during init!\r\n");
        return pdFAIL;
    }

    for (i = 0; i < numUnits; ++i)
    {
        // Prepare parameters for this specific task instance
        fireDeptTaskParams[i].xDepartmentQueue = xFireDeptQueue;

        // Create a unique name for this task instance
        snprintf(fireDeptTaskNames[i], configMAX_TASK_NAME_LEN, "FireDept_%d", i + 1);

        // Create the task
        xReturned = xTaskCreate(
            ResourceUnit_Task,          // Generic task function
            fireDeptTaskNames[i],       // Unique name for the instance
            TASK_STACK_SIZE_DEPARTMENT, // Stack size from config
            &fireDeptTaskParams[i],     // Pointer to parameters for this instance
            TASK_PRIO_DEPT_LOW,         // Priority from config
            NULL);                      // Task handle (optional)

        if (xReturned != pdPASS)
        {
            printf("Failed to create Fire Department Task %d!\r\n", i + 1);
            return pdFAIL; // Stop initialization on first failure
        }
    }
    printf("Fire Department task creation complete.\r\n");
    return xReturned;
}