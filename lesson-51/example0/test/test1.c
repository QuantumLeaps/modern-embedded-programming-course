#include "header.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void TUN_PRJ_free_fun_00(void) {
    uint8_t const *result = free_fun(0U);
    TEST_ASSERT_EQUAL_UINT8(0, result[0]);
    TEST_ASSERT_EQUAL_UINT8(0, result[1]);
    TEST_ASSERT_EQUAL_UINT8(0, result[2]);
    TEST_ASSERT_EQUAL_UINT8(0, result[3]);
}

void TUN_PRJ_free_fun_01(void) {
    uint8_t const* result = free_fun(0x1A2B3C4CU);
    TEST_ASSERT_EQUAL_UINT8(0x1A, result[0]);
    TEST_ASSERT_EQUAL_UINT8(0x2B, result[1]);
    TEST_ASSERT_EQUAL_UINT8(0x3C, result[2]);
    TEST_ASSERT_EQUAL_UINT8(0x4D, result[3]);
}
