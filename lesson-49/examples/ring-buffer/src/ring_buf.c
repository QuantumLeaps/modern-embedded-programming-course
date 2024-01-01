/*============================================================================
* Lock-Free Ring Buffer (LFRB) for embedded systems
* GitHub: https://github.com/QuantumLeaps/lock-free-ring-buffer
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005 Quantum Leaps, <state-machine.com>.
*
* SPDX-License-Identifier: MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
============================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include "ring_buf.h"

/*..........................................................................*/
void RingBuf_ctor(RingBuf * const me,
                  RingBufElement sto[], RingBufCtr sto_len) {
    me->buf  = &sto[0];
    me->end  = sto_len;
    me->head = 0U;
    me->tail = 0U;
}
/*..........................................................................*/
bool RingBuf_put(RingBuf * const me, RingBufElement const el) {
    RingBufCtr head = me->head + 1U;
    if (head == me->end) {
        head = 0U;
    }
    if (head != me->tail) { /* buffer NOT full? */
        me->buf[me->head] = el; /* copy the element into the buffer */
        me->head = head; /* update the head to a *valid* index */
        return true;  /* element placed in the buffer */
    }
    else {
        return false; /* element NOT placed in the buffer */
    }
}
/*..........................................................................*/
bool RingBuf_get(RingBuf * const me, RingBufElement *pel) {
    RingBufCtr tail = me->tail;
    if (me->head != tail) { /* ring buffer NOT empty? */
        *pel = me->buf[tail];
        ++tail;
        if (tail == me->end) {
            tail = 0U;
        }
        me->tail = tail; /* update the tail to a *valid* index */
        return true;
    }
    else {
        return false;
    }
}
/*..........................................................................*/
RingBufCtr RingBuf_num_free(RingBuf * const me) {
    RingBufCtr head = me->head;
    RingBufCtr tail = me->tail;
    if (head == tail) { /* buffer empty? */
        return (RingBufCtr)(me->end - 1U);
    }
    else if (head < tail) {
        return (RingBufCtr)(tail - head - 1U);
    }
    else {
        return (RingBufCtr)(me->end + tail - head - 1U);
    }
}

/*..........................................................................*/
void RingBuf_process_all(RingBuf * const me, RingBufHandler handler) {
    RingBufCtr tail = me->tail;
    while (me->head != tail) { /* ring buffer NOT empty? */
        (*handler)(me->buf[tail]);
        ++tail;
        if (tail == me->end) {
            tail = 0U;
        }
        me->tail = tail; /* update the tail to a *valid* index */
    }
}
