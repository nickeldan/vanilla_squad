#ifndef VANILLA_SQUAD_DEFINITIONS_H
#define VANILLA_SQUAD_DEFINITIONS_H

#define VASQ_VERSION "5.0.0"

#ifndef NO_OP
#define NO_OP ((void)0)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

enum vasqRetValue {
    VASQ_RET_OK = 0,
    VASQ_RET_USAGE,
    VASQ_RET_BAD_FORMAT,
    VASQ_RET_OUT_OF_MEMORY,
    VASQ_RET_DUP_FAIL,
    VASQ_RET_FCNTL_FAIL,

    VASQ_RET_UNUSED,
};

const char *
vasqErrorString(int errnum) __attribute__((pure));

#endif  // VANILLA_SQUAD_DEFINITIONS_H
