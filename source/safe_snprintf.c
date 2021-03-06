#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "vasq/safe_snprintf.h"

static unsigned int
numToBuffer(char *buffer, uintmax_t value)
{
    unsigned int ret;

    for (ret = 0; value > 0; ret++) {
        *(buffer--) = '0' + value % 10;
        value /= 10;
    }

    return ret;
}

static unsigned int
numToBufferHex(char *buffer, uintmax_t value, bool capitalize)
{
    unsigned int ret;
    char hex_letter;

    hex_letter = capitalize ? 'A' : 'a';

    for (ret = 0; value > 0; ret++) {
        int nibble;

        nibble = value & 0x0f;
        *(buffer--) = (nibble < 10) ? ('0' + nibble) : (hex_letter + nibble - 10);
        value = (value >> 4);
    }

    return ret;
}

static bool
safeIsDigit(char c)
{
    return c >= '0' && c <= '9';
}

ssize_t
vasqSafeSnprintf(char *buffer, size_t size, const char *format, ...)
{
    ssize_t ret;
    va_list args;

    va_start(args, format);
    ret = vasqSafeVsnprintf(buffer, size, format, args);
    va_end(args);

    return ret;
}

ssize_t
vasqSafeVsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
    char *start = buffer;

    if (!buffer || size == 0 || !format) {
        return -1;
    }

    for (size--; *format && size > 0; format++) {  // The -- is to leave space for the null terminator.
        char c = *format;

        if (c == '%') {
            c = *(++format);
            if (c == '\0') {
                return -1;
            }
            else if (c == '%') {
                *(buffer++) = '%';
                size--;
            }
            else if (c == 's') {
                for (const char *string = va_arg(args, const char *); *string && size > 0; string++) {
                    *(buffer++) = *string;
                    size--;
                }
            }
            else if (c == '.') {
                unsigned int length;
                const char *string;

                if (*(++format) != '*') {
                    return -1;
                }

                if (*(++format) != 's') {
                    return -1;
                }

                length = va_arg(args, unsigned int);
                string = va_arg(args, const char *);

                for (unsigned int k = 0; k < length; k++) {
                    c = *(string++);
                    if (c == '\0') {
                        break;
                    }

                    *(buffer++) = c;
                    size--;
                }
            }
            else if (c == 'c') {
                *(buffer++) = va_arg(args, int);
                size--;
            }
            else {
                bool is_long;
                char subbuffer[39];  // Big enough to hold a 128-bit unsigned integer without the null
                                     // terminator.
                unsigned int index = sizeof(subbuffer);

                if (c == 'l') {
                    is_long = true;
                    c = *(++format);
                }
                else {
                    is_long = false;
                }

                if (c == 'u') {
                    if (is_long) {
                        unsigned long value;

                        value = va_arg(args, unsigned long);
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else {
                        unsigned int value;

                        value = va_arg(args, unsigned int);
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                }
                else if (c == 'd' || c == 'i') {
                    if (is_long) {
                        long value;

                        if (c == 'd') {
                            return -1;
                        }

                        value = va_arg(args, long);
                        if (value < 0) {
                            *(buffer++) = '-';
                            if (--size == 0) {
                                goto done;
                            }
                            value *= -1;
                        }
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else {
                        int value;

                        value = va_arg(args, int);
                        if (value < 0) {
                            *(buffer++) = '-';
                            if (--size == 0) {
                                goto done;
                            }
                            value *= -1;
                        }
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                }
                else if (c == 'l') {
                    // is_long must be true.

                    c = *(++format);
                    if (c == 'i') {
                        long long value;

                        value = va_arg(args, long long);
                        if (value < 0) {
                            *(buffer++) = '-';
                            if (--size == 0) {
                                goto done;
                            }
                            value *= -1;
                        }
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else if (c == 'u') {
                        unsigned long long value;

                        value = va_arg(args, unsigned long long);
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else {
                        return -1;
                    }
                }
                else if (c == 'z') {
                    if (is_long) {
                        return -1;
                    }

                    c = *(++format);
                    if (c == 'u') {
                        size_t value;

                        value = va_arg(args, size_t);
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else if (c == 'i') {
                        ssize_t value;

                        value = va_arg(args, ssize_t);
                        if (value < 0) {
                            *(buffer++) = '-';
                            if (--size == 0) {
                                goto done;
                            }
                            value *= -1;
                        }
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else {
                        return -1;
                    }
                }
                else if (c == 'j') {
                    if (is_long) {
                        return -1;
                    }

                    c = *(++format);
                    if (c == 'u') {
                        uintmax_t value;

                        value = va_arg(args, uintmax_t);
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else if (c == 'i') {
                        intmax_t value;

                        value = va_arg(args, intmax_t);
                        if (value < 0) {
                            *(buffer++) = '-';
                            if (--size == 0) {
                                goto done;
                            }
                            value *= -1;
                        }
                        index -= numToBuffer(subbuffer + index - 1, value);
                    }
                    else {
                        return -1;
                    }
                }
                else if (c == 'p') {
                    uintptr_t value;

                    if (is_long) {
                        return -1;
                    }

                    *(buffer++) = '0';
                    if (--size == 0) {
                        goto done;
                    }

                    *(buffer++) = 'x';
                    if (--size == 0) {
                        goto done;
                    }

                    value = (uintptr_t)va_arg(args, void *);
                    index -= numToBufferHex(subbuffer + index - 1, value, false);
                }
                else if (safeIsDigit(c) || c == 'x' || c == 'X') {
                    unsigned int value;
                    int min_length;
                    char padding;
                    bool capitalize;

                    if (is_long) {
                        return -1;
                    }

                    if (c == '0') {
                        padding = '0';
                        c = *(++format);
                    }
                    else {
                        padding = ' ';
                    }

                    if (safeIsDigit(c)) {
                        min_length = c - '0';
                        c = *(++format);
                    }
                    else {
                        min_length = 0;
                    }

                    if (c == 'x') {
                        capitalize = false;
                    }
                    else if (c == 'X') {
                        capitalize = true;
                    }
                    else {
                        return -1;
                    }

                    value = va_arg(args, unsigned int);
                    index -= numToBufferHex(subbuffer + index - 1, value, capitalize);

                    if (index == sizeof(subbuffer)) {
                        subbuffer[--index] = '0';
                    }

                    while ((sizeof(subbuffer) - index) < (unsigned int)min_length && index > 0) {
                        subbuffer[--index] = padding;
                    }
                }
                else {
                    return -1;
                }

                if (index == sizeof(subbuffer)) {
                    subbuffer[--index] = '0';
                }

                while (index < sizeof(subbuffer)) {
                    *(buffer++) = subbuffer[index++];
                    if (--size == 0) {
                        goto done;
                    }
                }
            }
        }
        else {
            *(buffer++) = c;
            size--;
        }
    }

done:

    *buffer = '\0';
    return buffer - start;
}

ssize_t
vasqIncSnprintf(char **output, size_t *capacity, const char *format, ...)
{
    ssize_t ret;
    va_list args;

    va_start(args, format);
    ret = vasqIncVsnprintf(output, capacity, format, args);
    va_end(args);

    return ret;
}

ssize_t
vasqIncVsnprintf(char **output, size_t *capacity, const char *format, va_list args)
{
    ssize_t ret;

    if (!output || !capacity) {
        return -1;
    }

    ret = vasqSafeVsnprintf(*output, *capacity, format, args);
    if (ret > 0) {
        *output += ret;
        *capacity -= ret;
    }

    return ret;
}
