/**
 * @file definitions.h
 * @author Daniel Walker
 * @brief Provides common definitions for the library.
 */
#ifndef VANILLA_SQUAD_DEFINITIONS_H
#define VANILLA_SQUAD_DEFINITIONS_H

/**
 * @brief Current version of the library.
 */
#define VASQ_VERSION "5.1.1"

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

/**
 * @brief Error values that some library functions can return.
 */
enum vasqRetValue {
    VASQ_RET_OK = 0,
    VASQ_RET_USAGE,
    VASQ_RET_BAD_FORMAT,
    VASQ_RET_OUT_OF_MEMORY,
    VASQ_RET_DUP_FAIL,
    VASQ_RET_FCNTL_FAIL,

    VASQ_RET_UNUSED,
};

/**
 * @brief Convert an error value (see enum vasqRetValue) into a string.
 *
 * @param errnum The error value.
 *
 * @return The string.
 */
const char *
vasqErrorString(int errnum) __attribute__((pure));

#endif  // VANILLA_SQUAD_DEFINITIONS_H
