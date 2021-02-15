#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "vasq/logger.h"
#include "vasq/safe_snprintf.h"

#define MAX_HEXDUMP_SIZE 1024
#define HEXDUMP_WIDTH    16
#if MAX_HEXDUMP_SIZE % HEXDUMP_WIDTH != 0
#error "MAX_HEXDUMP_SIZE must be a multiple of HEXDUMP_WIDTH."
#endif

struct vasqLogger {
    const char *format;
    vasqLoggerDataProcessor processor;
    void *user_data;
    int fd;
    vasqLogLevel_t level;
};

static bool
validLogFormat(const char *format)
{
    unsigned int m_occurrences = 0;

    for (size_t k = 0; format[k]; k++) {
        if (format[k] == '%') {
            switch (format[++k]) {
            case 'M':
                if (++m_occurrences == 2) {
                    return false;
                }
            /* FALLTHROUGH */
            case 'p':
            case 'L':
            case '_':
            case 'u':
            case 't':
            case 'h':
            case 'm':
            case 's':
            case 'F':
            case 'f':
            case 'l':
            case 'x':
            case '%': break;

            default: return false;
            }
        }
    }

    return true;
}

static bool
safeIsprint(char c)
{
    return c >= ' ' && c <= '~';
}

static const char *
logLevelName(vasqLogLevel_t level)
{
    switch (level) {
    case VASQ_LL_ALWAYS: return "ALWAYS";
    case VASQ_LL_CRITICAL: return "CRITICAL";
    case VASQ_LL_ERROR: return "ERROR";
    case VASQ_LL_WARNING: return "WARNING";
    case VASQ_LL_INFO: return "INFO";
    case VASQ_LL_DEBUG: return "DEBUG";
    default: return "INVALID";
    }
}

static unsigned int
logLevelNamePadding(vasqLogLevel_t level)
{
    switch (level) {
    case VASQ_LL_ALWAYS: return 2;
    case VASQ_LL_CRITICAL: return 0;
    case VASQ_LL_ERROR: return 3;
    case VASQ_LL_WARNING: return 1;
    case VASQ_LL_INFO: return 4;
    case VASQ_LL_DEBUG: return 3;
    default: return 0;
    }
}

int
vasqLoggerCreate(int fd, vasqLogLevel_t level, const char *format, vasqLoggerDataProcessor processor,
                 void *user_data, vasqLogger **logger)
{
    if (fd < 0 || !logger || !format || !validLogFormat(format)) {
        return VASQ_RET_USAGE;
    }

    *logger = malloc(sizeof(**logger));
    if (!*logger) {
        return VASQ_RET_OUT_OF_MEMORY;
    }

    (*logger)->fd = fd;
    (*logger)->level = level;
    (*logger)->format = format;
    (*logger)->processor = processor;
    (*logger)->user_data = user_data;

    return VASQ_RET_OK;
}

void
vasqLoggerFree(vasqLogger *logger)
{
    if (logger) {
        free(logger);
    }
}

bool
vasqSetLoggerFormat(vasqLogger *logger, const char *format)
{
    if (logger && format && validLogFormat(format)) {
        logger->format = format;
        return true;
    }
    else {
        return false;
    }
}

void
vasqSetLogLevel(vasqLogger *logger, vasqLogLevel_t level)
{
    if (logger) {
        logger->level = level;
        VASQ_DEBUG(logger, "Log level set to %s", logLevelName(level));
    }
}

void
vasqSetLoggerProcessor(vasqLogger *logger, vasqLoggerDataProcessor processor, void *user_data)
{
    if (logger) {
        logger->processor = processor;
        logger->user_data = user_data;
    }
}

void
vasqLogStatement(const vasqLogger *logger, vasqLogLevel_t level, const char *file_name,
                 const char *function_name, unsigned int line_no, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVLogStatement(logger, level, file_name, function_name, line_no, format, args);
    va_end(args);
}

