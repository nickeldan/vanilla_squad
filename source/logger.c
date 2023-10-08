#ifndef VASQ_NO_LOGGING

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "vasq/config.h"
#include "vasq/logger.h"
#include "vasq/safe_snprintf.h"

#if VASQ_HEXDUMP_SIZE % VASQ_HEXDUMP_WIDTH != 0
#error "VASQ_HEXDUMP_SIZE must be a multiple of VASQ_HEXDUMP_WIDTH."
#endif

struct vasqLogger {
    const char *format;
    char *name;
    vasqHandler handler;
    vasqLoggerOptions options;
    vasqLogLevel level;
};

static bool
validLogFormat(const char *format)
{
    unsigned int m_occurrences = 0;

    if (!format) {
        return false;
    }

    for (size_t k = 0; format[k]; k++) {
        if (format[k] == '%') {
            switch (format[++k]) {
            case 'M':
                if (++m_occurrences == 2) {
                    return false;
                }
            /* FALLTHROUGH */
            case 'p':
#ifdef __linux__
            case 'T':
#endif
            case 'L':
            case '_':
            case 'N':
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
safeIsPrint(char c)
{
    return c >= ' ' && c <= '~';
}

static const char *
logLevelName(vasqLogLevel level)
{
    switch (level) {
    case VASQ_LL_ALWAYS: return "ALWAYS";
    case VASQ_LL_CRITICAL: return "CRITICAL";
    case VASQ_LL_ERROR: return "ERROR";
    case VASQ_LL_WARNING: return "WARNING";
    case VASQ_LL_INFO: return "INFO";
    case VASQ_LL_DEBUG: return "DEBUG";
    default: return "INVALID";  // This should never happen.
    }
}

static unsigned int
logLevelNamePadding(vasqLogLevel level)
{
    switch (level) {
    case VASQ_LL_ALWAYS: return 2;
    case VASQ_LL_CRITICAL: return 0;
    case VASQ_LL_ERROR: return 3;
    case VASQ_LL_WARNING: return 1;
    case VASQ_LL_INFO: return 4;
    case VASQ_LL_DEBUG: return 3;
    default: return 0;  // This should never happen.
#define LOG_LEVEL_NAME_MAX_PADDING 4
    }
}

static void
vlogToBuffer(vasqLogger *logger, vasqLogLevel level, const char *file_name, const char *function_name,
             unsigned int line_no, char **dst, size_t *remaining, const char *format, va_list args)
{
    size_t position = 0;
    time_t now;
    struct tm now_fields;

    now = time(NULL);
    localtime_r(&now, &now_fields);

    for (size_t k = 0; logger->format[k]; k++) {
        char c = logger->format[k];

        if (c == '%') {
            switch (logger->format[++k]) {
                unsigned int padding_length, len;
                size_t idx;
                char time_string[30], padding[LOG_LEVEL_NAME_MAX_PADDING + 1];
                struct timespec epoch;

            case 'M': vasqIncVsnprintf(dst, remaining, format, args); break;

            case 'p': vasqIncSnprintf(dst, remaining, "%li", (long)getpid()); break;

#ifdef __linux__
            case 'T': vasqIncSnprintf(dst, remaining, "%li", (long)syscall(SYS_gettid)); break;
#endif

            case 'L': vasqIncSnprintf(dst, remaining, "%s", logLevelName(level)); break;

            case '_':
                padding_length = logLevelNamePadding(level);
                memset(padding, ' ', padding_length);
                padding[padding_length] = '\0';
                vasqIncSnprintf(dst, remaining, "%s", padding);
                break;

            case 'N':
                if (logger->options.name) {
                    vasqIncSnprintf(dst, remaining, "%s", logger->options.name);
                }
                break;

            case 'u':
                clock_gettime(CLOCK_REALTIME, &epoch);
                vasqIncSnprintf(dst, remaining, "%lli", (long long)epoch.tv_sec);
                break;

            case 't':
                ctime_r(&now, time_string);
                len = strnlen(time_string, sizeof(time_string));
                vasqIncSnprintf(dst, remaining, "%.*s", len - 1,
                                time_string);  // Don't include the newline character.
                break;

            case 'h': vasqIncSnprintf(dst, remaining, "%02i", now_fields.tm_hour); break;

            case 'm': vasqIncSnprintf(dst, remaining, "%02i", now_fields.tm_min); break;

            case 's': vasqIncSnprintf(dst, remaining, "%02i", now_fields.tm_sec); break;

            case 'F':
                for (idx = strlen(file_name); idx > 0; idx--) {
                    if (file_name[idx] == '/') {
                        idx++;
                        goto print_file_name;
                    }
                }
                if (file_name[0] == '/') {  // idx equals 0 here.
                    idx = 1;
                }
print_file_name:
                vasqIncSnprintf(dst, remaining, "%s", file_name + idx);
                break;

            case 'f': vasqIncSnprintf(dst, remaining, "%s", function_name); break;

            case 'l': vasqIncSnprintf(dst, remaining, "%u", line_no); break;

            case 'x':
                if (logger->options.processor) {
                    logger->options.processor(logger->options.user, position, level, dst, remaining);
                }
                position++;
                break;

            case '%':
                if (*remaining > 1) {
                    *((*dst)++) = '%';
                    **dst = '\0';
                    (*remaining)--;
                }
                break;

            default: __builtin_unreachable();
            }
        }
        else if (*remaining > 1) {
            *((*dst)++) = c;
            **dst = '\0';
            (*remaining)--;
        }
    }
}

static void
logToBuffer(vasqLogger *logger, vasqLogLevel level, const char *file_name, const char *function_name,
            unsigned int line_no, char **dst, size_t *remaining, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vlogToBuffer(logger, level, file_name, function_name, line_no, dst, remaining, format, args);
    va_end(args);
}

static void
writeToFd(void *user, vasqLogLevel level, const char *text, size_t size)
{
    int fd = (intptr_t)user;

    (void)level;

    if (write(fd, text, size) < 0) {
        NO_OP;
    }
}

static void
closeFd(void *user)
{
    int fd = (intptr_t)user;

    close(fd);
}

int
vasqFdHandlerCreate(int fd, unsigned int flags, vasqHandler *handler)
{
    int new_fd;

    if (!handler) {
        errno = EINVAL;
        return -1;
    }

    new_fd = dup(fd);
    if (new_fd < 0) {
        return -1;
    }

    if (flags & VASQ_LOGGER_FLAG_CLOEXEC) {
        int fd_flags;

        fd_flags = fcntl(new_fd, F_GETFD);
        if (fd_flags == -1 || fcntl(new_fd, F_SETFD, fd_flags | FD_CLOEXEC) == -1) {
            int local_errno = errno;

            close(new_fd);
            errno = local_errno;
            return -1;
        }
    }

    handler->func = writeToFd;
    handler->cleanup = closeFd;
    handler->user = (void *)(intptr_t)new_fd;

    return 0;
}

vasqLogger *
vasqLoggerCreate(vasqLogLevel level, const char *format, const vasqHandler *handler,
                 const vasqLoggerOptions *options)
{
    int errno_value;
    vasqLogger *logger;
    const vasqLoggerOptions default_options = {0};

    if (!options) {
        options = &default_options;
    }

    if (!handler || !handler->func) {
        errno = EINVAL;
        return NULL;
    }

    if (!validLogFormat(format)) {
        errno_value = EINVAL;
        goto error;
    }

    logger = malloc(sizeof(*logger));
    if (!logger) {
        errno_value = ENOMEM;
        goto error;
    }

    logger->format = format;
    memcpy(&logger->handler, handler, sizeof(*handler));
    memcpy(&logger->options, options, sizeof(*options));
    logger->level = level;

    if (options->name) {
        logger->options.name = strdup(options->name);
        if (!logger->options.name) {
            free(logger);
            errno_value = ENOMEM;
            goto error;
        }
    }

    return logger;

error:
    if (handler->cleanup) {
        handler->cleanup(handler->user);
    }
    errno = errno_value;
    return NULL;
}

void
vasqLoggerFree(vasqLogger *logger)
{
    if (!logger) {
        return;
    }

    if (logger->handler.cleanup) {
        logger->handler.cleanup(logger->handler.user);
    }

    free(logger->options.name);

    free(logger);
}

vasqLogLevel
vasqLoggerLevel(vasqLogger *logger)
{
    return logger ? logger->level : VASQ_LL_NONE;
}

void
vasqSetLoggerLevel(vasqLogger *logger, vasqLogLevel level)
{
    if (logger) {
        logger->level = level;
    }
}

const char *
vasqLoggerName(vasqLogger *logger)
{
    return logger ? logger->options.name : NULL;
}

void
vasqLogStatement(vasqLogger *logger, vasqLogLevel level, const char *file_name, const char *function_name,
                 unsigned int line_no, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVLogStatement(logger, level, file_name, function_name, line_no, format, args);
    va_end(args);
}

void
vasqVLogStatement(vasqLogger *logger, vasqLogLevel level, const char *file_name, const char *function_name,
                  unsigned int line_no, const char *format, va_list args)
{
    char output[VASQ_LOGGING_LENGTH];
    char *dst = output;
    size_t remaining = sizeof(output);
    int remote_errno;

    if (!logger || level > logger->level || logger->level == VASQ_LL_NONE) {
        return;
    }

    remote_errno = errno;
    vlogToBuffer(logger, level, file_name, function_name, line_no, &dst, &remaining, format, args);
    logger->handler.func(logger->handler.user, level, output, dst - output);
    errno = remote_errno;
}

void
vasqRawLog(vasqLogger *logger, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVRawLog(logger, format, args);
    va_end(args);
}

void
vasqVRawLog(vasqLogger *logger, const char *format, va_list args)
{
    int remote_errno;
    ssize_t written;
    char output[VASQ_LOGGING_LENGTH];

    if (!logger || logger->level == VASQ_LL_NONE) {
        return;
    }

    remote_errno = errno;
    written = vasqSafeVsnprintf(output, sizeof(output), format, args);
    logger->handler.func(logger->handler.user, VASQ_LL_NONE, output, written);
    errno = remote_errno;
}

void
vasqHexDump(vasqLogger *logger, const char *file_name, const char *function_name, unsigned int line_no,
            const char *name, const void *data, size_t size)
{
#define NUM_HEXDUMP_LINES   (VASQ_HEXDUMP_SIZE / VASQ_HEXDUMP_WIDTH)
#define HEXDUMP_LINE_LENGTH (VASQ_HEXDUMP_WIDTH * 4 + 10)
#define HEXDUMP_BUFFER_SIZE (NUM_HEXDUMP_LINES * HEXDUMP_LINE_LENGTH + 250)

    const unsigned char *bytes = data;
    char output[VASQ_LOGGING_LENGTH + HEXDUMP_BUFFER_SIZE];
    char *dst = output;
    int remote_errno;
    unsigned int actual_dump_size;
    size_t remaining = sizeof(output);
    vasqLogLevel dump_level;

    if (!logger) {
        return;
    }
    dump_level = (logger->options.flags & VASQ_LOGGER_FLAG_HEX_DUMP_INFO) ? VASQ_LL_INFO : VASQ_LL_DEBUG;
    if (logger->level < dump_level) {
        return;
    }

    remote_errno = errno;

    logToBuffer(logger, dump_level, file_name, function_name, line_no, &dst, &remaining,
                "%s (%zu byte%s):", name, size, (size == 1) ? "" : "s");

    actual_dump_size = MIN(size, VASQ_HEXDUMP_SIZE);
    for (unsigned int k = 0; k < actual_dump_size; k += VASQ_HEXDUMP_WIDTH) {
        unsigned int line_length;
        char print_buffer[VASQ_HEXDUMP_WIDTH];

        vasqIncSnprintf(&dst, &remaining, "\t%04x\t", k);

        line_length = MIN(actual_dump_size - k, VASQ_HEXDUMP_WIDTH);
        for (unsigned int j = 0; j < line_length; j++) {
            vasqIncSnprintf(&dst, &remaining, "%02x ", bytes[k + j]);
        }

        for (unsigned int j = line_length; j < VASQ_HEXDUMP_WIDTH; j++) {
            vasqIncSnprintf(&dst, &remaining, "   ");
        }

        for (unsigned int j = 0; j < line_length; j++) {
            char c = bytes[k + j];

            print_buffer[j] = safeIsPrint(c) ? c : '.';
        }
        vasqIncSnprintf(&dst, &remaining, "\t%.*s\n", line_length, print_buffer);
    }

    if (size > actual_dump_size) {
        vasqIncSnprintf(&dst, &remaining, "\t... (%zu more byte%s)\n", size - actual_dump_size,
                        (size - actual_dump_size == 1) ? "" : "s");
    }

    logger->handler.func(logger->handler.user, dump_level, output, dst - output);
    errno = remote_errno;

#undef NUM_HEXDUMP_LINES
#undef HEXDUMP_LINE_LENGTH
#undef HEXDUMP_BUFFER_SIZE
}

#endif  // VASQ_NO_LOGGING
