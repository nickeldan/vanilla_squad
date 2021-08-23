#include "vasq/definitions.h"

const char *
vasqErrorString(int errnum)
{
    switch (errnum) {
    case VASQ_RET_OK: return "No error";
    case VASQ_RET_USAGE: return "A function was called with invalid arguments"; break;
    case VASQ_RET_BAD_FORMAT: return "Invalid format string";
    case VASQ_RET_OUT_OF_MEMORY: return "Ran out of memory";
    case VASQ_RET_DUP_FAIL: return "dup failed";
    case VASQ_RET_FCNTL_FAIL: return "fcntl failed";
    default: return "Invalid error value";
    }
}