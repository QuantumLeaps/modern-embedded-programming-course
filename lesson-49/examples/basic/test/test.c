/*============================================================================
* ET: embedded test; very simple test example
============================================================================*/
#include "sum.h" /* Code Under Test (CUT) */
#include "et.h" /* ET: embedded test */

void setup(void) {
    /* executed before *every* non-skipped test */
}

void teardown(void) {
    /* executed after *every* non-skipped and non-failing test */
}

/* test group --------------------------------------------------------------*/
TEST_GROUP("Basic") {

TEST("first test (passing)") {
    VERIFY(4 == 2*2);
}

TEST("CUT test (passing)") {
    VERIFY(5 == sum(2, 3));
    VERIFY(5 == sum(3, 2));
}

SKIP_TEST("test (skipped)") {
    VERIFY(3 == 2*2); /* this would fail, but it's not checked */
}

TEST("CUT test (failing)") {
    VERIFY(5 == sum(2, 3));
    VERIFY(4 == sum(3, 2)); /* <--- fails */
}

/* this test follows a failing test and is NOT executed */
TEST("simple test (passing)") {
    VERIFY(4 == 2*2);
}

} /* TEST_GROUP() */
