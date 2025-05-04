/**
 * @file project_config.h
 * @brief Configuration file for the City Emergency Dispatch system.
 *
 * This file contains macro definitions, data structures, and configurations
 * for the City Emergency Dispatch system. It includes settings for event
 * generation, task priorities, queue configurations, and resource counts.
 *
 * @date April 17, 2025
 * @author shayb
 */

#ifndef INC_PROJECT_CONFIG_H_
#define INC_PROJECT_CONFIG_H_


#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "main.h"
#include "stm32f7xx_hal.h"

// --- Event Generation ---
/**
 * @def EVENT_TIMER_TICK_MS
 * @brief Timer interrupt frequency in milliseconds.
 */
#define EVENT_TIMER_TICK_MS 10 // Timer interrupt frequency

/**
 * @def MIN_EVENT_DELAY_MS
 * @brief Minimum delay between events in milliseconds.
 */
#define MIN_EVENT_DELAY_MS 1000 // Minimum delay between events (1 second)

/**
 * @def MAX_EVENT_DELAY_MS
 * @brief Maximum delay between events in milliseconds.
 */
#define MAX_EVENT_DELAY_MS 5000 // Maximum delay between events (5 seconds)

// Calculate ticks based on timer period
/**
 * @def MIN_EVENT_DELAY_TICKS
 * @brief Minimum delay between events in timer ticks.
 */
#define MIN_EVENT_DELAY_TICKS (MIN_EVENT_DELAY_MS / EVENT_TIMER_TICK_MS)

/**
 * @def MAX_EVENT_DELAY_TICKS
 * @brief Maximum delay between events in timer ticks.
 */
#define MAX_EVENT_DELAY_TICKS (MAX_EVENT_DELAY_MS / EVENT_TIMER_TICK_MS)

/**
 * @def DELAY_RANGE_TICKS
 * @brief Range of delay in timer ticks.
 */
#define DELAY_RANGE_TICKS (MAX_EVENT_DELAY_TICKS - MIN_EVENT_DELAY_TICKS + 1)

// --- Event Codes ---
/**
 * @def EVENT_CODE_POLICE
 * @brief Event code for Police.
 */
#define EVENT_CODE_POLICE 1 // Code for Police event

/**
 * @def EVENT_CODE_AMBULANCE
 * @brief Event code for Ambulance.
 */
#define EVENT_CODE_AMBULANCE 2 // Code for Ambulance event

/**
 * @def EVENT_CODE_FIRE_DEPT
 * @brief Event code for Fire Department.
 */
#define EVENT_CODE_FIRE_DEPT 3 // Code for Fire Department event

// --- Department Resource Counts ---
#define RESOURCES_AMBULANCE 4 // Number of available ambulances
#define RESOURCES_POLICE 3    // Number of available police cars
#define RESOURCES_FIRE_DEPT 2 // Number of available fire trucks

// --- Task Simulation Timing ---
// Example: Define min/max task execution time in *timer ticks*
// Adjust based on EVENT_TIMER_TICK_MS
#define MIN_TASK_DURATION_TICKS (200 / EVENT_TIMER_TICK_MS)  // Example: 200 ms
#define MAX_TASK_DURATION_TICKS (1500 / EVENT_TIMER_TICK_MS) // Example: 1500 ms

// --- Queue Configuration ---
#define DISPATCHER_QUEUE_LENGTH 20                          // Max number of events waiting for dispatcher
#define DISPATCHER_QUEUE_ITEM_SIZE sizeof(EmergencyEvent_t) // Size of one event message

// Define queue lengths for individual departments if they queue pending calls
#define POLICE_DEPT_QUEUE_LENGTH 10
#define AMBULANCE_DEPT_QUEUE_LENGTH 10
#define FIRE_DEPT_QUEUE_LENGTH 10
// ... other department queue lengths ...

//// --- FreeRTOS Task Configuration ---
// Priorities (higher number = higher priority)
#define TASK_PRIO_LOGGER (tskIDLE_PRIORITY + 1)
#define TASK_PRIO_EVENT_GENERATOR (tskIDLE_PRIORITY + 2) // Often handled in Timer ISR directly
#define TASK_PRIO_DEPT_LOW (tskIDLE_PRIORITY + 2)        // Base priority for departments
#define TASK_PRIO_DEPT_HIGH (tskIDLE_PRIORITY + 3)       // If prioritization is used
#define TASK_PRIO_DISPATCHER (tskIDLE_PRIORITY + 4)      // Dispatcher likely needs high priority

// Stack Sizes (in words, not bytes! Adjust based on usage)
#define TASK_STACK_SIZE_LOGGER 128 // May need more if using complex formatting (sprintf)
#define TASK_STACK_SIZE_DISPATCHER 256
#define TASK_STACK_SIZE_DEPARTMENT 256 // For Police, Ambulance, etc.

// --- Common Data Structures ---
typedef struct
{
    uint8_t eventCode;    // 1=Police, 2=Ambulance, 3=Fire Dept.
    TickType_t timeStamp; // Track when event was generated
    //    uint8_t priority;
} EmergencyEvent_t;

#endif /* INC_PROJECT_CONFIG_H_ */
