#include "vasq/unittest.h"
#include "vasq/logger.h"

void
vasqTesterInit(vasqTester *tester)
{
    if ( tester ) {
        tester->num_errors = 0;
    }
}
