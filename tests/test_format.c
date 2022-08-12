#include <string.h>
#include <unistd.h>

#include <vasq/logger.h>
#include <vasq/safe_snprintf.h>

static void
processor(void *user, size_t idx, vasqLogLevel_t level, char **dst, size_t *remaining)
{
    (void)level;
    int *x = (int *)user;

    (*x)++;
    vasqIncSnprintf(dst, remaining, "%zu-%i", idx, *x);
}

int
main(int argc, char **argv)
{
    int ret, x = 0;
    vasqLogger *logger;
    vasqLoggerOptions options = {.processor = processor, .user = &x};

    if (argc < 2) {
        return VASQ_RET_USAGE;
    }

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_INFO, argv[1], &options, &logger);
    if (ret != VASQ_RET_OK) {
        return ret;
    }

    VASQ_INFO(logger, "Check");
    if (strstr(argv[1], "%l")) {
        VASQ_INFO(logger, "Check 2");
    }
    vasqLoggerFree(logger);

    return VASQ_RET_OK;
}
