#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef VASQ_ENABLE_LOGGING
#define VASQ_ENABLE_LOGGING
#endif
#include "vasq/logger.h"
#include "vasq/safe_snprintf.h"

#define MAX_HEXDUMP_SIZE 512
#define HEXDUMP_WIDTH 16
#if MAX_HEXDUMP_SIZE%HEXDUMP_WIDTH != 0
#error "MAX_HEXDUMP_SIZE must be a multiple of HEXDUMP_WIDTH."
#endif

#ifndef VASQ_NEW_CHILD_LOG_LEVEL
#define VASQ_NEW_CHILD_LOG_LEVEL VASQ_LL_ALWAYS
#endif

// Global variables
static vasqLogLevel_t max_log_level;
static int log_fd = -1;
static bool include_file_name_in_log;
static pid_t my_pid;

static void
logShutdown(void);

static unsigned int
logLevelNamePadding(vasqLogLevel_t level) __attribute__ ((pure));

int
vasqLogInit(vasqLogLevel_t level, FILE *out, bool include_file_name) {
    if ( !out ) {
        return VASQ_RET_IMPROPER_USE;
    }

    log_fd = dup(fileno(out));
    if ( log_fd < 0 ) {
        perror("dup");
        return VASQ_RET_REDIRECT_FAIL;
    }
    atexit(logShutdown);

    include_file_name_in_log = include_file_name;
    my_pid = getpid();

    VASQ_SET_LOG_LEVEL(level);

    return VASQ_RET_OK;
}

void
vasqSetLogLevel(const char *file_name, const char *function_name, int line_no, vasqLogLevel_t level)
{
    max_log_level = level;

    vasqLogStatement(VASQ_LL_ALWAYS, file_name, function_name, line_no, "Log level set to %s",
        vasqLogLevelName(level));
}

void
vasqLogStatement(vasqLogLevel_t level, const char *file_name, const char *function_name, int line_no,
                 const char *format, ...)
{
    char output[1024], padding[8];
    va_list args;
    ssize_t so_far, temp;

    if ( level > max_log_level || log_fd == -1 ) {
        return;
    }

    memset(padding, ' ', sizeof(padding));
    padding[logLevelNamePadding(level)] = '\0';

    so_far = vasqSafeSnprintf(output, sizeof(output)-1, "(%i) (%i) [%s]%s ", my_pid, (int)time(NULL),
        vasqLogLevelName(level), padding);
    if ( include_file_name_in_log ) {
        so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-1-so_far, "%s:", file_name);
    }
    so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-1-so_far, "%s:%i ", function_name, line_no);

    va_start(args, format);
    temp = vasqSafeVsnprintf(output+so_far, sizeof(output)-1-so_far, format, args);
    va_end(args);
    if ( temp >= 0 ) {
        so_far += temp;
    }
    output[so_far++] = '\n';

    if ( write(log_fd, output, so_far) < 0 ) {
        NO_OP;
    }
}

void
vasqHexDump(const char *file_name, const char *function_name, int line_no, const char *name,
            const unsigned char *data, size_t size)
{
    size_t actual_dump_size;
    ssize_t so_far = 0;
    char output[4096];

    if ( max_log_level < VASQ_LL_DEBUG || log_fd == -1 ) {
        return;
    }

    vasqLogStatement(VASQ_LL_DEBUG, file_name, function_name, line_no, "%s (%zu byte%s):", name, size,
        (size == 1)? "" : "s");


    actual_dump_size = MIN(size,MAX_HEXDUMP_SIZE);
    for (size_t k=0; k<actual_dump_size; k++) {
        so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "%02x ", data[k]);
        if ( k%HEXDUMP_WIDTH == HEXDUMP_WIDTH-1 ) {
            so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "\t");

            for (size_t j=k+1-HEXDUMP_WIDTH; j<=k; j++) {
                char c;

                c = data[j];
                so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "%c", isprint(c)? c : '.');
            }
            so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "\n");
        }
    }

    if ( actual_dump_size%HEXDUMP_WIDTH != 0 ) {
        for (size_t k=0; k<HEXDUMP_WIDTH-actual_dump_size%HEXDUMP_WIDTH; k++) {
            so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "   ");
        }

        for (size_t k=actual_dump_size-actual_dump_size%HEXDUMP_WIDTH; k<actual_dump_size; k++) {
            char c;

            c = data[k];
            so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "%c", isprint(c)? c : '.');
        }
        so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "\n");
    }
    else if ( size > actual_dump_size ) {
        so_far += vasqSafeSnprintf(output+so_far, sizeof(output)-so_far, "... (%zu more byte%s)\n",
            size-actual_dump_size, (size-actual_dump_size == 1)? "" : "s");
    }

    if ( write(log_fd, output, so_far) < 0 ) {
        NO_OP;
    }
}

void*
vasqMalloc(const char *file_name, const char *function_name, int line_no, size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if ( !ptr && size > 0 ) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to allocate %zu bytes",
            size);
    }
    return ptr;
}

void*
vasqCalloc(const char *file_name, const char *function_name, int line_no, size_t nmemb, size_t size)
{
    void *ptr;

    ptr = calloc(nmemb,size);
    if ( !ptr && nmemb*size > 0 ) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to allocate %zu bytes",
            nmemb*size);
    }
    return ptr;
}

void*
vasqRealloc(const char *file_name, const char *function_name, int line_no, void *ptr, size_t size)
{
    void *success;

    success = realloc(ptr,size);
    if ( !success && size > 0 ) {
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "Failed to reallocate %zu bytes",
            size);
    }
    return success;
}

pid_t
vasqFork(const char *file_name, const char *function_name, int line_no)
{
    pid_t child;

    switch ( (child=fork()) ) {
    case -1:
        vasqLogStatement(VASQ_LL_ERROR, file_name, function_name, line_no, "fork: %s", strerror(errno));
        break;

    case 0:
        my_pid = getpid();
        break;

    default:
        vasqLogStatement(VASQ_NEW_CHILD_LOG_LEVEL, file_name, function_name, line_no,
            "Child process started (PID = %i)", child);
        break;
    }

    return child;
}

const char*
vasqLogLevelName(vasqLogLevel_t level)
{
    switch ( level ) {
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
logShutdown(void)
{
    close(log_fd);
}

static unsigned int
logLevelNamePadding(vasqLogLevel_t level)
{
    switch ( level ) {
    case VASQ_LL_ALWAYS: return 2;
    case VASQ_LL_CRITICAL: return 0;
    case VASQ_LL_ERROR: return 3;
    case VASQ_LL_WARNING: return 1;
    case VASQ_LL_INFO: return 4;
    case VASQ_LL_DEBUG: return 3;
    default: return 1;
    }
}
