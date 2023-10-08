/**
 * @file placeholder.h
 * @author Daniel Walker
 * @brief Defines the PLACEHOLDER macro.
 */
#pragma once

#include "config.h"
#include "definitions.h"

#if __STDC_VERSION__ >= 199901L

#if defined(VASQ_REJECT_PLACEHOLDER) || (!defined(DEBUG) && !defined(VASQ_ALLOW_PLACEHOLDER))

#define PLACEHOLDER() _Pragma("GCC error \"Placeholder code left in project.\"")

#elif defined(VASQ_WARN_PLACEHOLDER)

#define PLACEHOLDER() _Pragma("GCC warning \"Placeholder code left in project.\"")

#else

#define PLACEHOLDER() NO_OP

#endif

#else  // __STDC_VERSION__

#warning "_Pragma not defined and so PLACEHOLDER() will not work."
#define PLACEHOLDER() NO_OP

#endif  // __STDC_VERSION__
