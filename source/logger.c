#ifndef VASQ_NO_LOGGING

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
    vasqLoggerDataProcessor processor;
    void *user;
    int fd;
    vasqLogLevel_t level;
    unsigned int duped : 1;
    unsigned int hex_dump_info : 1;
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
logLevelName(vasqLogLevel_t level)
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
logLevelNamePadding(vasqLogLevel_t level)
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

int
vasqLoggerCreate(int fd, vasqLogLevel_t level, const char *format, const vasqLoggerOptions *options,
                 vasqLogger **logger)
{
    int new_fd, local_errno;
    const vasqLoggerOptions default_options = {0};
    const vasqLoggerOptions *use_these_opts = options ? options : &default_options;

    if (fd < 0 || !logger) {
        return VASQ_RET_USAGE;
    }

    if (!validLogFormat(format)) {
        return VASQ_RET_BAD_FORMAT;
    }

    *logger = malloc(sizeof(**logger));
    if (!*logger) {
        return VASQ_RET_OUT_OF_MEMORY;
    }

    if (use_these_opts->flags & VASQ_LOGGER_FLAG_DUP) {
        while (true) {
            new_fd = dup(fd);
            if (new_fd == -1) {
                local_errno = errno;

                switch (local_errno) {
#ifdef EBUSY
                case EBUSY:
#endif
                case EINTR: continue;

                default:
                    free(*logger);
                    errno = local_errno;
                    return VASQ_RET_DUP_FAIL;
                }
            }
            else {
                break;
            }
        }

        (*logger)->duped = true;
    }
    else {
        new_fd = fd;
        (*logger)->duped = false;
    }

    (*logger)->fd = new_fd;
    (*logger)->format = format;
    (*logger)->processor = use_these_opts->processor;
    (*logger)->user = use_these_opts->user;
    (*logger)->hex_dump_info = !!(use_these_opts->flags & VASQ_LOGGER_FLAG_HEX_DUMP_INFO);
    vasqSetLoggerLevel(*logger, level);

    if (use_these_opts->flags & VASQ_LOGGER_FLAG_CLOEXEC) {
        int flags;

        flags = fcntl(new_fd, F_GETFD);
        if (flags == -1 || fcntl(new_fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
            local_errno = errno;
            vasqLoggerFree(*logger);
            errno = local_errno;
            return VASQ_RET_FCNTL_FAIL;
        }
    }

    return VASQ_RET_OK;
}

void
vasqLoggerFree(vasqLogger *logger)
{
    if (!logger) {
        return;
    }

    if (logger->duped) {
        close(logger->fd);
    }

    free(logger);
}

int
vasqLoggerFd(const vasqLogger *logger)
{
    return logger ? logger->fd : -1;
}

bool
vasqSetLoggerFormat(vasqLogger *logger, const char *format)
{
    if (logger && validLogFormat(format)) {
        logger->format = format;
        return true;
    }
    else {
        return false;
    }
}

vasqLogLevel_t
vasqLoggerLevel(const vasqLogger *logger)
{
    return logger ? logger->level : VASQ_LL_NONE;
}

void
vasqSetLoggerLevel(vasqLogger *logger, vasqLogLevel_t level)
{
    if (!logger) {
        return;
    }

    logger->level = level;
    vasqLogStatement(logger, VASQ_LL_LEVEL_CHANGE, VASQ_CONTEXT_PARAMS, "Log level set to %s",
                     logLevelName(level));
}

void
vasqSetLoggerProcessor(vasqLogger *logger, vasqLoggerDataProcessor processor)
{
    if (!logger) {
        return;
    }

    logger->processor = processor;
}

void *
vasqLoggerUserData(const vasqLogger *logger)
{
    return logger ? logger->user : NULL;
}

void
vasqSetLoggerUserData(vasqLogger *logger, void *user)
{
    if (!logger) {
        return;
    }

    logger->user = user;
}

void
vasqLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVLogStatement(logger, level, file_name, function_name, line_no, format, args);
    va_end(args);
}

