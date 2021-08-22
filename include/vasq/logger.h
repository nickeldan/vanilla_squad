/**
 * @file logger.h
 * @author Daniel Walker
 * @brief Provides the logger interface.
 */
#ifndef VANILLA_SQUAD_LOGGER_H
#define VANILLA_SQUAD_LOGGER_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "definitions.h"

/**
 * @brief Logging levels
 */
typedef enum vasqLogLevel {
    VASQ_LL_NONE = -1,
    VASQ_LL_ALWAYS,
    VASQ_LL_CRITICAL,
    VASQ_LL_ERROR,
    VASQ_LL_WARNING,
    VASQ_LL_INFO,
    VASQ_LL_DEBUG,
} vasqLogLevel_t;

typedef struct vasqLogger vasqLogger;

typedef void (*vasqLoggerDataProcessor)(void *, size_t, vasqLogLevel_t, char **, size_t *);

/*
    Format symbols:

    %M : Message string
    %p : PID
    %T : Thread ID
    %L : Log level
    %_ : Log level name padding
    %u : Unix epoch time in seconds
    %t : "Pretty" timestamp
    %h : Hour
    %m : Minute
    %s : Second
    %F : File name
    %f : Function name
    %l : Line number
    %x : User data
    %% : Literal percent sign
*/

/**
 * @brief Cause the provided file descriptor to be duplicated (and closed when the logger is freed).
 */
#define VASQ_LOGGER_OPT_DUP 0x00000001

/**
 * @brief Set FD_CLOEXEC on the file descriptor.
 */
#define VASQ_LOGGER_OPT_CLOEXEC 0x00000002

/**
 * @brief Allocate and initialize a logger.
 *
 * @param fd The descriptor of the file to where log messages will be written.
 * @param level The maximum log level that this logger will handle.
 * @param format The format string for the log messages.
 * @param options Bitwise inclusive-or of any flags to be used for logger initialization.
 * @param processor If not NULL, the function to be called when encountering a %x in the format string.
 * @param user_data The pointer to be passed to the processor function.
 * @param logger A pointer to the logger handle to be allocated.
 *
 * @return VASQ_RET_OK upon success and an error value otherwise.
 */
int
vasqLoggerCreate(int fd, vasqLogLevel_t level, const char *format, unsigned int options,
                 vasqLoggerDataProcessor processor, void *user_data, vasqLogger **logger);

/**
 * @brief Free a logger.
 *
 * If the VASQ_LOGGER_OPT_DUP flag was used to create the logger, then the file descriptor will be closed.
 *
 * @param logger The logger handle to be freed.  This function does nothing if logger is NULL.
 */
void
vasqLoggerFree(vasqLogger *logger);

/**
 * @brief Return a logger's file descriptor.
 *
 * @param logger The logger handle.
 *
 * @return The logger's file descriptor if not NULL.  Otherwise, -1.
 */
int
vasqLoggerFd(const vasqLogger *logger);

/**
 * @brief Set a new format string for a logger.
 *
 * @param logger The logger handle.
 * @param format The new format string.
 *
 * @return true if the format string is valid and false otherwise.
 */
bool
vasqSetLoggerFormat(vasqLogger *logger, const char *format);

/**
 * @brief Return a logger's maximum log level.
 *
 * @param logger The logger handle.
 *
 * @return The maximum log level if logger is not NULL and VASQ_LL_NONE otherwise.
 */
vasqLogLevel_t
vasqLoggerLevel(const vasqLogger *logger);

/**
 * @brief Set the maximum log level for a logger.
 *
 * @param logger The logger handle.
 * @param level The new maximum log level.
 */
void
vasqSetLoggerLevel(vasqLogger *logger, vasqLogLevel_t level);

/**
 * @brief Set the processor for a logger.
 *
 * @param logger The logger handle.
 * @param processor The processor to be used.
 */
void
vasqSetLoggerProcessor(vasqLogger *logger, vasqLoggerDataProcessor processor);

/**
 * Return a logger's user data.
 *
 * @param logger The logger handle.
 *
 * @return The logger's user data.
 */
void *
vasqLoggerUseData(const vasqLogger *logger);

/**
 * @brief Set the user data for a logger.
 *
 * @param logger The logger handle.
 * @param user_data The user data.
 */
void
vasqSetLoggerUserData(vasqLogger *logger, void *user_data);

#define VASQ_CONTEXT_DECL   const char *file_name, const char *function_name, unsigned int line_no
#define VASQ_CONTEXT_PARAMS __FILE__, __func__, __LINE__

/**
 * @brief Emit a logging message.
 *
 * This function has no effect if either logger is NULL or the logger's maximum log level is VASQ_LL_NONE.
 *
 * @param logger The logger handle.
 * @param level The level of the message.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param format A format string (corresponding to vasqSafeSnprintf's syntax) for the the message.
 */
void
vasqLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format, ...);

/**
 * @brief Emit a message at the ALWAYS level.
 */
#define VASQ_ALWAYS(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_ALWAYS, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Emit a message at the CRITICAL level.
 */
#define VASQ_CRITICAL(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_CRITICAL, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Emit a message at the ERROR level.
 */
#define VASQ_ERROR(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_ERROR, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Emit a message at the WARNING level.
 */
#define VASQ_WARNING(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_WARNING, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Emit a message at the INFO level.
 */
#define VASQ_INFO(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_INFO, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Emit a message at the DEBUG level.
 */
#define VASQ_DEBUG(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_DEBUG, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

/**
 * @brief Same as vasqLogStatement but takes a va_list instead of variable arguments.
 */
