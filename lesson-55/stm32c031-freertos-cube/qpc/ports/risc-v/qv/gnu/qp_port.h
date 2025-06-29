//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>     // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h>    // Boolean type.      WG14/N843 C99 Standard
#include "qp_config.h"  // QP configuration from the application

// no-return function specifier (C11 Standard)
#define Q_NORETURN   _Noreturn void

// interrupt disabling policy, see NOTE2
#define QF_INT_DISABLE()       __asm__ volatile("csrc mstatus,8")
#define QF_INT_ENABLE()        __asm__ volatile("csrs mstatus,8")

// QF critical section entry/exit, see NOTE3
#define QF_CRIT_STAT           unsigned int mstatus_;
#define QF_CRIT_ENTRY()  do { \
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus_)); \
    QF_INT_DISABLE(); \
} while (false)

#define QF_CRIT_EST()          QF_INT_DISABLE()
#define QF_CRIT_EXIT() \
    __asm__ volatile ("csrw mstatus, %0" :: "rK"(mstatus_))

// support for fast LOG2, see NOTE4
#define QF_LOG2(n_) ((uint_fast8_t)(32U - __builtin_clz((n_))))

// NOP instruction for RISC-V, NOTE5
#define QF_CRIT_EXIT_NOP()     __asm__ volatile ("nop")

// include files -------------------------------------------------------------
#include "qequeue.h"   // QV kernel uses the native QP event queue
#include "qmpool.h"    // QV kernel uses the native QP memory pool
#include "qp.h"        // QP framework
#include "qv.h"        // QV kernel

//============================================================================
// NOTE2:
// Interrupts are disabled by clearing the MIE (bit-3) in the mstatus register.
// Interrupts are enabled by setting the MIE (bit-3) in the mstatus register.
//
// NOTE3:
// QP critical section uses the "save and restore interrupt status" policy.
// This policy is appropriate for CPUs that disable interrupts inside ISRs,
// which is the case with RISC-V. Also, this policy allows *nesting* of
// critical sections.
//
// NOTE4:
// The __builtin_clz() function is used for the integer log-base-2
// implementation, which is used in the QV scheduler.
//
// NOTE5:
// The NOP instruction is used to prevent merging of back-to-back critical
// sections.
//

#endif // QP_PORT_H_

