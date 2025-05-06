/**
 * @file dispatcher.c
 * @brief Implementation of the Dispatcher module for handling emergency events.
 *
 * This file contains the implementation of the Dispatcher module, which is responsible
 * for receiving emergency events and dispatching them to the appropriate department queues.
 * It also initializes the required FreeRTOS queues and semaphores for inter-task communication.
 *
 * @date April 17, 2025
 * @author shayb
 */
#include "dispatcher.h"
#include "project_config.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h" // For mutex creation
#include "logging.h"

#include "event_generator.h"
#include "ambulance.h"
#include "police.h"
#include "fire_dept.h"

QueueHandle_t xDispatcherQueue = NULL;

extern QueueHandle_t xPoliceQueue;    // Queue for Police department task
extern QueueHandle_t xAmbulanceQueue; // Queue for Ambulance department task
extern QueueHandle_t xFireDeptQueue;  // Queue for Fire Dept department task
extern SemaphoreHandle_t xUartMutex;

static void Dispatcher_Task(void *pvParameters);

/**
 * @brief Error handler for initialization failures.
 *
 * Logs the context of the error and calls the main HAL Error_Handler.
 *
 * @param context A string describing the context where the error occurred.
 */
static void ErrorHandler(const char *context)
{
    // Log the context where the error occurred (e.g., "DispatcherQ", "AmbulanceQ")
    LogError("Initialization Error in %s!\r\n", context);

    // Call the main HAL Error_Handler (which is likely defined in main.c)
    // to halt the system or perform other critical error actions.
    Error_Handler();
}

void CreateQueuesAndSemaphores(void)
{
    printf("Creating Queues and Semaphores...\r\n"); // Logging might not work reliably yet

    // Create Dispatcher Queue
    xDispatcherQueue = xQueueCreate(DISPATCHER_QUEUE_LENGTH, DISPATCHER_QUEUE_ITEM_SIZE);
    if (xDispatcherQueue == NULL)
    {
        ErrorHandler("DispatcherQ");
    }

    // Create Shared Department Queues
    xAmbulanceQueue = xQueueCreate(AMBULANCE_DEPT_QUEUE_LENGTH, DISPATCHER_QUEUE_ITEM_SIZE);
    if (xAmbulanceQueue == NULL)
    {
        ErrorHandler("AmbulanceQ");
    }

    xPoliceQueue = xQueueCreate(POLICE_DEPT_QUEUE_LENGTH, DISPATCHER_QUEUE_ITEM_SIZE);
    if (xPoliceQueue == NULL)
    {
        ErrorHandler("PoliceQ");
    }

    xFireDeptQueue = xQueueCreate(FIRE_DEPT_QUEUE_LENGTH, DISPATCHER_QUEUE_ITEM_SIZE);
    if (xFireDeptQueue == NULL)
    {
        ErrorHandler("FireDeptQ");
    }

    // Create Corona Queue if needed

    // *** Create UART Mutex (Needed by Logger Task) ***
    xUartMutex = xSemaphoreCreateMutex();
    if (xUartMutex == NULL)
    {
        // Cannot log this error yet!
        printf("FATAL ERROR: Failed to create UART Mutex!\r\n");
        Error_Handler(); // Use HAL Error Handler
    }

    printf("Queues and Mutex created successfully.\r\n"); // Logging should work after mutex creation if called later
}

/**
 * @brief Calls the initialization functions for each project module.
 * These functions typically create the tasks associated with the module.
 * (Implementation moved from main.c)
 */
