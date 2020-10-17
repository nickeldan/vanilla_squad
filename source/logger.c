#ifdef DEBUG

#include <time.h>

#include <vasq/logger.h>

// Global variables
static logLevel_t max_log_level;
static int log_fd = -1;
static bool include_file_name_in_log;
static pid_t my_pid;

static void
log_shutdown(void);

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
    atexit(log_shutdown);

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
    char output[1024];
    time_t now;

    if ( level > max_log_level || log_fd == -1 ) {
        return;
    }
}

void
vasqHexDump(const char *file_name, const char *function_name, int line_no, const char *name,
            const unsigned char *data, size_t size)
{
    char output[4096];

    if ( VASQ_LL_DEBUG > max_log_level || log_fd == -1 ) {
        return;
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
        vasqLogStatement(VASQ_LL_ALWAYS, file_name, function_name, line_no, "Child process started (PID = "
                         "%i)", child);
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
log_shutdown(void)
{
    close(log_fd);
}

#endif // DEBUG
