#pragma once

#define VASQ_VERSION "7.1.0"

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
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifdef __GNUC__

#define VASQ_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define VASQ_FORMAT(pos)  __attribute__((format(printf, pos, pos + 1)))
#define VASQ_MALLOC       __attribute__((malloc))

#else

#define VASQ_NONNULL(...)
#define VASQ_FORMAT(pos)
#define VASQ_MALLOC

#endif
