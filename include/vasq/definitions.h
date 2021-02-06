#ifndef __VANILLA_SQUAD_DEFINITIONS_H__
#define __VANILLA_SQUAD_DEFINITIONS_H__

#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>

#define NO_OP ((void)0)

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

enum vasqRetValue {
    VASQ_RET_OK = 0,
    VASQ_RET_USAGE,
    VASQ_RET_REDIRECT_FAIL,
    VASQ_RET_OUT_OF_MEMORY,
    VASQ_RET_FORK,

    VASQ_RET_UNUSED,
};

#endif // __VANILLA_SQUAD_DEFINITIONS_H__
