#include "header.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

/*!
@brief constructor test
@par Details
This test checks that Foo_ctor() produces valid instance.
*/
void TUN_PRJ_Foo_ctor_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_EQUAL_INT32(0x12345678, Foo_inst.x);
    TEST_ASSERT_EQUAL_INT32(~0x12345678, Foo_inst.x_dis);
}

/*!
@brief verify positive test}
@par Details
This test checks that Foo_verify_() distinguishes valid instance.
*/
void TUN_PRJ_Foo_verify_00(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_TRUE(Foo_verify_(&Foo_inst));
}

/*!
@brief verify negative test}
@par Details
This test checks that Foo_verify_() distinguishes invalid instance.
*/
void TUN_PRJ_Foo_verify_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    Foo_inst.x -= 1; // change x
    TEST_ASSERT_FALSE(Foo_verify_(&Foo_inst));
}
