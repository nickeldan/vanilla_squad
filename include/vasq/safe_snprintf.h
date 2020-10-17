#ifndef __VANILLA_SQUAD_SAFE_SNPRINTF_H__
#define __VANILLA_SQUAD_SAFE_SNPRINTF_H__

#include <stdarg.h>

#include "definitions.h"

ssize_t
safe_snprintf(char *buffer, size_t size, const char *format, ...);

ssize_t
safe_vsnprintf(char *buffer, size_t size, const char *format, va_arg args);

#endif // __VANILLA_SQUAD_SAFE_SNPRINTF_H__
