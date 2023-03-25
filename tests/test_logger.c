#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef DEBUG
#define DEBUG
#endif

#define VASQ_TEST_ASSERT

#include <scrutiny/scrutiny.h>
#include <vasq/logger.h>
#include <vasq/safe_snprintf.h>

struct test_ctx {
    char buffer[100];
};

bool _vasq_abort_caught;

void
test_logger_null_logger(void)
{
    VASQ_INFO(NULL, "Check");
}

static void
handler_func(void *user, vasqLogLevel level, const char *text, size_t size)
{
    int *num_ptr = user;

    SCR_ASSERT_EQ(level, VASQ_LL_INFO);

    SCR_ASSERT_STR_EQ(text, "Check");
    SCR_ASSERT_EQ(size, 5);

    SCR_ASSERT_EQ(*num_ptr, 0);
    (*num_ptr)++;
}

static void
handler_cleanup(void *user)
{
    int *num_ptr = user;

    (*num_ptr)++;
}

void
test_logger_handler(void)
{
    int num = 0;
    vasqHandler handler = {.func = handler_func, .cleanup = handler_cleanup, .user = &num};
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = vasqLoggerCreate(VASQ_LL_INFO, "%M", &handler, NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_EQ(num, 1);
    vasqLoggerFree(logger);
    SCR_ASSERT_EQ(num, 2);
}

void
test_logger_no_handler(void)
{
    SCR_ASSERT_PTR_EQ(vasqLoggerCreate(VASQ_LL_INFO, "%M", NULL, NULL), NULL);
}

void
test_logger_handler_no_func(void)
{
    vasqHandler handler = {0};

    SCR_ASSERT_PTR_EQ(vasqLoggerCreate(VASQ_LL_INFO, "%M", &handler, NULL), NULL);
    SCR_ASSERT_EQ(errno, EINVAL);
}

static void
write_to_ctx(void *user, vasqLogLevel level, const char *text, size_t size)
{
    struct test_ctx *ctx = user;

    (void)level;

    memset(ctx, 0, sizeof(*ctx));
    size = MIN(size, sizeof(ctx->buffer) - 1);
    memcpy(ctx->buffer, text, size);
}

static vasqLogger *
create_logger(struct test_ctx *ctx, vasqLogLevel level, const char *format, const vasqLoggerOptions *options)
{
    vasqHandler handler = {
        .func = write_to_ctx,
        .user = ctx,
    };

    memset(ctx, 0, sizeof(*ctx));
    return vasqLoggerCreate(level, format, &handler, options);
}

void
test_logger_get_level(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "", NULL), NULL);
    SCR_ASSERT_EQ(vasqLoggerLevel(logger), VASQ_LL_INFO);

    vasqLoggerFree(logger);
}

void
test_logger_set_level(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "", NULL), NULL);
    vasqSetLoggerLevel(logger, VASQ_LL_DEBUG);
    SCR_ASSERT_EQ(vasqLoggerLevel(logger), VASQ_LL_DEBUG);

    vasqLoggerFree(logger);
}

void
test_logger_empty(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "");

    vasqLoggerFree(logger);
}

void
test_logger_message(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%M", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "Check");

    vasqLoggerFree(logger);
}

void
test_logger_none_level(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_NONE, "%M", NULL), NULL);
    VASQ_ALWAYS(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "");

    vasqLoggerFree(logger);
}

void
test_logger_level_too_high(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%M", NULL), NULL);
    VASQ_DEBUG(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "");

    vasqLoggerFree(logger);
}

void
test_logger_pid(void)
{
    char answer[20];
    struct test_ctx ctx;
    vasqLogger *logger;

    snprintf(answer, sizeof(answer), "%li", (long)getpid());

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%p", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_tid(void)
{
#ifdef __linux__
    char answer[20];
    struct test_ctx ctx;
    vasqLogger *logger;

    snprintf(answer, sizeof(answer), "%li", (long)syscall(SYS_gettid));

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%T", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
#else
    SCR_TEST_SKIP();
#endif
}

void
test_logger_level(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%L", NULL), NULL);

    VASQ_ALWAYS(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "ALWAYS");

    VASQ_CRITICAL(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "CRITICAL");

    VASQ_ERROR(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "ERROR");

    VASQ_WARNING(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "WARNING");

    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "INFO");

    VASQ_DEBUG(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "DEBUG");

    vasqLoggerFree(logger);
}

