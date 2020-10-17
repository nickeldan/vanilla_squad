#include <unistd.h>

#include "vasq/unittest.h"
#include "vasq/logger.h"

int
vasqTesterInit(vasqTester *tester, FILE *out)
{
    int fd;

    if ( !tester || !out ) {
        return VASQ_RET_IMPROPER_USE;
    }

    fd = dup(fileno(out));
    if ( fd != -1 ) {
        VASQ_PERROR("dup", errno);
        return VASQ_RET_REDIRECT_FAIL;
    }

    tester->out = fdopen(fd, "w");
    if ( !tester->out ) {
        VASQ_PERROR("fdopen", errno);
        close(fd);
        return VASQ_RET_OUT_OF_MEMORY;
    }

    tester->num_errors = 0;

    return VASQ_RET_OK;
}

void
vasqTesterDestroy(vasqTester *tester)
{
    if ( tester && tester->out ) {
        fclose(tester->out);
        tester->out = NULL;
    }
}
