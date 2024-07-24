#include "header.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

/*!
@code_uid{TUN_PRJ_Foo_ctor_01, constructor test}
@code_litem{Details}
This test checks that Foo_ctor() produces valid instance.
@code_bw_trace{brief}
- @tr{Foo_ctor()}
@endcode_uid
*/
void TUN_PRJ_Foo_ctor_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_EQUAL_INT32(0x12345678, Foo_inst.x);
    TEST_ASSERT_EQUAL_INT32(~0x12345678, Foo_inst.x_dis);
}

/*!
@code_uid{TUN_PRJ_Foo_verify_00, verify positive test}
@code_litem{Details}
This test checks that Foo_verify_() distinguishes valid instance.
@code_bw_trace{brief}
- @tr{Foo_verify_()}
@endcode_uid
*/
void TUN_PRJ_Foo_verify_00(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    TEST_ASSERT_TRUE(Foo_verify_(&Foo_inst));
}

/*!
@code_uid{TUN_PRJ_Foo_verify_01, verify negative test}
@code_litem{Details}
This test checks that Foo_verify_() distinguishes invalid instance.
@code_bw_trace{brief}
- @tr{Foo_verify_()}
@endcode_uid
*/
void TUN_PRJ_Foo_verify_01(void) {
    Foo_ctor(&Foo_inst, 0x12345678);
    Foo_inst.x -= 1; // change x
    TEST_ASSERT_FALSE(Foo_verify_(&Foo_inst));
}
