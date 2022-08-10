#include <unistd.h>

#include <vasq/logger.h>

int
main(int argc, char **argv)
{
    int ret;
    vasqLogger *logger;

    if (argc < 2) {
        return VASQ_RET_USAGE;
    }

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_INFO, argv[1], NULL, &logger);
    if (ret != VASQ_RET_OK) {
        return ret;
    }

    VASQ_INFO(logger, "Check");
    vasqLoggerFree(logger);

    return VASQ_RET_OK;
}
