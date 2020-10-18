#ifndef __VANILLA_SQUAD_UNITTEST_H__
#define __VANILLA_SQUAD_UNITTEST_H__

#include <stdio.h>
#include <stdarg.h>

#include "definitions.h"
#include "logger.h"

typedef struct vasqTester {
    FILE *out;
    unsigned int num_errors;
    bool success;
} vasqTester;

int
vasqTesterInit(vasqTester *tester, FILE *out);

void
vasqTesterDestroy(vasqTester *tester);

#ifndef VASQ_TESTER_NAME
#define VASQ_TESTER_NAME tester
#endif

#define VASQ_TEST_FUNCTION(func_name) void func_name(vasqTester *VASQ_TESTER_NAME)
#define VASQ_SUB_TEST_FUNCTION(func_name,...) void func_name(vasqTester *VASQ_TESTER_NAME, ##__VA_ARGS__)

#define VASQ_RUN_TEST(tester,func) do { \
    (tester)->success = true; \
    (func)(tester); \
} while (0)

#define VASQ_ASSERT_MSG(expr,format,...) do { \
    if ( !(expr) ) { \
        VASQ_ERROR("Assertion failed: %s", #expr); \
        VASQ_ERROR(format,##__VA_ARGS__); \
        VASQ_TESTER_NAME->success = false; \
        VASQ_TESTER_NAME->num_errors++; \
        goto test_cleanup; \
    } \
} while (0)

#define VASQ_ASSERT(expr) do { \
    if ( !(expr) ) { \
        VASQ_ERROR("Assertion failed: %s", #expr); \
        VASQ_TESTER_NAME->success = false; \
        VASQ_TESTER_NAME->num_errors++; \
        goto test_cleanup; \
    } \
} while (0)

#define VASQ_RUN_SUB_TEST(func,...) do { \
    (func)(VASQ_TESTER_NAME,##__VA_ARGS__); \
    if ( !VASQ_TESTER_NAME->success ) { \
        VASQ_ERROR("Sub-test failed."); \
        goto test_cleanup; \
    } \
} while (0)

#endif // __VANILLA_SQUAD_UNITTEST_H__
