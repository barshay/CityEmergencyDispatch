/**
 * @file logging.c
 * @brief Implementation of the queue-based logging utility.
 * The Logger Task uses a mutex to protect UART access.
 *
 * @author shayb
 * @date April 17, 2025
 */

#include "logging.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// FreeRTOS includes for queue, task, mutex, potentially task info/timestamps
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// --- External Handles

extern UART_HandleTypeDef huart3;    // UART peripheral used for logging
extern SemaphoreHandle_t xUartMutex; // Mutex created in CreateQueuesAndSemaphores()

// --- Module Handles ---
// Queue for log messages. Defined file-static as it's managed within this module.
static QueueHandle_t xLoggerQueue = NULL;

// --- Private Function Prototypes ---
static void Logger_Task(void *pvParameters);

// --- Public Functions ---

/**
 * @brief Initializes the Logger Task module.
 * Creates the logger queue and the logger task.
 * Assumes xUartMutex is created elsewhere (e.g., CreateQueuesAndSemaphores).
 * @retval pdPASS if successful, pdFAIL otherwise.
 */
BaseType_t Logger_Init(void)
{
    BaseType_t xStatus = pdPASS;

    // 1. Create the Logger Queue
    //    The queue will hold fixed-size character arrays.
    xLoggerQueue = xQueueCreate(LOGGER_QUEUE_LENGTH, LOGGER_QUEUE_ITEM_SIZE);
    if (xLoggerQueue == NULL)
    {
        // Cannot log this error easily as logging depends on the queue
        printf("FATAL ERROR: Failed to create Logger Queue!\r\n"); // Use raw printf if desperate
        return pdFAIL;
    }

    // 2. Create the Logger Task
    xStatus = xTaskCreate(
        Logger_Task,            // Function that implements the task.
        "Logger",               // Text name for the task.
        TASK_STACK_SIZE_LOGGER, // Stack size from config.
        NULL,                   // Parameter passed (not used).
        TASK_PRIO_LOGGER,       // Priority from config.
        NULL);                  // Task handle (optional).

    if (xStatus != pdPASS)
    {
        // Cannot log this easily
        printf("FATAL ERROR: Failed to create Logger Task!\r\n");
    }
    return xStatus;
}

/**
 * @brief Core logging function (implementation).
 * Formats the message and sends it to the logger queue.
 *
 * @param level The severity level of the message.
 * @param file The source file name where the log originated (__FILE__).
 * @param line The line number where the log originated (__LINE__).
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
void Project_Log(LogLevel level, const char *file, int line, const char *format, ...)
{
    char buffer[LOGGER_MSG_MAX_SIZE];     // Buffer for the final formatted message
    char tempBuffer[LOGGER_MSG_MAX_SIZE]; // Temporary buffer for user message part
    char levelPrefix[10];                 // Buffer for "[LEVEL] " prefix
    int currentLen = 0;                   // Current length written to the main buffer
    va_list args;
    BaseType_t xQueueSendStatus;

    // Check if logger queue has been created - avoid logging before init
    // It's also possible the scheduler hasn't started yet.
    if (xLoggerQueue == NULL || xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
    {
        return;
    }

    // 1. Determine level prefix string
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
        strncpy(levelPrefix, "[DEBUG] ", sizeof(levelPrefix));
        break;
    case LOG_LEVEL_INFO:
        strncpy(levelPrefix, "[INFO]  ", sizeof(levelPrefix));
        break;
    case LOG_LEVEL_WARN:
        strncpy(levelPrefix, "[WARN]  ", sizeof(levelPrefix));
        break;
    case LOG_LEVEL_ERROR:
        strncpy(levelPrefix, "[ERROR] ", sizeof(levelPrefix));
        break;
    default:
        strncpy(levelPrefix, "[???]   ", sizeof(levelPrefix));
        break;
    }
    levelPrefix[sizeof(levelPrefix) - 1] = '\0'; // Ensure null termination

    // 2. Format the user's message part into a temporary buffer
    va_start(args, format);
    vsnprintf(tempBuffer, sizeof(tempBuffer), format, args);
    va_end(args);
    tempBuffer[sizeof(tempBuffer) - 1] = '\0'; // Ensure null termination

    // 3. Assemble the final log message in 'buffer'
    // Add Level Prefix
    currentLen += snprintf(buffer + currentLen, sizeof(buffer) - currentLen, "%s", levelPrefix);

    // Add the formatted user message
    currentLen += snprintf(buffer + currentLen, sizeof(buffer) - currentLen, "%s", tempBuffer);
    // Add CRLF
    if (currentLen < (int)(sizeof(buffer) - 2))
    {
        buffer[currentLen++] = '\r';
        buffer[currentLen++] = '\n';
        buffer[currentLen] = '\0'; // Null terminate
    }
    else
    {
        // Ensure termination even if message was truncated
        buffer[sizeof(buffer) - 3] = '\r';
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
        currentLen = sizeof(buffer) - 1; // Adjust length
    }

    // 4. Send the formatted buffer to the Logger Queue
    // Use a small timeout (e.g., 0 or 10ms) to avoid blocking the calling task significantly
    // if the logger queue is full.
    xQueueSendStatus = xQueueSend(xLoggerQueue, buffer, pdMS_TO_TICKS(10)); // 10ms timeout
}

// --- Private Functions ---

/**
 * @brief Task responsible for processing log messages from the queue.
 * Waits for messages on xLoggerQueue, takes the UART mutex, transmits
 * the message via UART, and releases the mutex.
 *
 * @param pvParameters Unused.
 */
static void Logger_Task(void *pvParameters)
{
    // Buffer to receive messages from the queue. Must be same size as queue item size.
    char logBuffer[LOGGER_MSG_MAX_SIZE];
    BaseType_t xQueueReceiveStatus;
    const TickType_t xMutexWaitTicks = pdMS_TO_TICKS(100); // Max time to wait for mutex
    const uint32_t uartTxTimeoutMs = 100;                  // Timeout for HAL_UART_Transmit

    while (1)
    {
        // Wait indefinitely for a log message to arrive in the queue
        xQueueReceiveStatus = xQueueReceive(xLoggerQueue, logBuffer, portMAX_DELAY);

        if (xQueueReceiveStatus == pdPASS)
        {
            // Successfully received a log message.
            logBuffer[LOGGER_MSG_MAX_SIZE - 1] = '\0';

            // --- Take Mutex ---
            if (xUartMutex != NULL)
            {
                if (xSemaphoreTake(xUartMutex, xMutexWaitTicks) == pdTRUE)
                {
                    // --- Mutex Acquired ---

                    // Transmit the log message via UART
                    HAL_UART_Transmit(&huart3, (uint8_t *)logBuffer, strlen(logBuffer), uartTxTimeoutMs);

                    // --- Release Mutex ---
                    xSemaphoreGive(xUartMutex);
                    // --- Mutex Released ---
                }
            }
            // --- End Mutex Handling ---
        }
    }
}