void
test_logger_level_with_padding(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%L%_", NULL), NULL);

    VASQ_ALWAYS(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "ALWAYS  ");

    VASQ_CRITICAL(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "CRITICAL");

    VASQ_ERROR(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "ERROR   ");

    VASQ_WARNING(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "WARNING ");

    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "INFO    ");

    VASQ_DEBUG(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "DEBUG   ");

    vasqLoggerFree(logger);
}

void
test_logger_epoch(void)
{
    long long epoch;
    char *endptr;
    struct timespec now;
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%u", NULL), NULL);
    VASQ_INFO(logger, "Check");
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
        SCR_FAIL("clock_gettime: %s", strerror(errno));
    }

    epoch = strtoll(ctx.buffer, &endptr, 10);
    if (ctx.buffer[0] == '\0' || *endptr != '\0') {
        SCR_FAIL("Invalid logged time: %s", ctx.buffer);
    }

    SCR_ASSERT_LE((long long)now.tv_sec, epoch);
    SCR_ASSERT_LE(epoch - (long long)now.tv_sec, 1);

    vasqLoggerFree(logger);
}

void
test_logger_pretty_timestamp(void)
{
    time_t now;
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[30];

    now = time(NULL);
    ctime_r(&now, answer);
    answer[strlen(answer) - 1] = '\0';  // Remove the newline character.

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%t", NULL), NULL);
    VASQ_INFO(logger, "Check");

    SCR_LOG("Logged time: %s", ctx.buffer);
    SCR_LOG("Actual time: %s", answer);

    // We can't be sure that the seconds will be the same but everything before that should be.
    ctx.buffer[17] = '\0';
    answer[17] = '\0';
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    // The years should be the same.
    SCR_ASSERT_STR_EQ(ctx.buffer + 19, answer + 19);

    vasqLoggerFree(logger);
}

static void
get_date_fields(struct tm *fields)
{
    time_t now;

    now = time(NULL);
    localtime_r(&now, fields);
}

void
test_logger_hour(void)
{
    struct tm date_fields;
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[10];

    get_date_fields(&date_fields);
    snprintf(answer, sizeof(answer), "%02i", date_fields.tm_hour);

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%h", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_minute(void)
{
    struct tm date_fields;
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[10];

    get_date_fields(&date_fields);
    snprintf(answer, sizeof(answer), "%02i", date_fields.tm_min);

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%m", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_second(void)
{
    long second;
    struct tm date_fields;
    struct test_ctx ctx;
    char *endptr;
    vasqLogger *logger;

    get_date_fields(&date_fields);

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_DEBUG, "%s", NULL), NULL);
    VASQ_INFO(logger, "Check");

    second = strtol(ctx.buffer, &endptr, 10);
    if (ctx.buffer[0] == '\0' || *endptr != '\0') {
        SCR_FAIL("Invalid logged second: %s", ctx.buffer);
    }

    SCR_ASSERT_LE(date_fields.tm_sec, second);
    SCR_ASSERT_LE(second - date_fields.tm_sec, 1);

    vasqLoggerFree(logger);
}

static const char *
get_file_name(void)
{
    size_t idx;
    const char *name = __FILE__;

    for (idx = strlen(name) - 1; idx > 0; idx--) {
        if (name[idx] == '/') {
            idx++;
            goto found_slash;
        }
    }

    if (name[0] == '/') {
        idx = 1;
    }

found_slash:
    return name + idx;
}

void
test_logger_file(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%F", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, get_file_name());

    vasqLoggerFree(logger);
}

void
test_logger_func(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%f", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, __func__);

    vasqLoggerFree(logger);
}

