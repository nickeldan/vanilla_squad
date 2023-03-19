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

#ifndef VASQ_NO_LOGGING

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
} vasqLogLevel;

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
 * @brief Function type outputting log messages.
 *
 * @param user  User-provided data.
 * @param text  The message to be printed.
 * @param size  The number of non-null characters in the message.
 */
typedef void
vasqHandlerFunc(void *user, const char *text, size_t size);

/**
 * @brief Function type for cleaning up a handler.
 *
 * @param user  User-provided data.
 */
typedef void
vasqHandlerCleanup(void *user);

/**
 * @brief Handles the outputting of log messages.
 */
typedef struct vasqLoggerHandler {
    vasqHandlerFunc *func;        /// Called whenever log messages are generated.
    vasqHandlerCleanup *cleanup;  /// Called when the logger is freed.
    void *user;                   /// User-provided data.
} vasqHandler;

/**
 * @brief Creates a handler from a file descriptor.
 *
 * @param fd            The file descriptor to be used.  The descriptor will be duplicated.
 * @param flags         Bitwise-or-combined flags.
 * @param handler[out]  The handler to be populated.
 *
 * @return              0 if successful.  Otherwise, -1 is returned and errno is set.
 *
 * @note Currently, the only available flag is VASQ_LOGGER_FLAG_CLOEXEC.
 */
int
vasqFdHandlerCreate(int fd, unsigned int flags, vasqHandler *handler);

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
typedef void
vasqDataProcessor(void *user, size_t idx, vasqLogLevel level, char **dst, size_t *remaining);

/**
 * @brief Options passed to vasqLoggerCreate.
 */
typedef struct vasqLoggerOptions {
    vasqDataProcessor *processor;  /// The processor to be used for %x format tokens.
    void *user;                    /// User-provided data.
    unsigned int flags;            /// Bitwise-or-combined flags.
} vasqLoggerOptions;

#define VASQ_LOGGER_FLAG_CLOEXEC       0x00000001  /// Set FD_CLOEXEC on a file descriptor.
#define VASQ_LOGGER_FLAG_HEX_DUMP_INFO 0x00000002  /// Emit hex dumps at the INFO level.

/**
 * @brief Allocate and initialize a logger.
 *
 * @param level     The maximum log level that this logger will handle.
 * @param format    The format string for the log messages.
 * @param handler   A pointer to the handler to be used.
 * @param options   A pointer to an options structure.  If options is NULL, then default options are used.
 *
 * @return          A pointer to the logger if successful. If not, then NULL is returned and errno is set.
 *
 * @note Currently, the only available flag for options is VASQ_LOGGER_FLAG_HEX_DUMP_INFO.
 */
vasqLogger *
vasqLoggerCreate(vasqLogLevel level, const char *format, const vasqHandler *handler,
                 const vasqLoggerOptions *options) VASQ_MALLOC;

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
 * @brief Return a logger's maximum log level.
 *
 * @param logger The logger handle.
 *
 * @return The maximum log level if logger is not NULL and VASQ_LL_NONE otherwise.
 */
vasqLogLevel
vasqLoggerLevel(const vasqLogger *logger);

/**
 * @brief Set the maximum log level for a logger.
 *
 * @param logger The logger handle.
 * @param level The new maximum log level.
 */
void
vasqSetLoggerLevel(vasqLogger *logger, vasqLogLevel level);

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
vasqLogStatement(const vasqLogger *logger, vasqLogLevel level, VASQ_CONTEXT_DECL, const char *format, ...)
    VASQ_FORMAT(6);

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
vasqVLogStatement(const vasqLogger *logger, vasqLogLevel level, VASQ_CONTEXT_DECL, const char *format,
                  va_list args) VASQ_NONNULL(6);

/**
 * @brief Write directly to a logger's descriptor.
 *
 * @param logger The logger handle.
 * @param format A format string (corresponding to vasqSafeSnprintf's syntax) for the the message.
 */
void
vasqRawLog(const vasqLogger *logger, const char *format, ...) VASQ_FORMAT(2);

/**
 * @brief Same as vasqRawLog but takes a va_list insead of variable arguments.
 */
void
vasqVRawLog(const vasqLogger *logger, const char *format, va_list args) VASQ_NONNULL(2);

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
vasqHexDump(const vasqLogger *logger, VASQ_CONTEXT_DECL, const char *name, const void *data, size_t size)
    VASQ_NONNULL(5, 6);

/**
 * @brief Wrap vasqHexDump by automatically supplying the file name, function name, and line number.
 */
#define VASQ_HEXDUMP(logger, name, data, size) vasqHexDump(logger, VASQ_CONTEXT_PARAMS, name, data, size)

#ifdef DEBUG

#ifdef VASQ_TEST_ASSERT

extern bool _vasq_abort_caught;
#define _VASQ_ABORT()              \
    do {                           \
        _vasq_abort_caught = true; \
    } while (0)

#else  // VASQ_TEST_ABORT

#define _VASQ_ABORT() abort()

#endif  // VASQ_TEST_ABORT

/**
 * @brief Verifies than an expression is true and, if not, logs a critical message and calls abort().
 * Resolves to a no op if the DEBUG preprocessor variable is not defined.
 */
#define VASQ_ASSERT(logger, expr)                                 \
    do {                                                          \
        if (!(expr)) {                                            \
            VASQ_CRITICAL(logger, "Assertion failed: %s", #expr); \
            _VASQ_ABORT();                                        \
        }                                                         \
    } while (0)

#else  // DEBUG

#define VASQ_ASSERT(logger, expr) NO_OP

#endif  // DEBUG

#else  // VASQ_NO_LOGGING

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
#define VASQ_ASSERT(...)       NO_OP

#endif  // VASQ_NO_LOGGING

#endif  // VANILLA_SQUAD_LOGGER_H
