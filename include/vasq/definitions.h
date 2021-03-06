#ifndef VANILLA_SQUAD_DEFINITIONS_H
#define VANILLA_SQUAD_DEFINITIONS_H

#ifndef NO_OP
#define NO_OP ((void)0)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

enum vasqRetValue {
    VASQ_RET_OK = 0,
    VASQ_RET_USAGE,
    VASQ_RET_OUT_OF_MEMORY,

    VASQ_RET_UNUSED,
};

#define VASQ_VERSION "3.0.1"

#endif  // VANILLA_SQUAD_DEFINITIONS_H
