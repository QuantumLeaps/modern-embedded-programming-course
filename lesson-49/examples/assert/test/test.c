/*============================================================================
* ET: embedded test; very simple test example
============================================================================*/
#include "div.h" /* Code Under Test (CUT) */
#include "et.h"  /* ET: embedded test */
#include "dbc_assert.h" /* Design By Contract (DBC) assertions */

void setup(void) {
    /* executed before *every* non-skipped test */
}

void teardown(void) {
    /* executed after *every* non-skipped and non-failing test */
}

/* test group --------------------------------------------------------------*/
TEST_GROUP("Basic") {

TEST("CUT test (passing)") {
    VERIFY(3 == div(9, 3));
    VERIFY(3 == div(10, 3));
}

SKIP_TEST("test (skipped)") {
    VERIFY(3 == 2*2); /* this would fail, but it's not checked */
}

TEST("CUT test (expected assertion)") {
    ET_expect_assert("div", 100);
    VERIFY(0 == div(2, 0)); /* <--- causes assertion, but succeeds */
}

/* this test follows an assert test and is NOT executed */
TEST("simple test (passing)") {
    VERIFY(4 == 2*2);
}

} /* TEST_GROUP() */

/*..........................................................................*/
DBC_NORETURN
void DBC_fault_handler(char const * module, int label) {
    VERIFY_ASSERT(module, label);
    for (;;) { /* explicitly make it "noreturn" */
    }
}
