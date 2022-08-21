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
#include <stdlib.h>
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

#ifndef VASQ_NO_LOGGING

#define VASQ_CONTEXT_DECL   const char *file_name, const char *function_name, unsigned int line_no
#define VASQ_CONTEXT_PARAMS __FILE__, __func__, __LINE__

typedef struct vasqLogger vasqLogger;

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
 * @brief Function type for processing the %x logger format token.
 *
 * @param user User-provided data.
 * @param idx A 0-up counter of which %x in the format string is being processed.
 * @param level The level of the message.
 * @param dst A pointer to the destination pointer as used in vasqIncSnprintf (see safe_snprintf.h).
 * @param remaining A pointer to the remaining number of characters in the destination buffer as used in
 * vasqIncSnprintf (see safe_snprintf.h).
 */
typedef void (*vasqLoggerDataProcessor)(void *user, size_t idx, vasqLogLevel_t level, char **dst,
                                        size_t *remaining);

/**
 * @brief Options passed to vasqLoggerCreate.
 */
typedef struct vasqLoggerOptions {
    vasqLoggerDataProcessor processor;  /// The processor to be used for %x format tokens.
    void *user;                         /// A pointer to user data to be passed to the processor.
    unsigned int flags;                 /// Bitwise-or-combined flags.
} vasqLoggerOptions;

#define VASQ_LOGGER_FLAG_DUP \
    0x00000001  /// Cause the provided file descriptor to be duplicated (and closed when the logger is
                /// freed).
#define VASQ_LOGGER_FLAG_CLOEXEC       0x00000002  /// Set FD_CLOEXEC on the file descriptor.
#define VASQ_LOGGER_FLAG_HEX_DUMP_INFO 0x00000004  /// Emit hex dumps at the INFO level.

/**
 * @brief Allocate and initialize a logger.
 *
 * @param fd The descriptor of the file to where log messages will be written.
 * @param level The maximum log level that this logger will handle.
 * @param format The format string for the log messages.
 * @param options A pointer to an options structure.  If options is NULL, then default options are used.
 * @param logger A pointer to the logger handle to be allocated.
 *
 * @return VASQ_RET_OK upon success and an error value otherwise.
 */
int
vasqLoggerCreate(int fd, vasqLogLevel_t level, const char *format, const vasqLoggerOptions *options,
                 vasqLogger **logger);

/**
 * @brief Free a logger.
 *
 * If the VASQ_LOGGER_FLAG_DUP flag was used to create the logger, then the file descriptor will be closed.
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
 * @brief Return a logger's user data.
 *
 * @param logger The logger handle.
 *
 * @return The logger's user data.
 */
void *
vasqLoggerUserData(const vasqLogger *logger);

/**
 * @brief Set the user data for a logger.
 *
 * @param logger The logger handle.
 * @param user_data The user data.
 */
void
vasqSetLoggerUserData(vasqLogger *logger, void *user);

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
vasqLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 6, 7)))
#endif
    ;

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
vasqRawLog(const vasqLogger *logger, const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
    ;

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
vasqMalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t size)
#ifdef __GNUC__
    __attribute__((deprecated))
#endif
    ;

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
vasqCalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t nmemb, size_t size)
#ifdef __GNUC__
    __attribute__((deprecated))
#endif
    ;

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
vasqRealloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, void *ptr, size_t size)
#ifdef __GNUC__
    __attribute__((deprecated))
#endif
    ;

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
vasqFork(const vasqLogger *logger, VASQ_CONTEXT_DECL)
#ifdef __GNUC__
    __attribute__((deprecated))
#endif
    ;

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
vasqExit(vasqLogger *logger, VASQ_CONTEXT_DECL, int value, bool quick)
#ifdef __GNUC__
    __attribute__((noreturn)) __attribute__((deprecated))
#endif
    ;

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

#else  // VASQ_NO_LOGGING

#define vasqLoggerCreate(...)                     VASQ_RET_OK
#define vasqLoggerFree(logger)                    NO_OP
#define vasqLoggerFd(logger)                      -1
#define vasqSetLoggerFormat(logger, format)       true
#define vasqLoggerLevel(logger)                   VASQ_LL_NONE
#define vasqsetLoggerLevel(logger, level)         NO_OP
#define vasqSetLoggerProcessor(logger, processor) NO_OP
#define vasqLoggerUserData(logger)                NULL
#define vasqSetLogerUserData(logger, user)        NO_OP

#define vasqLogStatement(...)  NO_OP
#define vasqVLogStatement(...) NO_OP
#define VASQ_ALWAYS(...)       NO_OP
#define VASQ_CRITICAL(...)     NO_OP
#define VASQ_ERROR(...)        NO_OP
#define VASQ_WARNING(...)      NO_OP
#define VASQ_INFO(...)         NO_OP
#define VASQ_DEBUG(...)        NO_OP
#define VASQ_PCRITICAL(...)    NO_OP
#define VASQ_PERROR(...)       NO_OP
#define VASQ_PWARNING(...)     NO_OP
#define vasqRawLog(...)        NO_OP
#define vasqVRawLog(...)       NO_OP
#define vasqHexDump(...)       NO_OP
#define VASQ_HEXDUMP(...)      NO_OP

#define vasqMalloc(logger, file, func, line, size)        malloc(size)
#define VASQ_MALLOC(logger, size)                         malloc(size)
#define vasqCalloc(logger, file, func, line, nmemb, size) calloc(nmemb, size)
#define VASQ_CALLOC(logger, nmemb, size)                  calloc(nmemb, size)
#define vasqRealloc(logger, file, func, line, ptr, size)  realloc(ptr, size)
#define VASQ_REALLOC(logger, ptr, size)                   realloc(ptr, size)
#define vasqFork(...)                                     fork()
#define VASQ_FORK()                                       fork()
#define vasqExit(logger, file, func, line, value, quick)  (((quick) ? _exit : exit)(value))
#define VASQ_EXIT(logger, value)                          exit(value)
#define VASQ_QUICK_EXIT(logger, value)                    _exit(value)

#endif  // VASQ_NO_LOGGING

#ifdef DEBUG

#ifdef VASQ_TEST_ASSERT

extern bool _vasq_abort_caught;
#define _VASQ_ABORT()              \
    do {                           \
        _vasq_abort_caught = true; \
    } while (0)
#define _VASQ_CLEAR_ABORT()         \
    do {                            \
        _vasq_abort_caught = false; \
    } while (0)

#else  // VASQ_TEST_ABORT

#define _VASQ_ABORT()       abort()
#define _VASQ_CLEAR_ABORT() NO_OP

#endif  // VASQ_TEST_ABORT

#define VASQ_ASSERT(logger, expr)                                 \
    do {                                                          \
        _VASQ_CLEAR_ABORT();                                      \
        if (!(expr)) {                                            \
            VASQ_CRITICAL(logger, "Assertion failed: %s", #expr); \
            _VASQ_ABORT();                                        \
        }                                                         \
    } while (0)

#else  // DEBUG

#define VASQ_ASSERT(logger, expr) NO_OP

#endif  // DEBUG

#endif  // VANILLA_SQUAD_LOGGER_H
