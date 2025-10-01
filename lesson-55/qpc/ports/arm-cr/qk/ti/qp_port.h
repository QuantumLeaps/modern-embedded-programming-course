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

// QF configuration for QK -- data members of the QActive class...

// QK event-queue used for AOs
#define QACTIVE_EQUEUE_TYPE     QEQueue

// QF interrupt disable/enable, see NOTE2
#ifdef __16bis__
    #define QF_INT_DISABLE()    __asm(" CPSID i")
    #define QF_INT_ENABLE()     __asm(" CPSIE i")
    #define QF_INT_ENABLE_ALL() __asm(" CPSIE if")
#else
    #define QF_INT_DISABLE()    _disable_IRQ()
    #define QF_INT_ENABLE()     _enable_IRQ()
    #define QF_INT_ENABLE_ALL() _enable_interrupts()
#endif

// Cortex-R provide the CLZ instruction for fast LOG2
#define QF_LOG2(n_) ((uint8_t)(32U - __clz(n_)))

// QF critical section entry/exit, see NOTE3
#define QF_CRIT_STAT            uint32_t cpsr_;
#define QF_CRIT_ENTRY() do {          \
    cpsr_ = _get_CPSR();              \
    QF_INT_DISABLE();                 \
} while (false)
#define QF_CRIT_EXIT()  do {          \
    if ((cpsr_ & (1U << 7U)) == 0U) { \
        QF_INT_ENABLE();              \
    }                                 \
} while (false)

#define QF_CRIT_EST()          QF_INT_DISABLE()
#define QF_CRIT_EXIT_NOP()     __asm(" ISB")

// Check if the code executes in the ISR context
#define QK_ISR_CONTEXT_() (QK_priv_.intNest != 0U)

// QK-specific Interrupt Request handler BEGIN
#define QK_IRQ_BEGIN(name_)              \
    void name_(void);                    \
    __asm(" .def " #name_ "\n"           \
    " .arm\n"                            \
    " .align 4\n"                        \
    " .armfunc " #name_ "\n"             \
    #name_":\n"                          \
    " SUB LR, LR, #4\n"                  \
    " SRSDB #31!\n"                      \
    " CPS #31\n"                         \
    " PUSH {R0-R3, R12}\n"               \
    " .if __TI_VFPV3D16_SUPPORT__ = 1\n" \
    " FMRX R12, FPSCR\n"                 \
    " STMFD SP!, {R12}\n"                \
    " FMRX R12, FPEXC\n"                 \
    " STMFD SP!, {R12}\n"                \
    " FSTMDBD SP!, {D0-D7}\n"            \
    " .endif\n"                          \
    " AND R3, SP, #4\n"                  \
    " SUB SP, SP, R3\n"                  \
    " PUSH {R3, LR}\n"                   \
    " BLX " #name_ "_isr\n"              \
    " POP {R3, LR}\n"                    \
    " ADD SP, SP, R3\n"                  \
    " .if __TI_VFPV3D16_SUPPORT__ = 1\n" \
    " FLDMIAD SP!, {D0-D7}\n"            \
    " LDMFD SP!, {R12}\n"                \
    " FMXR FPEXC, R12 \n"                \
    " LDMFD SP!, {R12} \n"               \
    " FMXR FPSCR, R12\n"                 \
    " .endif\n"                          \
    " POP {R0-R3, R12}\n"                \
    " RFEIA SP!\n");                     \
    void name_ ## _isr(void) {           \
    ++QK_priv_.intNest; {

// QK-specific Interrupt Request handler END
#define QK_IRQ_END()              \
    } --QK_priv_.intNest;         \
    if (QK_priv_.intNest == 0U) { \
        if (QK_sched_() != 0U) {  \
            QK_activate_();       \
        }                         \
    }                             \
}

// include files -------------------------------------------------------------
#include "qequeue.h"   // QK kernel uses the native QP event queue
#include "qmpool.h"    // QK kernel uses the native QP memory pool
#include "qp.h"        // QP framework
#include "qk.h"        // QK kernel

//============================================================================
// NOTE2:
// The FIQ-type interrupts are NEVER disabled in this port, so the FIQ is
// a "kernel-unaware" interrupt. If the FIQ is ever used in the application,
// it must be an "ARM FIQ"-type function. For this to work, the FIQ
// stack needs to be initialized.
//
// NOTE3:
// This port implements the "save and restore" interrupt status policy,
// which again never disables the FIQ-type interrupts. This policy allows
// for nesting critical sections, which is necessary inside IRQ-type
// interrupts that run with interrupts (IRQ) disabled.
//

#endif // QP_PORT_H_

