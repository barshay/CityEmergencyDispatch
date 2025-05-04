/**
 * @file event_generator.c
 * @brief Implementation of the Event Generator module.
 *
 * This file contains the implementation of the Event Generator module, which is responsible
 * for generating emergency events at random intervals and sending them to the dispatcher queue.
 * It uses a hardware timer (TIM2) and a random number generator (RNG) to determine event codes
 * and delays between events.
 *
 * @date April 17, 2025
 * @author shayb
 */

#include "event_generator.h" // Header for this module
#include "project_config.h"  // For event codes, timing, queue handle etc.
#include "logging.h"         // For logging macros

#include "main.h" // For HAL types and HAL function prototypes (TIM, RNG)
#include "FreeRTOS.h"
#include "queue.h" // For xQueueSendFromISR

// --- HAL Handles (Assumed defined globally in main.c or stm32f7xx_hal_msp.c) ---
extern TIM_HandleTypeDef htim2; // Timer used for periodic interrupt
extern RNG_HandleTypeDef hrng;  // Random Number Generator handle

// --- RTOS Handles (Assumed defined globally in main.c or queues.c) ---
extern QueueHandle_t xDispatcherQueue; // Queue to send events to

// --- Static Variables ---
// These maintain state across timer interrupt calls
static volatile uint32_t ticksUntilNextEvent = MIN_EVENT_DELAY_TICKS; // Start with min delay for first event
static volatile uint32_t currentTickCount = 0;

// --- Public Functions ---

/**
 * @brief Initializes the Event Generator module.
 *
 * Starts the hardware timer (TIM2) in interrupt mode. Assumes TIM2 and RNG peripherals
 * have already been initialized by CubeMX (MX_TIM2_Init, MX_RNG_Init).
 *
 * @retval pdPASS if the timer started successfully, pdFAIL otherwise.
 */
BaseType_t EventGenerator_Init(void)
{
    printf("Initializing Event Generator...\r\n");

    // Ensure necessary handles are valid before starting
    // (Ideally check hrng and xDispatcherQueue too if possible at this stage)
    if (htim2.Instance == NULL)
    {
        printf("TIM2 Handle not initialized before EventGenerator_Init!\r\n");
        return pdFAIL;
    }
    if (hrng.Instance == NULL)
    {
        printf("RNG Handle not initialized before EventGenerator_Init!\r\n");
        return pdFAIL;
    }
    if (xDispatcherQueue == NULL)
    {
        printf("Dispatcher Queue handle is NULL during EventGenerator_Init!\r\n");
        return pdFAIL;
    }

    // Reset state variables
    ticksUntilNextEvent = MIN_EVENT_DELAY_TICKS; // Generate first event quickly
    currentTickCount = 0;

    // Start the timer in Interrupt mode
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        printf("Failed to start TIM2 for Event Generator!\r\n");
        return pdFAIL; // Indicate failure
    }

    printf("Event Generator Timer (TIM2) started.\r\n");
    return pdPASS; // Indicate success
}

// --- HAL Callback Implementation ---

/**
 * @brief Period elapsed callback in non-blocking mode.
 *
 * This function is called when the TIM2 interrupt occurs. It generates an emergency event
 * and sends it to the dispatcher queue. The delay until the next event is determined
 * using the RNG peripheral.
 *
 * @param htim TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        HAL_IncTick();
    }
    // Check if the interrupt comes from the timer we are using (TIM2)
    else if (htim->Instance == TIM2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; // Must be initialised pdFALSE for FromISR calls
        uint32_t randomValue;                          // To store RNG output

        currentTickCount++;

        // Check if it's time to generate the event
        if (currentTickCount >= ticksUntilNextEvent)
        {
            // --- Event Generation ---
            EmergencyEvent_t eventToSend;

            // 1. Generate the event CODE (1, 2, or 3) using RNG
            if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
            {
                // Scale 32-bit random to 1-3
                eventToSend.eventCode = (randomValue % 3) + 1; // Assumes codes 1, 2, 3
            }
            else
            {
                eventToSend.eventCode = EVENT_CODE_POLICE; // Default to Police on RNG error
            }

            // --- Send Event to Queue ---
            // Check queue handle validity just in case, though it should be valid after Init
            if (xDispatcherQueue != NULL)
            {
                BaseType_t xQueueSendStatus = xQueueSendFromISR(xDispatcherQueue, &eventToSend, &xHigherPriorityTaskWoken);

                if (xQueueSendStatus != pdPASS)
                {
                    // Queue is full! Handle this scenario.
                    // Again, logging is hard from ISR. Increment counter? Set flag?
                    // For now, the event is lost if the dispatcher queue is full.
                }
            }

            // --- Determine Delay for Next Event ---
            if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
            {
                // Scale the 32-bit random number to our delay range in ticks
                ticksUntilNextEvent = (randomValue % DELAY_RANGE_TICKS) + MIN_EVENT_DELAY_TICKS;
            }
            else
            {
                // RNG Error - Handle it, maybe default delay or log error
                ticksUntilNextEvent = MIN_EVENT_DELAY_TICKS; // Default to min delay on error
            }

            // --- Reset Counter ---
            currentTickCount = 0;

            // --- Yield if Necessary ---
            // If xQueueSendFromISR unblocked a task with higher priority than the interrupted task, yield.
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