void InitializeModules(void)
{
    // Logging should be safe now if CreateQueuesAndSemaphores was called first
    printf("Initializing project modules...\r\n");

    // *** Initialize Logger Task FIRST (or early) ***
    // It needs to be ready before other tasks start logging heavily.
    if (Logger_Init() != pdPASS)
    {
        printf("Logger Initialization failed!\r\n");
        // Decide if this is fatal
        Error_Handler();
    }
    else
    {
        printf("Logger Initialized.\r\n");
    }

    // Initialize Event Generator
    if (EventGenerator_Init() != pdPASS)
    {
        printf("Event Generator Initialization failed!\r\n");
        Error_Handler();
    }
    else
    {
        printf("Event Generator Initialized.\r\n");
    }

    // Initialize Dispatcher Task
    if (Dispatcher_Init() != pdPASS)
    {
        printf("Dispatcher Task Creation failed!\r\n");
        Error_Handler();
    }
    else
    {
        printf("Dispatcher Task Initialized.\r\n");
    }

    // Initialize Police Department
    if (Police_Init(RESOURCES_POLICE) != pdPASS)
    {
        printf("Police Department Initialization failed!\r\n");
    }
    else
    {
        printf("Police Department Initialized.\r\n");
    }

    // Initialize Ambulance Department
    if (Ambulance_Init(RESOURCES_AMBULANCE) != pdPASS)
    {
        printf("Ambulance Department Initialization failed!\r\n");
    }
    else
    {
        printf("Ambulance Department Initialized.\r\n");
    }

    // Initialize Fire Department
    if (FireDept_Init(RESOURCES_FIRE_DEPT) != pdPASS)
    {
        printf("Fire Department Initialization failed!\r\n");
    }
    else
    {
        printf("Fire Department Initialized.\r\n");
    }

    // Initialize other modules (Corona?)

    printf("All project modules initialized.\r\n");
}

BaseType_t Dispatcher_Init(void)
{
    BaseType_t xReturned;

    printf("Initializing Dispatcher...\r\n");

    // Create the Dispatcher Task
    xReturned = xTaskCreate(
        Dispatcher_Task,            // Function that implements the task.
        "Dispatcher",               // Text name for the task (for debugging).
        TASK_STACK_SIZE_DISPATCHER, // Stack size from project_config.h
        NULL,                       // Parameter passed into the task (not used).
        TASK_PRIO_DISPATCHER,       // Priority from project_config.h
        NULL);                      // Used to pass out the created task's handle (optional).

    if (xReturned != pdPASS)
    {
        printf("Failed to create Dispatcher Task\r\n");
    }
    else
    {
        printf("Dispatcher Task created.\r\n");
    }
    return xReturned;
}

