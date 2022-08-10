#include <unistd.h>

#include <vasq/logger.h>

int
main()
{
    int ret;
    ssize_t num_bytes;
    unsigned char buffer[4096];
    vasqLogger *logger;
    vasqLoggerOptions options = {.flags = VASQ_LOGGER_FLAG_HEX_DUMP_INFO};

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_INFO, "%M\n", &options, &logger);
    if (ret != VASQ_RET_OK) {
        return ret;
    }

    num_bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (num_bytes < 0) {
        ret = -1;
        goto done;
    }

    VASQ_HEXDUMP(logger, "stdin", buffer, num_bytes);

done:
    vasqLoggerFree(logger);
    return ret;
}
