#include "header.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

/*!
@code_uid{TUN_PRJ_free_fun_00, zero input test}
@code_litem{Details}
This test checks that zero input to free_fun() produces zero array.
@code_bw_trace{brief}
- @tr{free_fun()}
@endcode_uid
*/
void TUN_PRJ_free_fun_00(void) {
    uint8_t const *result = free_fun(0U);
    TEST_ASSERT_EQUAL_UINT8(0, result[0]);
    TEST_ASSERT_EQUAL_UINT8(0, result[1]);
    TEST_ASSERT_EQUAL_UINT8(0, result[2]);
    TEST_ASSERT_EQUAL_UINT8(0, result[3]);
}

/*!
@code_uid{TUN_PRJ_free_fun_01, non-zero input test}
@code_litem{Details}
This test checks that non-zero input to free_fun() produces expected array.
@code_bw_trace
- @tr{free_fun()}
@endcode_uid
*/
void TUN_PRJ_free_fun_01(void) {
    uint8_t const* result = free_fun(0x1A2B3C4CU);
    TEST_ASSERT_EQUAL_UINT8(0x1A, result[0]);
    TEST_ASSERT_EQUAL_UINT8(0x2B, result[1]);
    TEST_ASSERT_EQUAL_UINT8(0x3C, result[2]);
    TEST_ASSERT_EQUAL_UINT8(0x4D, result[3]);
}