void
vasqVLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format,
                  va_list args);

/**
 * @brief Write directly to a logger's descriptor.
 *
 * @param logger The logger handle.
 * @param format A format string (corresponding to vasqSafeSnprintf's syntax) for the the message.
 */
void
vasqRawLog(const vasqLogger *logger, const char *format, ...);

/**
 * @brief Same as vasqRawLog but takes a va_list insead of variable arguments.
 */
void
vasqVRawLog(const vasqLogger *logger, const char *format, va_list args);

/**
 * @brief Display a hex dump of a section of memory.  The dump appears at the DEBUG level.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param name A description of the data.
 * @param data A pointer to the data.
 * @param size The number of bytes to display.
 */
void
vasqHexDump(const vasqLogger *logger, VASQ_CONTEXT_DECL, const char *name, const void *data, size_t size);

/**
 * @brief Wrap vasqHexDump by automatically supplying the file name, function name, and line number.
 */
#define VASQ_HEXDUMP(logger, name, data, size) vasqHexDump(logger, VASQ_CONTEXT_PARAMS, name, data, size)

/**
 * @brief Wrap malloc.
 *
 * Emits a log message at the ERROR level if the allocation fails.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param size The number of bytes to allocate.
 *
 * @return The same return value as malloc.
 */
void *
vasqMalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t size);

/**
 * @brief Wrap vasqMalloc by automatically supplying the file name, function name, and line number.
 */
#define VASQ_MALLOC(logger, size) vasqMalloc(logger, VASQ_CONTEXT_PARAMS, size)

/**
 * @brief Wrap calloc.
 *
 * Emits a log message at the ERROR level if the allocation fails.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param nmemb The number of items to allocate.
 * @param size The size of each allocated item.
 *
 * @return The same return value as calloc.
 */
void *
vasqCalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t nmemb, size_t size);

/**
 * @brief Wrap vasqCalloc by automatically supplying the file name, function name, and line number.
 */
#define VASQ_CALLOC(logger, nmemb, size) vasqCalloc(logger, VASQ_CONTEXT_PARAMS, nmemb, size)

/**
 * @brief Wrap realloc.
 *
 * Emits a log message at the ERROR level if the allocation fails.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param ptr A pointer to the original data.
 * @param size The size of the new data.
 *
 * @return The same return value as realloc.
 */
void *
vasqRealloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, void *ptr, size_t size);

/**
 * @brief Wrap vasqRealloc by automatically supplying the file name, function name, and line number.
 */
#define VASQ_REALLOC(logger, ptr, size) vasqRealloc(logger, VASQ_CONTEXT_PARAMS, ptr, size)

/**
 * @brief Wrap fork.
 *
 * Emits a log message at the ERROR level if fork fails.  Otherwise, the child process emits a log message
 * at the VASQ_LL_PROCESS (see config.h) level.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 *
 * @return The same return value as fork.
 */
pid_t
vasqFork(const vasqLogger *logger, VASQ_CONTEXT_DECL);

/**
 * @brief Wrap vasqFork by automatically supplying the file name, function name, and line number.
 */
#define VASQ_FORK(logger) vasqFork(logger, VASQ_DECL_PARAMS)

/**
 * @brief Wrap exit or _exit.
 *
 * Emits a log message at the VASQ_LL_PROCESS (see config.h) level.  After that, vasqLoggerFree is called on
 * the logger handle.
 *
 * @param logger The logger handle.
 * @param file_name The name of the file where the message originated.
 * @param function_name The name of the function where the message originated.
 * @param line_no The line number where the message originated.
 * @param value The value passed to exit/_exit.
 * @param quick If true, _exit is called.  Otherwise, exit is called.
 */
void
vasqExit(vasqLogger *logger, VASQ_CONTEXT_DECL, int value, bool quick) __attribute__((noreturn));

/**
 * @brief Wrap vasqExit by automatically supplying the file name, function name, and line number and setting
 * quick to false.
 */
#define VASQ_EXIT(logger, value) vasqExit(logger, VASQ_CONTEXT_PARAMS, value, false)

/**
 * @brief Wrap vasqExit by automatically supplying the file name, function name, and line number and setting
 * quick to true.
 */
#define VASQ_QUICK_EXIT(logger, value) vasqExit(logger, VASQ_CONTEXT_PARAMS, value, true)

/**
 * @brief Logging equivalent of the perror function at the CRITICAL level.
 *
 * @param logger The logger handle.
 * @param msg The argument that would normally be passed to perror.
 * @param errnum The errno value.
 *
 * @warning msg must be a string literal and not a variable.
 */
#define VASQ_PCRITICAL(logger, msg, errnum) VASQ_CRITICAL(logger, msg ": %s", strerror(errnum))
/**
 * @brief Logging equivalent of the perror function at the ERROR level.
 *
 * @param logger The logger handle.
 * @param msg The argument that would normally be passed to perror.
 * @param errnum The errno value.
 *
 * @warning msg must be a string literal and not a variable.
 */
#define VASQ_PERROR(logger, msg, errnum) VASQ_ERROR(logger, msg ": %s", strerror(errnum))

/**
 * @brief Logging equivalent of the perror function at the WARNING level.
 *
 * @param logger The logger handle.
 * @param msg The argument that would normally be passed to perror.
 * @param errnum The errno value.
 *
 * @warning msg must be a string literal and not a variable.
 */
#define VASQ_PWARNING(logger, msg, errnum) VASQ_WARNING(logger, msg ": %s", strerror(errnum))

#endif  // VANILLA_SQUAD_LOGGER_H
