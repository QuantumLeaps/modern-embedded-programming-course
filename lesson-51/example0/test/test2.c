#include "header.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void TUN_PRJ_Foo_ctor_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_EQUAL_INT32(0x12345678, Foo_inst.x);
    TEST_ASSERT_EQUAL_INT32(~0x12345678, Foo_inst.x_dis);
}

void TUN_PRJ_Foo_verify_00(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_TRUE(Foo_verify_(&Foo_inst));
}

void TUN_PRJ_Foo_verify_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    Foo_inst.x -= 1; // change x
    TEST_ASSERT_FALSE(Foo_verify_(&Foo_inst));
}
