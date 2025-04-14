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

#if defined(__TI_COMPILER_VERSION__) || defined(__GNUC__)
    // no-return function specifier (C11 Standard)
    #define Q_NORETURN   _Noreturn void
#elif defined(__IAR_SYSTEMS_ICC__)
    #define Q_NORETURN   __noreturn void
#endif

// QF configuration for QV -- data members of the QActive class...

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE     QEQueue
// QACTIVE_OS_OBJ_TYPE not used in this port
// QACTIVE_THREAD_TYPE not used in this port

// QF interrupt disable/enable...
#define QF_INT_DISABLE()        __disable_interrupt()
#define QF_INT_ENABLE()         __enable_interrupt()

// QF critical section entry/exit...
#define QF_CRIT_STAT            unsigned short int_state_;
#define QF_CRIT_ENTRY() do {               \
    int_state_ =  __get_interrupt_state(); \
    __disable_interrupt();                 \
} while (false)

#define QF_CRIT_EXIT()          __set_interrupt_state(int_state_)

// include files -------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#elif defined(__GNUC__)
    #include <msp430.h>
    #include "in430.h"
#endif

#include "qequeue.h"   // QV kernel uses the native QP event queue
#include "qmpool.h"    // QV kernel uses the native QP memory pool
#include "qp.h"        // QP framework
#include "qv.h"        // QV kernel

#endif // QP_PORT_H_

