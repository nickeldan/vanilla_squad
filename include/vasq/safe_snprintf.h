/**
 * @file safe_snprintf.h
 * @author Daniel Walker
 * @brief Provides the safe_snprintf family of functions.
 */
#ifndef VANILLA_SQUAD_SAFE_SNPRINTF_H
#define VANILLA_SQUAD_SAFE_SNPRINTF_H

#include <stdarg.h>
#include <sys/types.h>

#include "definitions.h"

/**
 * @brief Signal-safe version of snprintf.
 *
 * If -1 is not returned, then a null-terminator is guaranteed to be written.
 *
 * @param buffer The destination buffer.
 * @param size The number of bytes in the buffer.  This must be at least 1 so that a null-terminator can be
 * written.
 * @param format The format string.  See the README for the list of supported %-tokens.
 *
 * @return The number of characters (not counting the null-terminator) that were actually written.  If either
 * buffer or format is NULL or if size is 0, then -1 is returned.
 */
ssize_t
vasqSafeSnprintf(char *buffer, size_t size, const char *format, ...) VASQ_FORMAT(3);

/**
 * @brief Same as vasqSafeSnprintf but takes a va_list instead of variable arguments.
 */
ssize_t
vasqSafeVsnprintf(char *buffer, size_t size, const char *format, va_list args);

/**
 * @brief Same as vasqSafeSnprintf but adjusts the output pointer and remaining capacity.
 *
 * Where vasqSafeSnprintf takes a char* and a size_t, this function takes pointers to those objects.  It
 * prints the buffer in the usual way but then adjusts the char* and size_t so that subsequent calls to this
 * function can pick up where the last one left off.
 *
 * @param output A pointer to the location of the next write.  This value is adjusted when the function
 * returns.
 * @param capacity A pointer to the remaining number of bytes left in the buffer starting at *output.  This
 * value is adjusted when the function returns.
 * @param format Same as for vasqSafeSnprintf.
 *
 * @return Same as for vasqSafeSnprintf.
 */
ssize_t
vasqIncSnprintf(char **output, size_t *capacity, const char *format, ...) VASQ_FORMAT(3);

/**
 * @brief Same as vasqIncSnprintf but takes a va_list instead of variable arguments.
 */
ssize_t
vasqIncVsnprintf(char **output, size_t *capacity, const char *format, va_list args) VASQ_NONNULL(3);

#endif  // VANILLA_SQUAD_SAFE_SNPRINTF_H
