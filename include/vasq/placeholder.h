/**
 * @file placeholder.h
 * @author Daniel Walker
 * @brief Defines the PLACEHOLDER macro.
 */
#ifndef VANILLA_SQUAD_PLACEHOLDER_H
#define VANILLA_SQUAD_PLACEHOLDER_H

#include "config.h"
#include "definitions.h"

#ifdef __USE_ISOC99

#if defined(VASQ_REJECT_PLACEHOLDER) || (!defined(DEBUG) && !defined(VASQ_ALLOW_PLACEHOLDER))

#define PLACEHOLDER() _Pragma("GCC error \"Placeholder code left in project.\"")

#elif defined(VASQ_WARN_PLACEHOLDER)

#define PLACEHOLDER() _Pragma("GCC warning \"Placeholder code left in project.\"")

#else

#define PLACEHOLDER() NO_OP

#endif

#else  // __USE_ISOC99

#warning "_Pragma not defined and so PLACEHOLDER() will not work."
#define PLACEHOLDER() NO_OP

#endif  // __USE_ISOC99

#endif  // VANILLA_SQUAD_PLACEHOLDER_H