void
vasqVLogStatement(const vasqLogger *logger, vasqLogLevel_t level, VASQ_CONTEXT_DECL, const char *format,
                  va_list args)
{
    char output[VASQ_LOGGING_LENGTH];
    char *dst = output;
    size_t remaining = sizeof(output), position = 0;
    int remote_errno;
    time_t now;
    struct tm now_fields;

    if (!logger || level > logger->level || logger->level == VASQ_LL_NONE) {
        return;
    }

    remote_errno = errno;

    now = time(NULL);
    localtime_r(&now, &now_fields);

    for (size_t k = 0; logger->format[k]; k++) {
        char c = logger->format[k];

        if (c == '%') {
            switch (logger->format[++k]) {
                unsigned int padding_length;
                size_t len, idx;
                char time_string[30], padding[LOG_LEVEL_NAME_MAX_PADDING + 1];

            case 'M': vasqIncVsnprintf(&dst, &remaining, format, args); break;

            case 'p': vasqIncSnprintf(&dst, &remaining, "%li", (long)getpid()); break;

#ifdef __linux__
            case 'T': vasqIncSnprintf(&dst, &remaining, "%li", syscall(SYS_gettid)); break;
#endif

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
                len = strnlen(time_string, sizeof(time_string));
                vasqIncSnprintf(&dst, &remaining, "%.*s", len - 1,
                                time_string);  // Don't include the newline character.
                break;

            case 'h': vasqIncSnprintf(&dst, &remaining, "%02i", now_fields.tm_hour); break;

            case 'm': vasqIncSnprintf(&dst, &remaining, "%02i", now_fields.tm_min); break;

            case 's': vasqIncSnprintf(&dst, &remaining, "%02i", now_fields.tm_sec); break;

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
                vasqIncSnprintf(&dst, &remaining, "%s", file_name + idx);
                break;

            case 'f': vasqIncSnprintf(&dst, &remaining, "%s", function_name); break;

            case 'l': vasqIncSnprintf(&dst, &remaining, "%u", line_no); break;

            case 'x':
                if (logger->processor) {
                    logger->processor(logger->user, position, level, &dst, &remaining);
                }
                position++;
                break;

            case '%': vasqIncSnprintf(&dst, &remaining, "%%"); break;

            default: __builtin_unreachable();
            }
        }
        else {
            vasqIncSnprintf(&dst, &remaining, "%c", c);
        }
    }

    if (write(logger->fd, output, dst - output) < 0) {
        NO_OP;
    }

    errno = remote_errno;
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
    int remote_errno;
    ssize_t written;
    char output[VASQ_LOGGING_LENGTH];

    if (!logger || logger->level == VASQ_LL_NONE) {
        return;
    }

    remote_errno = errno;

    written = vasqSafeVsnprintf(output, sizeof(output), format, args);

    if (written > 0 && write(logger->fd, output, written) < 0) {
        NO_OP;
    }

    errno = remote_errno;
}

void
vasqHexDump(const vasqLogger *logger, VASQ_CONTEXT_DECL, const char *name, const void *data, size_t size)
{
#define NUM_HEXDUMP_LINES   (VASQ_HEXDUMP_SIZE / VASQ_HEXDUMP_WIDTH)
#define HEXDUMP_LINE_LENGTH (VASQ_HEXDUMP_WIDTH * 4 + 10)
#define HEXDUMP_BUFFER_SIZE (NUM_HEXDUMP_LINES * HEXDUMP_LINE_LENGTH + 250)

    const unsigned char *bytes = data;
    char output[HEXDUMP_BUFFER_SIZE];
    char *dst = output;
    int remote_errno;
    size_t actual_dump_size, remaining = sizeof(output);
    vasqLogLevel_t dump_level;

    if (!logger) {
        return;
    }
    dump_level = logger->hex_dump_info ? VASQ_LL_INFO : VASQ_LL_DEBUG;
    if (logger->level < dump_level) {
        return;
    }

    remote_errno = errno;

    vasqLogStatement(logger, dump_level, file_name, function_name, line_no, "%s (%zu byte%s):", name, size,
                     (size == 1) ? "" : "s");

    actual_dump_size = MIN(size, VASQ_HEXDUMP_SIZE);
    for (size_t k = 0; k < actual_dump_size; k += VASQ_HEXDUMP_WIDTH) {
        unsigned int line_length;
        char print_buffer[VASQ_HEXDUMP_WIDTH];

        vasqIncSnprintf(&dst, &remaining, "\t%04x  ", k);

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
        vasqIncSnprintf(&dst, &remaining, "    %.*s\n", line_length, print_buffer);
    }

    if (size > actual_dump_size) {
        vasqIncSnprintf(&dst, &remaining, "\t... (%zu more byte%s)\n", size - actual_dump_size,
                        (size - actual_dump_size == 1) ? "" : "s");
    }

    if (write(logger->fd, output, dst - output) < 0) {
        NO_OP;
    }

    errno = remote_errno;

#undef NUM_HEXDUMP_LINES
#undef HEXDUMP_LINE_LENGTH
#undef HEXDUMP_BUFFER_SIZE
}

void *
vasqMalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t size)
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
vasqCalloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, size_t nmemb, size_t size)
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
vasqRealloc(const vasqLogger *logger, VASQ_CONTEXT_DECL, void *ptr, size_t size)
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
vasqFork(const vasqLogger *logger, VASQ_CONTEXT_DECL)
{
    pid_t child;

    switch ((child = fork())) {
    case -1:
        vasqLogStatement(logger, VASQ_LL_ERROR, file_name, function_name, line_no, "fork: %s",
                         strerror(errno));
        break;

    case 0: break;

    default:
        vasqLogStatement(logger, VASQ_LL_PROCESS, file_name, function_name, line_no,
                         "Child process started (PID = %li)", (long)child);
        break;
    }

    return child;
}

void
vasqExit(vasqLogger *logger, VASQ_CONTEXT_DECL, int value, bool quick)
{
    vasqLogStatement(logger, VASQ_LL_PROCESS, file_name, function_name, line_no,
                     "Process exiting with value %i", value);
    vasqLoggerFree(logger);
    (quick ? _exit : exit)(value);
}

#endif  // VASQ_NO_LOGGING
