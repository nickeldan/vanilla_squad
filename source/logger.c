#include <string.h>
#include <time.h>

#define VASQ_ENABLE_LOGGING
#include "vasq/logger.h"
#include "vasq/safe_snprintf.h"

#define MAX_HEXDUMP_SIZE 512
#define HEXDUMP_WIDTH    16
#if MAX_HEXDUMP_SIZE % HEXDUMP_WIDTH != 0
#error "MAX_HEXDUMP_SIZE must be a multiple of HEXDUMP_WIDTH."
#endif

#ifndef VASQ_NEW_CHILD_LOG_LEVEL
#define VASQ_NEW_CHILD_LOG_LEVEL VASQ_LL_ALWAYS
#endif

// Global variables
static vasqLogLevel_t max_log_level;
static int log_fd = -1;
static bool include_file_name_in_log;
static pid_t log_pid;

static void
logShutdown(void);

static unsigned int
logLevelNamePadding(vasqLogLevel_t level);

static void
incSnprintf(char **output, size_t *capacity, const char *format, ...);

static void
incVsnprintf(char **output, size_t *capacity, const char *format, va_list args);

static bool
safeIsprint(char c);

int
vasqLogInit(vasqLogLevel_t level, FILE *out, bool include_file_name) {
    if (!out) {
        return VASQ_RET_IMPROPER_USE;
    }

    log_fd = dup(fileno(out));
    if (log_fd < 0) {
        perror("dup");
        return VASQ_RET_REDIRECT_FAIL;
    }
    atexit(logShutdown);

    include_file_name_in_log = include_file_name;
    log_pid = getpid();

    VASQ_SET_LOG_LEVEL(level);

    return VASQ_RET_OK;
}

void
vasqSetLogLevel(const char *file_name, const char *function_name, int line_no, vasqLogLevel_t level) {
    max_log_level = level;

    vasqLogStatement(VASQ_LL_ALWAYS, file_name, function_name, line_no, "Log level set to %s",
                     vasqLogLevelName(level));
}

void
vasqLogStatement(vasqLogLevel_t level, const char *file_name, const char *function_name, int line_no,
                 const char *format, ...) {
    char output[1024], padding[8];
    char *dst = output;
    va_list args;
    size_t remaining = sizeof(output) - 1;  // Leave room for the '\n'.

    if (level > max_log_level || log_fd == -1) {
        return;
    }

    memset(padding, ' ', sizeof(padding));
    padding[logLevelNamePadding(level)] = '\0';

    incSnprintf(&dst, &remaining, "(%i) (%i) [%s]%s ", log_pid, (int)time(NULL), vasqLogLevelName(level),
                 padding);

    if (include_file_name_in_log) {
        incSnprintf(&dst, &remaining, "%s:", file_name);
    }

    incSnprintf(&dst, &remaining, "%s:%i ", function_name, line_no);

    va_start(args, format);
    incVsnprintf(&dst, &remaining, format, args);
    va_end(args);
    *(dst++) = '\n';

    if (write(log_fd, output, dst - output) < 0) {
        NO_OP;
    }
}

void
vasqHexDump(const char *file_name, const char *function_name, int line_no, const char *name,
            const void *data, size_t size) {
    const unsigned char *bytes = data;
    char output[4096];
    char *dst = output;
    size_t actual_dump_size, remaining = sizeof(output);

    if (max_log_level < VASQ_LL_DEBUG || log_fd == -1) {
        return;
    }

    vasqLogStatement(VASQ_LL_DEBUG, file_name, function_name, line_no, "%s (%zu byte%s):", name, size,
                     (size == 1) ? "" : "s");

    actual_dump_size = MIN(size, MAX_HEXDUMP_SIZE);
    for (size_t k = 0; k < actual_dump_size; k += HEXDUMP_WIDTH) {
        unsigned int line_length;

        incSnprintf(&dst, &remaining, "\t");

        line_length = MIN(actual_dump_size - k, HEXDUMP_WIDTH);
        for (unsigned int j = 0; j < line_length; j++) {
            incSnprintf(&dst, &remaining, "%02x ", bytes[k + j]);
        }

        for (unsigned int j=line_length; j<HEXDUMP_WIDTH; j++) {
            incSnprintf(&dst, &remaining, "   ");
        }

        incSnprintf(&dst, &remaining, "    ");

        for (unsigned int j = 0; j < line_length; j++) {
            char c;

            c = bytes[k + j];
            incSnprintf(&dst, &remaining, "%c", safeIsprint(c) ? c : '.');
        }

        incSnprintf(&dst, &remaining, "\n");
    }

    if (size > actual_dump_size) {
        incSnprintf(&dst, &remaining, "\t... (%zu more byte%s)\n", size - actual_dump_size,
                     (size - actual_dump_size == 1) ? "" : "s");
    }

    if (write(log_fd, output, dst - output) < 0) {
        NO_OP;
    }
}

void *
vasqMalloc(const char *file_name, const char *function_name, int line_no, size_t size) {
    void *ptr;

    ptr = malloc(size);
    if (!ptr && size > 0) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to allocate %zu bytes",
                         size);
    }
    return ptr;
}

void *
vasqCalloc(const char *file_name, const char *function_name, int line_no, size_t nmemb, size_t size) {
    void *ptr;

    ptr = calloc(nmemb, size);
    if (!ptr && nmemb * size > 0) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to allocate %zu bytes",
                         nmemb * size);
    }
    return ptr;
}

void *
vasqRealloc(const char *file_name, const char *function_name, int line_no, void *ptr, size_t size) {
    void *success;

    success = realloc(ptr, size);
    if (!success && size > 0) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to reallocate %zu bytes",
                         size);
    }
    return success;
}

pid_t
vasqFork(const char *file_name, const char *function_name, int line_no) {
    pid_t child;

    switch ((child = fork())) {
    case -1:
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "fork: %s", strerror(errno));
        break;

    case 0: log_pid = getpid(); break;

    default:
        vasqLogStatement(VASQ_NEW_CHILD_LOG_LEVEL, file_name, function_name, line_no,
                         "Child process started (PID = %i)", child);
        break;
    }

    return child;
}

const char *
vasqLogLevelName(vasqLogLevel_t level) {
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

static void
logShutdown(void) {
    close(log_fd);
}

static unsigned int
logLevelNamePadding(vasqLogLevel_t level) {
    switch (level) {
    case VASQ_LL_ALWAYS: return 2;
    case VASQ_LL_CRITICAL: return 0;
    case VASQ_LL_ERROR: return 3;
    case VASQ_LL_WARNING: return 1;
    case VASQ_LL_INFO: return 4;
    case VASQ_LL_DEBUG: return 3;
    default: return 1;
    }
}

static void
incSnprintf(char **output, size_t *capacity, const char *format, ...) {
    va_list args;

    va_start(args, format);
    incVsnprintf(output, capacity, format, args);
    va_end(args);
}

static void
incVsnprintf(char **output, size_t *capacity, const char *format, va_list args) {
    ssize_t ret;

    if (!output || !capacity || !format) {
        return;
    }

    ret = vasqSafeVsnprintf(*output, *capacity, format, args);
    if (ret > 0) {
        *output += ret;
        *capacity -= ret;
    }
}

static bool
safeIsprint(char c) {
    return c >= ' ' && c <= '~';
}
