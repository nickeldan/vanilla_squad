#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "vasq/safe_snprintf.h"

static unsigned int
numToBuffer(char *buffer, unsigned long long value);

static unsigned int
numToBufferHex(char *buffer, unsigned long long value, bool capitalize);

ssize_t
vasqSafeSnprintf(char *buffer, size_t size, const char *format, ...)
{
    ssize_t ret;
    va_list args;

    va_start(args,format);
    ret = vasqSafeVsnprintf(buffer,size,format,args);
    va_end(args);

    return ret;
}

ssize_t
vasqSafeVsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
    size_t so_far = 0;

    size--;
    for (; *format && size>0; format++) {
        char c;

        c = *format;
        if ( c == '%' ) {
            bool is_long;
            char subbuffer[20];
            unsigned int index = sizeof(subbuffer);

            c = *(++format);
            if ( c == '\0' ) {
                return -1;
            }

            if ( c == '%' ) {
                *(buffer++) = '%';
                so_far++;
                size--;
                continue;
            }

            if ( c == 's' ) {
                for (const char *string=va_arg(args,const char*); *string; string++) {
                    *(buffer++) = *string;
                    if ( --size == 0 ) {
                        goto done;
                    }
                }
                continue;
            }

            if ( c == '.' ) {
                unsigned int length;
                const char *string;

                if ( *(++format) != '*' ) {
                    return -1;
                }

                if ( *(++format) != 's' ) {
                    return -1;
                }

                length = va_arg(args, unsigned int);
                string = va_arg(args, const char*);

                for (unsigned int k=0; k<length; k++) {
                    c = *(string++);
                    if ( c == '\0' ) {
                        break;
                    }

                    *(buffer++) = c;
                    if ( --size == 0 ) {
                        goto done;
                    }
                }
                continue;
            }

            if ( c == 'c' ) {
                *(buffer++) = va_arg(args, int);
                if ( --size == 0 ) {
                    goto done;
                }
                continue;
            }

            if ( c == 'l' ) {
                is_long = true;
                c = *(++format);
                if ( c == '\0' ) {
                    return -1;
                }
            }
            else {
                is_long = false;
            }

            if ( c == 'u' ) {
                if ( is_long ) {
                    unsigned long value;

                    value = va_arg(args, unsigned long);
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else {
                    unsigned int value;

                    value = va_arg(args, unsigned int);
                    index -= numToBuffer(subbuffer+index-1, value);
                }
            }
            else if ( c == 'd' || c == 'i' ) {
                if ( is_long ) {
                    long value;

                    value = va_arg(args, long);
                    if ( value < 0 ) {
                        *(buffer++) = '-';
                        if ( --size == 0 ) {
                            break;
                        }
                        value *= -1;
                    }
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else {
                    int value;

                    value = va_arg(args, int);
                    if ( value < 0 ) {
                        *(buffer++) = '-';
                        if ( --size == 0 ) {
                            break;
                        }
                        value *= -1;
                    }
                    index -= numToBuffer(subbuffer+index-1, value);
                }
            }
            else if ( c == 'l' ) {
                // is_long must be true.

                c = *(++format);
                if ( c == '\0' ) {
                    return -1;
                }

                if ( c == 'i' ) {
                    long long value;

                    value = va_arg(args, long long);
                    if ( value < 0 ) {
                        *(buffer++) = '-';
                        if ( --size == 0 ) {
                            goto done;
                        }
                        value *= -1;
                    }
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else if ( c== 'u' ) {
                    unsigned long long value;

                    value = va_arg(args, unsigned long long);
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else {
                    return -1;
                }
            }
            else if ( c == 'z' ) {
                if ( is_long ) {
                    return -1;
                }

                c = *(++format);
                if ( c == '\0' ) {
                    return -1;
                }

                if ( c == 'u' ) {
                    size_t value;

                    value = va_arg(args, size_t);
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else if ( c == 'i' ) {
                    ssize_t value;

                    value = va_arg(args, ssize_t);
                    if ( value < 0 ) {
                        *(buffer++) = '-';
                        if ( --size == 0 ) {
                            goto done;
                        }
                        value *= -1;
                    }
                    index -= numToBuffer(subbuffer+index-1, value);
                }
                else {
                    return -1;
                }
            }
            else if ( c == 'p' ) {
                if ( is_long ) {
                    return -1;
                }

                *(buffer++) = '0';
                if ( --size == 0 ) {
                    goto done;
                }

                *(buffer++) = 'x';
                if ( --size == 0 ) {
                    goto done;
                }

                if ( sizeof(void*) == sizeof(uint64_t) ) {
                    uint64_t value;

                    value = va_arg(args, uint64_t);
                    index -= numToBufferHex(subbuffer+index-1, value, false);
                }
                else {
                    uint32_t value;

                    value = va_arg(args, uint32_t);
                    index -= numToBufferHex(subbuffer+index-1, value, false);
                }
            }
            else if ( isdigit(c) || c == 'x' || c == 'X' ) {
                unsigned int value;
                int min_length = 0;
                char padding;
                bool capitalize;

                if ( is_long ) {
                    return -1;
                }

                if ( c == '0' ) {
                    padding = '0';
                    c = *(++format);
                    if ( c == '\0' ) {
                        return -1;
                    }
                }
                else {
                    padding = ' ';
                }

                if ( isdigit(c) ) {
                    min_length = c - '0';
                    c = *(++format);
                    if ( c == '\0' ) {
                        return -1;
                    }
                }

                if ( c == 'x' ) {
                    capitalize = false;
                }
                else if ( c == 'X' ) {
                    capitalize = true;
                }
                else {
                    return -1;
                }

                value = va_arg(args, unsigned int);
                index -= numToBufferHex(subbuffer+index-1, value, capitalize);

                while ( (sizeof(subbuffer)-index) < (unsigned int)min_length && index > 0 ) {
                    subbuffer[index--] = padding;
                }
            }
            else {
                return -1;
            }

            while ( index < sizeof(subbuffer) ) {
                *(buffer++) = subbuffer[index++];
                if ( --size == 0 ) {
                    goto done;
                }
            }
        }
        else {
            *(buffer++) = c;
            so_far++;
            size--;
        }
    }

done:

    *buffer = '\0';
    return (ssize_t)so_far;
}

static unsigned int
numToBuffer(char *buffer, unsigned long long value)
{
    unsigned int ret = 0;

    while ( value > 0 ) {
        *(buffer--) = '0' + value%10;
        ret++;
        value /= 10;
    }

    return ret;
}

static unsigned int
numToBufferHex(char *buffer, unsigned long long value, bool capitalize)
{
    unsigned int ret = 0;
    char hex_letter;

    hex_letter = capitalize? 'A' : 'a';

    while ( value > 0 ) {
        int nibble;

        nibble = value&0x0f;
        *(buffer--) = ( nibble < 10 )? ( '0'+nibble ) :  (hex_letter+nibble-10 );
        ret++;
        value = ( value >> 4 );
    }

    return ret;
}
