#ifndef __VANILLA_SQUAD_SAFE_SNPRINTF_H__
#define __VANILLA_SQUAD_SAFE_SNPRINTF_H__

#include <stdarg.h>

#include "definitions.h"

ssize_t
vasqSafeSnprintf(char *buffer, size_t size, const char *format, ...);

ssize_t
vasqSafeVsnprintf(char *buffer, size_t size, const char *format, va_list args);

#endif  // __VANILLA_SQUAD_SAFE_SNPRINTF_H__
