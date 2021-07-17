#ifndef VANILLA_SQUAD_PLACEHOLDER_H
#define VANILLA_SQUAD_PLACEHOLDER_H

#include <features.h>

#include "config.h"
#include "definitions.h"

#ifdef __USE_ISOC99

#if (defined(DEBUG) || defined(VASQ_ALLOW_PLACEHOLDER)) && !defined(VASQ_REJECT_PLACEHOLDER)

#ifdef VASQ_WARN_PLACEHOLDER

#ifdef VASQ_REJECT_PLACEHOLDER
#warning "VASQ_WARN_PLACEHOLDER and VASQ_REJECT_PLACEHOLDER are both defined."
#endif

#define PLACEHOLDER() _Pragma("GCC warning \"Placeholder code left in project.\"")

#else  // VASQ_WARN_PLACEHOLDER

#define PLACEHOLDER() NO_OP

#endif  // VASQ_WARN_PLACEHOLDER

#else  // (defined(DEBUG) || defined(VASQ_ALLOW_PLACEHOLDER)) && !defined(VASQ_REJECT_PLACEHOLDER)

#ifdef VASQ_ALLOW_PLACEHOLDER
#warning "VASQ_ALLOW_PLACEHOLDER and VASQ_REJECT_PLACEHOLDER are both defined."
#endif

#define PLACEHOLDER() _Pragma("GCC error \"Placeholder code left in project.\"")

#endif  // (defined(DEBUG) || defined(VASQ_ALLOW_PLACEHOLDER)) && !defined(VASQ_REJECT_PLACEHOLDER)

#else  // __USE_ISOC99

#warning "_Pragma not defined and so PLACEHOLDER() will not work."
#define PLACEHOLDER() NO_OP

#endif  // __USE_ISOC99

#endif  // VANILLA_SQUAD_PLACEHOLDER_H
