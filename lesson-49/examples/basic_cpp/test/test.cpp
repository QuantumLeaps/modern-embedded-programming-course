//============================================================================
// ET: embedded test; basic C++ test example
//============================================================================
#include "sum.hpp" // Code Under Test (CUT)
#include "et.h"    // ET: embedded test
#include "dbc_assert.h" // Design By Contract (DBC) assertions

extern "C" {

static CUT::Sum sum_inst(123); // instance Sum class

void setup(void) {
    // executed before *every* non-skipped test
    sum_inst.clear();
}

void teardown(void) {
    // executed after *every* non-skipped and non-failing test
    sum_inst.clear();
}

// test group ----------------------------------------------------------------
TEST_GROUP("Basic") {

TEST("CUT test (passing)") {
    sum_inst.add(2);
    VERIFY(5 == sum_inst.add(3));
}

SKIP_TEST("test (skipped)") {
    VERIFY(3 == 2*2); // <-- would fail, but it's not executed
}

TEST("CUT test (failing)") {
    sum_inst.add(3);
    VERIFY(5 == sum_inst.add(2));
    VERIFY(4 == sum_inst.get()); // <-- should fail
}

// this test follows a failing test and is NOT executed
TEST("simple test (passing)") {
    VERIFY(4 == 2*2);
}

} // TEST_GROUP()

} // extern "C"

//============================================================================
// dependencies for the CUT

extern "C" {

//............................................................................
DBC_NORETURN
void DBC_fault_handler(char const * module, int label) {
    VERIFY_ASSERT(module, label);
    for (;;) { /* explicitly make it "noreturn" */
    }
}

} // extern "C"

