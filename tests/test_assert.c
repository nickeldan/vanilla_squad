#ifndef DEBUG
#define DEBUG
#endif

#define VASQ_TEST_ASSERT

#include <unistd.h>

#include <vasq/logger.h>

bool _vasq_abort_caught;

int
returnsZero(void)
{
    return 0;
}

int
returnsOne(void)
{
    return 1;
}

int
main()
{
    int ret;
    vasqLogger *logger;

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_INFO, "[%L]%_ %M\n", NULL, &logger);
    if (ret != VASQ_RET_OK) {
        return ret;
    }

    VASQ_INFO(logger, "Asserting a false statement");
    VASQ_ASSERT(logger, returnsZero() == 1);

    if (_vasq_abort_caught) {
        VASQ_INFO(logger, "Abort caught");
    }
    else {
        VASQ_ERROR(logger, "Assertion did not cause an abort");
        ret = -1;
        goto done;
    }

    VASQ_INFO(logger, "Asserting a true statement");
    VASQ_ASSERT(logger, returnsOne() == 1);

    if (_vasq_abort_caught) {
        VASQ_ERROR(logger, "Assertion caused an abort");
        ret = -1;
    }
    else {
        VASQ_INFO(logger, "No abort was caught");
    }

done:
    vasqLoggerFree(logger);
    return ret;
}