static void Dispatcher_Task(void *pvParameters)
{
    EmergencyEvent_t receivedEvent; // Structure to hold the received event
    BaseType_t xStatus;
    const TickType_t xSendTicksToWait = pdMS_TO_TICKS(10); // Small timeout for sending
    UBaseType_t primaryQueueMsgs;
    UBaseType_t alternativeQueueMsgs;

    LogInfo("Dispatcher Task running.\r\n");

    while (1)
    {
        // Wait indefinitely for an event to arrive from the event generator
        xStatus = xQueueReceive(xDispatcherQueue, &receivedEvent, portMAX_DELAY);

        if (xStatus == pdPASS)
        {
            // Successfully received an event
            LogDebug("Dispatcher received event code %d\r\n", receivedEvent.eventCode);

            // Determine primary target department and queue, and alternative if applicable
            QueueHandle_t xPrimaryQueue = NULL;
            QueueHandle_t xAlternativeQueue = NULL;
            const char *primaryDeptName = "Unknown";
            const char *alternativeDeptName = "None";
            BaseType_t attemptRedirect = pdFALSE;

            // --- Define Dispatching and Redirection Rules ---
            switch (receivedEvent.eventCode)
            {
            case EVENT_CODE_POLICE:
                xPrimaryQueue = xPoliceQueue;
                primaryDeptName = "Police";
                // No alternative defined for Police
                break;
            case EVENT_CODE_AMBULANCE:
                xPrimaryQueue = xAmbulanceQueue;
                primaryDeptName = "Ambulance";
                // Define Police as alternative
                xAlternativeQueue = xPoliceQueue;
                alternativeDeptName = "Police";
                attemptRedirect = pdTRUE; // Allow redirection for Ambulance calls
                break;
            case EVENT_CODE_FIRE_DEPT:
                xPrimaryQueue = xFireDeptQueue;
                primaryDeptName = "FireDept";
                // No alternative defined for Fire Dept
                break;
            default:
                LogWarn("Dispatcher received unknown event code: %d\r\n", receivedEvent.eventCode);
                continue; // Skip processing this unknown event
            }
            // --- End of Rules ---

            // Ensure primary queue is valid before proceeding
            if (xPrimaryQueue == NULL)
            {
                LogWarn("No primary queue defined for event code %d\r\n", receivedEvent.eventCode);
                continue;
            }

            // Check primary department queue status (check for available space)
            UBaseType_t primaryQueueSpaces = uxQueueSpacesAvailable(xPrimaryQueue);
            LogDebug("Primary Dept [%s] Queue Check: %lu spaces available.\r\n", primaryDeptName, primaryQueueSpaces);

            // Decide whether to send to primary or attempt redirect
            if (primaryQueueSpaces > 0 || !attemptRedirect || xAlternativeQueue == NULL)
            {
                // Send to primary if:
                // 1. Primary queue has space available, OR
                // 2. Redirection is not allowed for this event type, OR
                // 3. No alternative queue is defined.
                LogDebug("Dispatching event %d to Primary [%s].\r\n", receivedEvent.eventCode, primaryDeptName);
                xStatus = xQueueSend(xPrimaryQueue, &receivedEvent, xSendTicksToWait);
                if (xStatus != pdPASS)
                {
                    LogError("Failed to send event %d to Primary Queue [%s] (Timeout?)\r\n", receivedEvent.eventCode, primaryDeptName);
                }
            }
            else
            {
                // Primary queue is full AND redirection is allowed AND alternative exists
                LogWarn("Primary Dept [%s] is full. Checking Alternative [%s]...\r\n", primaryDeptName, alternativeDeptName);

                // Check alternative department queue status (check for available space)
                UBaseType_t alternativeQueueSpaces = uxQueueSpacesAvailable(xAlternativeQueue);
                LogDebug("Alternative Dept [%s] Queue Check: %lu spaces available.\r\n", alternativeDeptName, alternativeQueueSpaces);

                if (alternativeQueueSpaces > 0)
                {
                    // Alternative queue has space, redirect the call
                    LogInfo("Redirecting event %d from [%s] to Alternative [%s].\r\n", receivedEvent.eventCode, primaryDeptName, alternativeDeptName);

                    xStatus = xQueueSend(xAlternativeQueue, &receivedEvent, xSendTicksToWait);
                    if (xStatus != pdPASS)
                    {
                        LogError("Failed to send event %d to Alternative Queue [%s] (Timeout?)\r\n", receivedEvent.eventCode, alternativeDeptName);
                        // Fallback: Try sending to primary queue anyway if redirect fails
                        LogWarn("Redirect failed, sending event %d back to Primary Queue [%s] to wait.\r\n", receivedEvent.eventCode, primaryDeptName);
                        xStatus = xQueueSend(xPrimaryQueue, &receivedEvent, xSendTicksToWait);
                        if (xStatus != pdPASS)
                        {
                            LogError("Fallback send to Primary Queue [%s] also failed! Event %d lost.\r\n", primaryDeptName, receivedEvent.eventCode);
                        }
                    }
                }
                else
                {
                    // Alternative queue is full, send to original primary queue
                    LogWarn("Alternative Dept [%s] is full. Sending event %d to Primary Queue [%s] to wait.\r\n", alternativeDeptName, receivedEvent.eventCode, primaryDeptName);
                    xStatus = xQueueSend(xPrimaryQueue, &receivedEvent, xSendTicksToWait);
                    if (xStatus != pdPASS)
                    {
                        LogError("Failed to send event %d to Primary Queue [%s] even when busy (Timeout?) Event lost.\r\n", receivedEvent.eventCode, primaryDeptName);
                    }
                }
            }
        }
        // No else needed for xQueueReceive error with portMAX_DELAY,
        // unless the queue handle itself is invalid.
    }
}
