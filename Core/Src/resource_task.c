/**
 * @file resource_task.c
 * @brief Implementation of resource task for handling emergency events.
 *
 * This file contains the implementation of the ResourceUnit_Task function,
 * which represents an individual resource unit in the City Emergency Dispatch system.
 * The task listens for events on a shared department queue, processes the events,
 * and simulates task execution time. It also includes utility functions such as
 * GetRandomTaskDurationTicks for generating random task durations.
 *
 * @date April 23, 2025
 * @author shayb
 */

/*
 * resource_task.c
 *
 *  Created on: Apr 23, 2025
 *      Author: shayb
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "project_config.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include "resource_task.h"

// --- Function Prototypes ---

// --- Task Parameter Structure ---

/**
 * @brief The main function for an individual Resource Unit task.
 *
 * Waits for an event on the shared department queue, simulates handling the call,
 * and then waits for the next event. The task itself represents the resource.
 *
 * @param pvParameters A pointer to a ResourceTaskParams_t structure.
 */
void ResourceUnit_Task(void *pvParameters)
{
    // Ensure parameter is not NULL before dereferencing
    configASSERT(pvParameters != NULL);

    ResourceTaskParams_t *params = (ResourceTaskParams_t *)pvParameters;
    QueueHandle_t xDepartmentQueue = params->xDepartmentQueue;
    const char *taskName = pcTaskGetName(NULL); // Get task name assigned during creation

    EmergencyEvent_t receivedEvent;
    BaseType_t xQueueStatus;
    uint32_t taskDurationTicks;

    LogInfo("%s Task started, listening on its queue.\r\n", taskName);

    while (1)
    {
        // 1. Wait indefinitely for an event on the SHARED department queue
        LogDebug("%s waiting for event...\r\n", taskName);
        xQueueStatus = xQueueReceive(xDepartmentQueue, &receivedEvent, portMAX_DELAY);

        if (xQueueStatus == pdPASS)
        {
            // --- Event Received ---
            // This specific task instance is now "busy"
            LogInfo("%s received event code %d. Processing...\r\n", taskName, receivedEvent.eventCode);

            // 2. Simulate task execution time
            taskDurationTicks = GetRandomTaskDurationTicks();
            LogDebug("%s task duration: %lu ticks (%lu ms)\r\n", taskName, taskDurationTicks, taskDurationTicks * EVENT_TIMER_TICK_MS);
            vTaskDelay(taskDurationTicks); // Simulate work being done

            LogInfo("%s finished processing call %d. Becoming idle.\r\n", taskName, receivedEvent.eventCode);
            // --- Event Processed, task becomes implicitly "idle" by looping back ---
        }
        else
        {
            // Error receiving from queue? Should not happen with portMAX_DELAY
            LogError("%s failed to receive from queue!\r\n", taskName);
            // Avoid busy-waiting in case of error, delay slightly
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

uint32_t GetRandomTaskDurationTicks(void)
{
// Check if configuration values are defined
#if defined(MIN_TASK_DURATION_TICKS) && defined(MAX_TASK_DURATION_TICKS)
// Ensure min is not greater than max
#if MIN_TASK_DURATION_TICKS > MAX_TASK_DURATION_TICKS
#error "MIN_TASK_DURATION_TICKS cannot be greater than MAX_TASK_DURATION_TICKS in project_config.h"
    return pdMS_TO_TICKS(500); // Fallback
#endif

    // Calculate the range of possible durations
    uint32_t range = MAX_TASK_DURATION_TICKS - MIN_TASK_DURATION_TICKS + 1;

    // --- Option 1: Use Hardware RNG (Preferred if available) ---

    uint32_t randomValue;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
    {
        // Scale the 32-bit random number to the desired range
        return (randomValue % range) + MIN_TASK_DURATION_TICKS;
    }
    else
    {
        // Fallback to software RNG or default if hardware RNG fails
        // LogError("Hardware RNG failed in GetRandomTaskDurationTicks, using rand().\r\n"); // Logging here might need mutex
        return (rand() % range) + MIN_TASK_DURATION_TICKS;
    }
#else
    // Default fallback if config values are not defined
    return pdMS_TO_TICKS(500); // Default to 500ms if config values missing
#endif
}
