/**
 * @file config.h
 * @author Daniel Walker
 * @brief Allows the setting of various compile-term parameters.
 */
#ifndef VANILLA_SQUAD_CONFIG_H
#define VANILLA_SQUAD_CONFIG_H

/**
 * @brief Causes all logging logic to be removed during preprocessing.
 */
// #define VASQ_NO_LOGGING

/**
 * @brief The maximum number of bytes which will be written by a log statement.
 */
#define VASQ_LOGGING_LENGTH 1024

/**
 * @brief The maximum number of bytes displayed by a hex dump.  Any bytes past this limit are replaced by an
 * ellipsis.
 */
#define VASQ_HEXDUMP_SIZE 1024

/**
 * @brief The width of hex dumps.  Must divide into VASQ_HEXDUMP_SIZE.
 */
#define VASQ_HEXDUMP_WIDTH 16

/**
 * @brief The log level which displays the creation and exiting of processes.
 */
#define VASQ_LL_PROCESS VASQ_LL_DEBUG

/**
 * @brief The log level which displays the setting of a logger's log level.
 */
#define VASQ_LL_LEVEL_CHANGE VASQ_LL_DEBUG

/**
 * @brief Causes the PLACEHOLDER() macro to generate an error when used even if DEBUG or
 * VASQ_ALLOW_PLACEHOLDER are defined.
 */
// #define VASQ_REJECT_PLACEHOLDER

/** @brief If other macros are defined such that PLACEHOLDER() would normally resolve to a no op, this macro
 * causes the PLACEHOLDER() macro to generate a warning when used.
 */
// #define VASQ_WARN_PLACEHOLDER

#endif  // VANILLA_SQUAD_CONFIG_H