void
vasqVLogStatement(const vasqLogger *logger, vasqLogLevel_t level, const char *file_name,
                  const char *function_name, unsigned int line_no, const char *format, va_list args)
{
    char output[1024];
    char *dst = output;
    size_t remaining = sizeof(output);
    time_t now;
    struct tm now_fields;

    if (!logger || level > logger->level || logger->level <= VASQ_LL_RAWONLY) {
        return;
    }

    now = time(NULL);
    gmtime_r(&now, &now_fields);

    for (size_t k = 0; logger->format[k]; k++) {
        char c = logger->format[k];

        if (c == '%') {
            switch (logger->format[++k]) {
                unsigned int padding_length;
                size_t len, idx;
                char time_string[30], padding[8];

            case 'M': vasqIncVsnprintf(&dst, &remaining, format, args); break;

            case 'p': vasqIncSnprintf(&dst, &remaining, "%i", (int)getpid()); break;

            case 'L': vasqIncSnprintf(&dst, &remaining, "%s", logLevelName(level)); break;

            case '_':
                padding_length = logLevelNamePadding(level);
                memset(padding, ' ', padding_length);
                padding[padding_length] = '\0';
                vasqIncSnprintf(&dst, &remaining, "%s", padding);
                break;

            case 'u': vasqIncSnprintf(&dst, &remaining, "%li", (long)now); break;

            case 't':
                ctime_r(&now, time_string);
                len = strlen(time_string);
                vasqIncSnprintf(&dst, &remaining, "%.*s", len - 1, time_string);
                break;

            case 'h': vasqIncSnprintf(&dst, &remaining, "%i", now_fields.tm_hour); break;

            case 'm': vasqIncSnprintf(&dst, &remaining, "%i", now_fields.tm_min); break;

            case 's': vasqIncSnprintf(&dst, &remaining, "%i", now_fields.tm_sec); break;

            case 'F':
                for (idx = strlen(file_name); idx > 0; idx--) {
                    if (file_name[idx] == '/') {
                        idx++;
                        break;
                    }
                }
                vasqIncSnprintf(&dst, &remaining, "%s", file_name + idx);
                break;

            case 'f': vasqIncSnprintf(&dst, &remaining, "%s", function_name); break;

            case 'l': vasqIncSnprintf(&dst, &remaining, "%u", line_no); break;

            case 'x':
                if (logger->processor) {
                    logger->processor(logger->user_data, level, &dst, &remaining);
                }
                break;

            case '%': vasqIncSnprintf(&dst, &remaining, "%%"); break;

            default: break;  // This should never happen.
            }
        }
        else {
            vasqIncSnprintf(&dst, &remaining, "%c", c);
        }
    }

    if (write(logger->fd, output, dst - output) < 0) {
        NO_OP;
    }
}

void
vasqRawLog(const vasqLogger *logger, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVRawLog(logger, format, args);
    va_end(args);
}

void
vasqVRawLog(const vasqLogger *logger, const char *format, va_list args)
{
    ssize_t written;
    char output[1024];

    if (!logger || logger->level == VASQ_LL_NONE) {
        return;
    }

    written = vasqSafeVsnprintf(output, sizeof(output), format, args);

    if (written > 0 && write(logger->fd, output, written) < 0) {
        NO_OP;
    }
}

void
vasqHexDump(const vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no,
            const char *name, const void *data, size_t size)
{
    const unsigned char *bytes = data;
    char output[5000];
    char *dst = output;
    size_t actual_dump_size, remaining = sizeof(output);

    if (!logger || logger->level < VASQ_LL_DEBUG) {
        return;
    }

    vasqLogStatement(logger, VASQ_LL_DEBUG, file_name, function_name, line_no, "%s (%zu byte%s):", name,
                     size, (size == 1) ? "" : "s");

    actual_dump_size = MIN(size, MAX_HEXDUMP_SIZE);
    for (size_t k = 0; k < actual_dump_size; k += HEXDUMP_WIDTH) {
        unsigned int line_length;

        vasqIncSnprintf(&dst, &remaining, "\t%04x  ", k);

        line_length = MIN(actual_dump_size - k, HEXDUMP_WIDTH);
        for (unsigned int j = 0; j < line_length; j++) {
            vasqIncSnprintf(&dst, &remaining, "%02x ", bytes[k + j]);
        }

        for (unsigned int j = line_length; j < HEXDUMP_WIDTH; j++) {
            vasqIncSnprintf(&dst, &remaining, "   ");
        }

        vasqIncSnprintf(&dst, &remaining, "    ");

        for (unsigned int j = 0; j < line_length; j++) {
            char c;

            c = bytes[k + j];
            vasqIncSnprintf(&dst, &remaining, "%c", safeIsprint(c) ? c : '.');
        }

        vasqIncSnprintf(&dst, &remaining, "\n");
    }

    if (size > actual_dump_size) {
        vasqIncSnprintf(&dst, &remaining, "\t... (%zu more byte%s)\n", size - actual_dump_size,
                        (size - actual_dump_size == 1) ? "" : "s");
    }

    if (write(logger->fd, output, dst - output) < 0) {
        NO_OP;
    }
}

void *
vasqMalloc(const vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no,
           size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (!ptr && size > 0) {
        vasqLogStatement(logger, VASQ_LL_ERROR, file_name, function_name, line_no,
                         "Failed to allocate %zu bytes", size);
    }
    return ptr;
}

void *
vasqCalloc(const vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no,
           size_t nmemb, size_t size)
{
    void *ptr;

    ptr = calloc(nmemb, size);
    if (!ptr && nmemb * size > 0) {
        vasqLogStatement(logger, VASQ_LL_ERROR, file_name, function_name, line_no,
                         "Failed to allocate %zu bytes", nmemb * size);
    }
    return ptr;
}

void *
vasqRealloc(const vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no,
            void *ptr, size_t size)
{
    void *success;

    success = realloc(ptr, size);
    if (!success && size > 0) {
        vasqLogStatement(logger, VASQ_LL_ERROR, file_name, function_name, line_no,
                         "Failed to reallocate %zu bytes", size);
    }
    return success;
}

pid_t
vasqFork(const vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no)
{
    pid_t child;

    switch ((child = fork())) {
    case -1:
        vasqLogStatement(logger, VASQ_LL_ERROR, file_name, function_name, line_no, "fork: %s",
                         strerror(errno));
        break;

    case 0: break;

    default:
        vasqLogStatement(logger, VASQ_LL_DEBUG, file_name, function_name, line_no,
                         "Child process started (PID = %i)", child);
        break;
    }

    return child;
}
