#include "vasq/unittest.h"

void
vasqTesterInit(vasqTester* tester) {
    if (tester) {
        tester->num_errors = 0;
    }
}
