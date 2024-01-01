/*============================================================================
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2021 Quantum Leaps, LLC. All rights reserved.
*
* SPDX-License-Identifier: MIT
*
* Contact information:
* <www.state-machine.com>
* <info@state-machine.com>
============================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include "ring_buf.h"
#include "et.h" /* ET: embedded test */

RingBufElement buf[8];
RingBuf rb;

/* ring-buffer "handler" function for RingBuf_process_all() */
static void rb_handler(RingBufElement const el);

static RingBufElement test_data[] = {
    0xAAU,
    0xBBU,
    0xCCU,
    0xDDU
};
static RingBufCtr test_idx;

void setup(void) {
    /* executed before *every* non-skipped test */
}

void teardown(void) {
    /* executed after *every* non-skipped and non-failing test */
}

/* test group --------------------------------------------------------------*/
TEST_GROUP("lock-free ring buffer") {

RingBuf_ctor(&rb, buf, ARRAY_NELEM(buf));

TEST("RingBuf_num_free") {
    VERIFY(RingBuf_num_free(&rb) == ARRAY_NELEM(buf) - 1U);
}

TEST("RingBuf_put 3") {
    RingBuf_put(&rb, 0xAAU);
    RingBuf_put(&rb, 0xBBU);
    RingBuf_put(&rb, 0xCCU);
    VERIFY(RingBuf_num_free(&rb) == ARRAY_NELEM(buf) - 1U - 3U);
}

TEST("RingBuf_get") {
    RingBufElement el;
    VERIFY(true == RingBuf_get(&rb, &el));
    VERIFY(0xAAU == el);
    VERIFY(true == RingBuf_get(&rb, &el));
    VERIFY(0xBBU == el);
    VERIFY(true == RingBuf_get(&rb, &el));
    VERIFY(0xCCU == el);
    VERIFY(false == RingBuf_get(&rb, &el));
}

TEST("RingBuf_process_all test_data") {
    for (RingBufCtr i = 0U; i < ARRAY_NELEM(test_data); ++i) {
        RingBuf_put(&rb, test_data[i]);
    }
    test_idx = 0U;
    RingBuf_process_all(&rb, &rb_handler);
    VERIFY(RingBuf_num_free(&rb) == ARRAY_NELEM(buf) - 1U);
}

} /* TEST_GROUP() */

static void rb_handler(RingBufElement const el) {
    VERIFY(test_data[test_idx] == el);
    ++test_idx;
}

