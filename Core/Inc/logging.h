/**
 * @file logging.h
 * @brief Header file for the logging module.
 *
 * This file contains the configuration, log levels, and public function prototypes
 * for the logging module. It also provides macros for logging messages at various
 * severity levels.
 *
 * @author shayb
 * @date April 17, 2025
 */

#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_

#include "project_config.h"
#include <stdint.h>

// --- Configuration ---

#define ENABLE_DEBUG_LOGGING 1 // Set to 0 to disable debug prints

// Define the minimum log level to compile. Messages below this level won't be included in the build.
// Set to LOG_LEVEL_INFO or LOG_LEVEL_WARN for release builds to reduce code size and overhead.
#define MIN_LOG_LEVEL LOG_LEVEL_INFO // Default to include all levels during development

//// Define the maximum size of a single formatted log message string
// Define queue length for logging task if used
#define LOGGER_QUEUE_LENGTH 50
#define LOGGER_MSG_MAX_SIZE 128                    // Max size of a log message string
#define LOGGER_QUEUE_ITEM_SIZE LOGGER_MSG_MAX_SIZE // Max size of a log message string

// --- Log Levels ---
/**
 * @enum LogLevel
 * @brief Enumeration of log levels for the logging module.
 *
 * Defines the severity levels for log messages. Each level has a specific purpose:
 * - LOG_LEVEL_DEBUG: Detailed debug information.
 * - LOG_LEVEL_INFO: General informational messages.
 * - LOG_LEVEL_WARN: Warnings about potential issues.
 * - LOG_LEVEL_ERROR: Error conditions.
 */
typedef enum
{
    LOG_LEVEL_DEBUG, // Detailed debug information (Compiled only if ENABLE_DEBUG_LOGGING=1 and MIN_LOG_LEVEL<=DEBUG)
    LOG_LEVEL_INFO,  // General informational messages (Compiled only if MIN_LOG_LEVEL<=INFO)
    LOG_LEVEL_WARN,  // Warnings about potential issues (Compiled only if MIN_LOG_LEVEL<=WARN)
    LOG_LEVEL_ERROR  // Error conditions (Compiled only if MIN_LOG_LEVEL<=ERROR)
} LogLevel;

// --- Public Function Prototypes ---

/**
 * @brief Core logging function (implementation in logging.c).
 * Formats the message and sends it to the logger queue.
 * Generally not called directly; use the LogX macros instead.
 *
 * @param level The severity level of the message.
 * @param file The source file name where the log originated (__FILE__).
 * @param line The line number where the log originated (__LINE__).
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
void Project_Log(LogLevel level, const char *file, int line, const char *format, ...);

/**
 * @brief Initializes the Logger Task (if implemented as a separate module).
 * Creates the logger queue and the logger task.
 * @retval pdPASS if successful, pdFAIL otherwise.
 */
BaseType_t Logger_Init(void);

// --- Logging Macros ---

/**
 * @def Log_Base
 * @brief Base logging macro.
 *
 * Checks the minimum compile-time log level and automatically includes file and
 * line number information. This macro is called by the level-specific macros
 * (LogDebug, LogInfo, etc.).
 *
 * @param level The severity level of the message.
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
#define Log_Base(level, format, ...)                                         \
    do                                                                       \
    {                                                                        \
        if ((level) >= MIN_LOG_LEVEL)                                        \
        {                                                                    \
            Project_Log((level), __FILE__, __LINE__, format, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)

/**
 * @def LogDebug
 * @brief Logs a DEBUG level message.
 *
 * Compiled only if ENABLE_DEBUG_LOGGING is 1 AND MIN_LOG_LEVEL is LOG_LEVEL_DEBUG.
 *
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
#if defined(ENABLE_DEBUG_LOGGING) && ENABLE_DEBUG_LOGGING == 1
#define LogDebug(format, ...) Log_Base(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#else
#define LogDebug(format, ...) ((void)0) // Compiles to nothing if debug logging is disabled
#endif

/**
 * @def LogInfo
 * @brief Logs an INFO level message.
 *
 * Compiled only if MIN_LOG_LEVEL is LOG_LEVEL_DEBUG or LOG_LEVEL_INFO.
 *
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
#define LogInfo(format, ...) Log_Base(LOG_LEVEL_INFO, format, ##__VA_ARGS__)

/**
 * @def LogWarn
 * @brief Logs a WARNING level message.
 *
 * Compiled only if MIN_LOG_LEVEL is LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, or LOG_LEVEL_WARN.
 *
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
#define LogWarn(format, ...) Log_Base(LOG_LEVEL_WARN, format, ##__VA_ARGS__)

/**
 * @def LogError
 * @brief Logs an ERROR level message.
 *
 * Always compiled (assuming MIN_LOG_LEVEL is not set higher than ERROR).
 *
 * @param format The printf-style format string.
 * @param ... Variable arguments for the format string.
 */
#define LogError(format, ...) Log_Base(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

#endif /* INC_LOGGING_H_ */
