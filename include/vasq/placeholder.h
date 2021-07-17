#ifndef VANILLA_SQUAD_PLACEHOLDER_H
#define VANILLA_SQUAD_PLACEHOLDER_H

#include "definitions.h"

#if defined(DEBUG) || defined(VASQ_ALLOW_PLACEHOLDER)

#define PLACEHOLDER() NO_OP

#else

#include <features.h>

#ifdef __USE_ISOC11

#include <assert.h>

#define PLACEHOLDER() _Static_assert(0, "Placeholder code left in project.")

#else // __USE_ISOC11

#warning "_Static_assert not defined and so PLACEHOLDER() will not work."
#define PLACEHOLDER() NO_OP

#endif // __USE_ISOC11

#endif // defined(DEBUG) || defined(VASQ_ALLOW_PLACEHOLDER)

#endif // VANILLA_SQUAD_PLACEHOLDER_H