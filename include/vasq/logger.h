#ifndef VANILLA_SQUAD_LOGGER_H
#define VANILLA_SQUAD_LOGGER_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "definitions.h"

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

int
vasqLoggerCreate(int fd, vasqLogLevel_t level, const char *format, vasqLoggerDataProcessor processor,
                 void *user_data, vasqLogger **logger);

void
vasqLoggerFree(vasqLogger *logger);

int
vasqLoggerFd(const vasqLogger *logger);

bool
vasqSetLoggerFormat(vasqLogger *logger, const char *format);

vasqLogLevel_t
vasqLoggerLevel(const vasqLogger *logger);

vasqLogLevel_t
vasqLogLevel(const vasqLogger *logger) __attribute__((deprecated("Use vasqLoggerLevel.")));

void
vasqSetLoggerLevel(vasqLogger *logger, vasqLogLevel_t level);

void
vasqSetLogLevel(vasqLogger *logger, vasqLogLevel_t level)
    __attribute__((deprecated("Use vasqSetLoggerLevel.")));

void
vasqSetLoggerProcessor(vasqLogger *logger, vasqLoggerDataProcessor processor, void *user_data);

#define VASQ_CONTEXT_DECL   const char *file_name, const char *function_name, unsigned int line_no
#define VASQ_CONTEXT_PARAMS __FILE__, __func__, __LINE__

void
vasqLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format, ...);
#define VASQ_ALWAYS(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_ALWAYS, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)
#define VASQ_CRITICAL(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_CRITICAL, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)
#define VASQ_ERROR(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_ERROR, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)
#define VASQ_WARNING(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_WARNING, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)
#define VASQ_INFO(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_INFO, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)
#define VASQ_DEBUG(logger, format, ...) \
    vasqLogStatement(logger, VASQ_LL_DEBUG, VASQ_CONTEXT_PARAMS, format, ##__VA_ARGS__)

void
vasqVLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format,
                  va_list args);

void
vasqRawLog(const vasqLogger *logger, const char *format, ...);

void
vasqVRawLog(const vasqLogger *logger, const char *format, va_list args);

void
vasqHexDump(const vasqLogger *logger, VASQ_CONTEXT_DECL, const char *name, const void *data, size_t size);
#define VASQ_HEXDUMP(logger, name, data, size) vasqHexDump(logger, VASQ_CONTEXT_PARAMS, name, data, size)

void *
vasqMalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t size);
#define VASQ_MALLOC(logger, size) vasqMalloc(logger, VASQ_CONTEXT_PARAMS, size)

void *
vasqCalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t nmemb, size_t size);
#define VASQ_CALLOC(logger, nmemb, size) vasqCalloc(logger, VASQ_CONTEXT_PARAMS, nmemb, size)

void *
vasqRealloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, void *ptr, size_t size);
#define VASQ_REALLOC(logger, ptr, size) vasqRealloc(logger, VASQ_CONTEXT_PARAMS, ptr, size)

pid_t
vasqFork(const vasqLogger *logger, VASQ_CONTEXT_DECL);
#define VASQ_FORK(logger) vasqFork(logger, VASQ_DECL_PARAMS)

/*
    Wrapper around exit (if quick is false) or _exit (otherwise).  Frees the logger.
*/
void
vasqExit(vasqLogger *logger, VASQ_CONTEXT_DECL, int value, bool quick) __attribute__((noreturn));
#define VASQ_EXIT(logger, value)       vasqExit(logger, VASQ_CONTEXT_PARAMS, value, false)
#define VASQ_QUICK_EXIT(logger, value) vasqExit(logger, VASQ_CONTEXT_PARAMS, value, true)

#define VASQ_PCRITICAL(logger, function_name, errnum) \
    VASQ_CRITICAL(logger, "%s: %s", function_name, strerror(errnum))
#define VASQ_PERROR(logger, function_name, errnum) \
    VASQ_ERROR(logger, "%s: %s", function_name, strerror(errnum))
#define VASQ_PWARNING(logger, function_name, errnum) \
    VASQ_WARNING(logger, "%s: %s", function_name, strerror(errnum))

#endif  // VANILLA_SQUAD_LOGGER_H