void
test_logger_line(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[10];

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%l", NULL), NULL);
    snprintf(answer, sizeof(answer), "%u", __LINE__ + 1);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

static void
processor(void *user, size_t pos, vasqLogLevel level, char **buffer, size_t *remaining)
{
    int num = (intptr_t)user;

    SCR_ASSERT_EQ(level, VASQ_LL_INFO);

    vasqIncSnprintf(buffer, remaining, "%zu-%i", pos, num);
}

void
test_logger_user_data(void)
{
    struct test_ctx ctx;
    vasqLoggerOptions options = {.processor = processor, .user = (void *)(intptr_t)1};
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%x%M%x", &options), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "0-1Check1-1");

    vasqLoggerFree(logger);
}

void
test_logger_no_format(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_EQ(logger = create_logger(&ctx, VASQ_LL_INFO, NULL, NULL), NULL);
    SCR_ASSERT_EQ(errno, EINVAL);
}

void
test_logger_invalid_format(void)
{
    struct test_ctx ctx;

    SCR_ASSERT_PTR_EQ(create_logger(&ctx, VASQ_LL_INFO, "%k", NULL), NULL);
    SCR_ASSERT_EQ(errno, EINVAL);
}

void
test_logger_percent(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%%", NULL), NULL);
    VASQ_INFO(logger, "Check");
    SCR_ASSERT_STR_EQ(ctx.buffer, "%");

    vasqLoggerFree(logger);
}

static void
raw_handler(void *user, vasqLogLevel level, const char *text, size_t size)
{
    SCR_ASSERT_EQ(level, VASQ_LL_NONE);

    write_to_ctx(user, level, text, size);
}

static vasqLogger *
create_raw_logger(struct test_ctx *ctx)
{
    vasqHandler handler = {.func = raw_handler, .user = ctx};

    return vasqLoggerCreate(VASQ_LL_INFO, "%L: %M", &handler, NULL);
}

void
test_logger_raw(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_raw_logger(&ctx), NULL);
    vasqRawLog(logger, "Check: %i", 5);
    SCR_ASSERT_STR_EQ(ctx.buffer, "Check: 5");

    vasqLoggerFree(logger);
}

static void
do_vraw(vasqLogger *logger, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vasqVRawLog(logger, format, args);
    va_end(args);
}

void
test_logger_vraw(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_raw_logger(&ctx), NULL);
    do_vraw(logger, "Check: %i", 5);
    SCR_ASSERT_STR_EQ(ctx.buffer, "Check: 5");

    vasqLoggerFree(logger);
}

void
test_logger_perror(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[50];

    snprintf(answer, sizeof(answer), "ERROR: Check: %s", strerror(EINVAL));

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%L: %M", NULL), NULL);
    VASQ_PERROR(logger, "Check", EINVAL);
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_pwarning(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[50];

    snprintf(answer, sizeof(answer), "WARNING: Check: %s", strerror(EINVAL));

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%L: %M", NULL), NULL);
    VASQ_PWARNING(logger, "Check", EINVAL);
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_pcritical(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;
    char answer[50];

    snprintf(answer, sizeof(answer), "CRITICAL: Check: %s", strerror(EINVAL));

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_INFO, "%L: %M", NULL), NULL);
    VASQ_PCRITICAL(logger, "Check", EINVAL);
    SCR_ASSERT_STR_EQ(ctx.buffer, answer);

    vasqLoggerFree(logger);
}

void
test_logger_assert(void)
{
    struct test_ctx ctx;
    vasqLogger *logger;

    SCR_ASSERT_PTR_NEQ(logger = create_logger(&ctx, VASQ_LL_CRITICAL, "%L: %M", NULL), NULL);
    _vasq_abort_caught = false;
    VASQ_ASSERT(logger, 1 == 2);
    SCR_ASSERT_STR_EQ(ctx.buffer, "CRITICAL: Assertion failed: 1 == 2");
    SCR_ASSERT(_vasq_abort_caught);

    vasqLoggerFree(logger);
}

void
test_logger_fd_handler(void)
{
    int num_chars;
    vasqHandler handler;
    vasqLogger *logger;
    int fds[2];
    char buffer[100];

    if (pipe(fds) != 0) {
        SCR_FAIL("pipe: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(vasqFdHandlerCreate(fds[1], 0, &handler), 0);
    close(fds[1]);

    SCR_ASSERT_PTR_NEQ(logger = vasqLoggerCreate(VASQ_LL_INFO, "%M", &handler, NULL), NULL);
    VASQ_INFO(logger, "Check");
    num_chars = read(fds[0], buffer, sizeof(buffer) - 1);
    if (num_chars < 0) {
        SCR_FAIL("read: %s", strerror(errno));
    }
    buffer[num_chars] = '\0';
    SCR_ASSERT_STR_EQ(buffer, "Check");

    vasqLoggerFree(logger);
    close(fds[0]);
}

void
test_logger_fd_handler_cloexec(void)
{
    int fd_flags;
    vasqHandler handler;
    int fds[2];

    if (pipe(fds) != 0) {
        SCR_FAIL("pipe: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(vasqFdHandlerCreate(fds[1], VASQ_LOGGER_FLAG_CLOEXEC, &handler), 0);
    fd_flags = fcntl((intptr_t)handler.user, F_GETFD);
    if (fd_flags == -1) {
        SCR_FAIL("fcntl: %s", strerror(errno));
    }
    SCR_ASSERT(fd_flags & FD_CLOEXEC);

    handler.cleanup(handler.user);
    close(fds[0]);
    close(fds[1]);
}
