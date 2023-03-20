#ifndef VANILLA_SQUAD_CONFIG_H
#define VANILLA_SQUAD_CONFIG_H

// Causes all logging logic to be removed during preprocessing.
// #define VASQ_NO_LOGGING

// The maximum number of bytes which will be written by a log statement.
#ifndef VASQ_LOGGING_LENGTH
#define VASQ_LOGGING_LENGTH 1024
#endif

// The maximum number of bytes displayed by a hex dump.  Any bytes past this limit are replaced by an
// ellipsis.
#ifndef VASQ_HEXDUMP_SIZE
#define VASQ_HEXDUMP_SIZE 1024
#endif

// The width of hex dumps.  Must divide into VASQ_HEXDUMP_SIZE.
#ifndef VASQ_HEXDUMP_WIDTH
#define VASQ_HEXDUMP_WIDTH 16
#endif

// Causes the PLACEHOLDER() macro to generate an error when used even if DEBUG or VASQ_ALLOW_PLACEHOLDER are
// defined.
// #define VASQ_REJECT_PLACEHOLDER

// If other macros are defined such that PLACEHOLDER() would normally resolve to a no op, this macro causes
// the PLACEHOLDER() macro to generate a warning when used.
// #define VASQ_WARN_PLACEHOLDER

#endif  // VANILLA_SQUAD_CONFIG_H
