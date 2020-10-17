#ifndef __VANILLA_SQUAD_DEFINITIONS_H__
#define __VANILLA_SQUAD_DEFINITIONS_H__

#include <stdbool.h>
#include <sys/types.h>

#define NO_OP (void)0

enum vasqRetValue {
    VASQ_RET_OK = 0,
    VASQ_RET_IMPROPER_USE,
    VASQ_RET_REDIRECT_FAIL,
    VASQ_RET_OUT_OF_MEMORY,
    VASQ_RET_FORK,
};

#endif // __VANILLA_SQUAD_DEFINITIONS_H__
