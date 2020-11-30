#pragma once

#include <stdarg.h>

#include "definitions.h"

ssize_t
vasqSafeSnprintf(char *buffer, size_t size, const char *format, ...);

ssize_t
vasqSafeVsnprintf(char *buffer, size_t size, const char *format, va_list args);
