#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <vasq/logger.h>

int
main()
{
    int ret;
    uintmax_t value = 0xdadbeef;
    intmax_t signed_value = -1 * value;
    vasqLogger *logger;
    vasqLoggerOptions options = {.flags = VASQ_LOGGER_FLAG_DUP | VASQ_LOGGER_FLAG_CLOEXEC};
    unsigned char short_data[10];
    const char data[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non tellus vel turpis mattis porta "
        "lobortis at mi. Curabitur sed diam est. Praesent eget ex nulla. Morbi fermentum, eros aliquam "
        "ornare tempor, tellus metus dignissim lacus, in auctor nisi sem vitae magna. Sed eu lacus sed "
        "libero scelerisque consectetur. Nam vel quam condimentum, aliquet nunc at, lobortis diam. "
        "Curabitur eget sodales diam. Fusce ut varius enim. Sed at urna quam. Etiam dapibus dui tortor, ac "
        "efficitur libero venenatis suscipit. Duis risus purus, pretium in malesuada sed, lobortis eget "
        "leo. Nulla facilisi. Suspendisse venenatis erat a justo ornare rutrum. Phasellus sed risus quis "
        "neque suscipit efficitur.  Sed vehicula vitae tellus vel pulvinar. Aenean metus ante, vehicula vel "
        "consequat sit amet, laoreet nec ipsum. Etiam sed nulla velit. Aenean volutpat vitae enim eget "
        "feugiat. Aliquam ac arcu in ligula malesuada facilisis. Proin consectetur sem arcu, eget dictum "
        "diam gravida ut. Proin ultrices bibendum turpis, et consequat dui.  Vestibulum ultrices ultricies "
        "porttitor. Proin a ullamcorper libero, et efficitur orci. Nunc viverra interdum erat sit amet "
        "finibus. Duis lacus mauris, rhoncus sit amet euismod et, vulputate at eros. Quisque maximus turpis "
        "et quam placerat, eu semper magna efficitur. Donec nec vulputate justo, sed eleifend nunc. Donec "
        "dictum mattis massa, a feugiat elit pretium nec.  Maecenas eleifend enim in quam cursus, a "
        "sagittis neque tempus. Maecenas eu massa id dolor sollicitudin fermentum at in nunc. Vestibulum "
        "ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Nulla at bibendum ex. "
        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Fusce "
        "condimentum, ex eu hendrerit hendrerit, turpis dui scelerisque risus, nec sodales urna sem eget "
        "felis. Morbi elit libero, pulvinar sed euismod vitae, euismod quis elit. Mauris vehicula mi mollis "
        "tortor eleifend pretium. Etiam egestas congue metus, et feugiat ex rhoncus ac.  Maecenas sed magna "
        "lorem. Nam enim ex, porttitor a elit nec, commodo tempor nisi. Nulla tristique et odio in "
        "consectetur. Fusce accumsan ante arcu. Mauris euismod arcu sit amet libero porttitor vestibulum. "
        "Sed imperdiet justo nibh, sed viverra erat sagittis et. Mauris vel malesuada odio.";

#ifdef __linux__
#define LOGGER_FORMAT "(%p:%T) (%h:%m:%s) (%t) [%L]%_ %F:%f:%l: %M\n"
#else
#define LOGGER_FORMAT "(%p) (%h:%m:%s) (%t) [%L]%_ %F:%f:%l: %M\n"
#endif

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_DEBUG, LOGGER_FORMAT, &options, &logger);
    if (ret != VASQ_RET_OK) {
        fprintf(stderr, "vasqLoggerCreate failed: %s\n", vasqErrorString(ret));
        return ret;
    }

    VASQ_ALWAYS(logger, "This is an ALWAYS message");
    VASQ_CRITICAL(logger, "This is a CRITICAL message");
    VASQ_ERROR(logger, "This is an ERROR message");
    VASQ_WARNING(logger, "This is a WARNING message");
    VASQ_INFO(logger, "This is an INFO message");
    VASQ_DEBUG(logger, "This is a DEBUG message");

    VASQ_INFO(logger, "%%i: %i", (int)signed_value);
    VASQ_INFO(logger, "%%d: %d", (int)signed_value);
    VASQ_INFO(logger, "%%u: %u", (unsigned int)value);
    VASQ_INFO(logger, "%%x: %x", (unsigned int)value);
    VASQ_INFO(logger, "%%li: %li", (long)signed_value);
    VASQ_INFO(logger, "%%ld: %ld", (long)signed_value);
    VASQ_INFO(logger, "%%lu: %lu", (unsigned long)value);
    VASQ_INFO(logger, "%%lx: %lx", (unsigned long)value);
    VASQ_INFO(logger, "%%lli: %lli", (long long)signed_value);
    VASQ_INFO(logger, "%%lld: %lld", (long long)signed_value);
    VASQ_INFO(logger, "%%llu: %llu", (unsigned long long)value);
    VASQ_INFO(logger, "%%llx: %llx", (unsigned long long)value);
    VASQ_INFO(logger, "%%zi: %zi", (ssize_t)signed_value);
    VASQ_INFO(logger, "%%zu: %zu", (size_t)value);
    VASQ_INFO(logger, "%%ji: %ji", signed_value);
    VASQ_INFO(logger, "%%ju: %ju", value);
    VASQ_INFO(logger, "%%p: %p", &value);

    for (unsigned int k = 0; k < sizeof(short_data); k++) {
        short_data[k] = 10 * k;
    }
    VASQ_HEXDUMP(logger, "Short data", short_data, sizeof(short_data));

    VASQ_HEXDUMP(logger, "Long data", data, sizeof(data));

    VASQ_EXIT(logger, 0);
}
