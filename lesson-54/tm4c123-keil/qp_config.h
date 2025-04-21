//============================================================================
// QP configuration file (QV/QK/QXK on ARM Cortex-M)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
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
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_CONFIG_H_
#define QP_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

// <o>QP API compatibility version (QP_API_VERSION)
//   <0=>  0   (Maximum compatibility)
//   <691=>691 (QP 6.9.1 or newer)
//   <734=>7.3.4 (QP 7.3.4 or newer)
//   <9999=>9999 (Latest only)
// <i>QP API backwards compatibility with the QP API version.
// <i>Lower QP_API_VERSION values enable backwards compatibility
// <i>with lower (older) QP API versions.
// <i>For example, QP_API_VERSION==691 will enable the compatibility
// <i>layer with QP version 6.9.1 and newer, but not older than 6.9.1.
// <i>QP_API_VERSION==0 enables the maximum currently supported
// <i>backwards compatibility. Conversely, QP_API_VERSION==9999 means
// <i>that no backwards compatibility layer should be enabled.
// <i>Default: 0 (All supported)
#define QP_API_VERSION 0

//..........................................................................
// <h>QP Functional Safety (FuSa) Subsystem (Q_UNSAFE)
// <i>The QP FuSa Subsystem consists of the following facilities:
// <i>- Software assertions as a recommended technique
// <i>  (called Failure Assertion Programming (FAP) in IEC 61508)
// <i>- Software Self-Monitoring (SSM), which encompasses such techniques:
// <i>  * Duplicate Inverse Storage for critical variables
// <i>  * Memory Markers for critical objects (e.g., events)
// <i>  * Hard-limits for all loops
// <i>  * Memory Isolation by means of Memory Protection Unit (MPU)

// <c3>Disable QP FuSa in development (NOT recommended)
// <i>Disable assertions and other self monitoring features
// <i>in development build configurations (NDEBUG undefined).
// <i>NOTE: Disabling safety *violates* functional safety standards.
//#ifndef NDEBUG
//#define Q_UNSAFE
//#endif
// </c>

// <c3>Disable QP FuSa in production release (NOT recommended)
// <i>Disable assertions and other safety features
// <i>in the release build configurations (NDEBUG defined).
// <i>NOTE: Disabling safety *violates* functional safety standards.
//#ifdef NDEBUG
//#define Q_UNSAFE
//#endif
// </c>

// </h>

//..........................................................................
// <h>QEP Event Processor (Events)
// <i>Events and state machines.

// <o>Event signal size (Q_SIGNAL_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of the QEvt signal for QEP/QF [bytes]
// <i>Default: 2
#define Q_SIGNAL_SIZE  2U

// </h>

//..........................................................................
// <h>QF Framework (Active Objects)
// <i>Active Object framework

// <o>Maximum # Active Objects (QF_MAX_ACTIVE) <1-64>
// <i>Maximum # Active Objects in the system <1..64>
// <i>Default: 32
#define QF_MAX_ACTIVE  32U

// <o>Maximum # event pools (QF_MAX_EPOOL)
// <0=>0 no event pools
// <1=>1 <2=>2 <3=>3 (default) <4=>4 <5=>5
// <6=>6 <7=>7 <8=>8 <9=>9 <10=>10 <11=>11
// <12=>12 <13=>13 <14=>14 <15=>15
// <i>Maximum # Event Pools <1..15>
// <i>Default: 3
#define QF_MAX_EPOOL 3U

// <o>Maximum # clock tick rates (QF_MAX_TICK_RATE)
// <0=>0 no time events
// <1=>1 (default) <2=>2 <3=>3 <4=>4 <5=>5
// <6=>6 <7=>7 <8=>8 <9=>9 <10=>10 <11=>11
// <12=>12 <13=>13 <14=>14 <15=>15
// <i>Maximum # clock tick rates for time events <1..15>
// <i>Default: 1
#define QF_MAX_TICK_RATE 1U

// <o>Event size (QF_EVENT_SIZ_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of the dynamic events for QF [bytes]
// <i>Default: 2 (64K bytes maximum event size)
#define QF_EVENT_SIZ_SIZE   2U

// <o>Time event counter size (QF_TIMEEVT_CTR_SIZE)
//   <1U=>1
//   <2U=>2
//   <4U=>4 (default)
// <i>Size of the QTimeEvt counter [bytes]
// <i>Default: 4 (2^32 dynamic range)
#define QF_TIMEEVT_CTR_SIZE 4U

// <o>Event queue counter size (QF_EQUEUE_CTR_SIZE)
//   <1U=>1 (default)
//   <2U=>2
// <i>Size of event queue counter [bytes]
// <i>Default: 1 (255 events maximum in a queue)
#define QF_EQUEUE_CTR_SIZE  1U

// <o>Memory pool counter size (QF_MPOOL_CTR_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of memory pool counter [bytes]
// <i>Default: 2 (64K blocks maximum in a pool)
#define QF_MPOOL_CTR_SIZE 2U

// <o>Memory block size (QF_MPOOL_SIZ_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of memory pool block [bytes]
// <i>Default: 2 (64K bytes maximum block size)
#define QF_MPOOL_SIZ_SIZE 2U

// <c2>Enable event parameter initialization (QEVT_PAR_INIT)
// <i>Initialize parameters of dynamic events at allocation
// <i>(Resource Acquisition Is Initialization (RAII) for dynamic events)
//#define QEVT_PAR_INIT
// </c>

// <c1>Enable active object stop API (QACTIVE_CAN_STOP)
// <i>Enable active object stop API (Not recommended)
//#define QACTIVE_CAN_STOP
// </c>

// <c1>Enable context switch callback *without* QS (QF_ON_CONTEXT_SW)
// <i>Context switch callback QF_onContextSw() when Q_SPY is undefined.
//#ifndef Q_SPY
//#define QF_ON_CONTEXT_SW
//#endif
// </c>

// <c1>Enable context switch callback *with* QS (QF_ON_CONTEXT_SW)
// <i>Context switch callback QF_onContextSw() when Q_SPY is defined.
//#ifdef Q_SPY
//#define QF_ON_CONTEXT_SW
//#endif
// </c>

// <c2>Enable memory isolation (QF_MEM_ISOLATE)
// <i>Memory isolation (supported in SafeQP only, requires MPU)
// <i>NOTE: implies QF_ON_CONTEXT_SW.
//#define QF_MEM_ISOLATE
// </c>

// </h>

//..........................................................................
// <h>QV/QK/QXK built-in kernels (ARM Cortex-M)

// <c2>Kernel uses critical section based on BASEPRI (QF_USE_BASEPRI)
// <i>If not selected, critical section will be based on PRIMASK
// <i>NOTE: The BASEPRI threshold can be adjusted in the "Text Editor" mode.
#define QF_USE_BASEPRI 0x3F
// </c>

// <c2>QK Kernel uses IRQ for return-from-preemption
// <i>NOTE: Use "editor mode" to edit QK_USE_IRQ_NUM
// <i>NOTE: Use "editor mode" to edit QK_USE_IRQ_HANDLER
//#define QK_USE_IRQ_NUM     NNN
//#define QK_USE_IRQ_HANDLER XXX_IRQHandler
// </c>

// <c2>QXK Kernel uses IRQ for return-from-preemption
// <i>NOTE: Use "editor mode" to edit QXK_USE_IRQ_NUM
// <i>NOTE: Use "editor mode" to edit QXK_USE_IRQ_HANDLER
//#define QXK_USE_IRQ_NUM     NNN
//#define QXK_USE_IRQ_HANDLER XXX_IRQHandler
// </c>

// </h>

//..........................................................................
// <h>QS Software Tracing (Q_SPY)
// <i>Target-resident component of QP/Spy software tracing system
// <i>(tracing instrumentation and command-input).
// <i>NOTE: The QS software tracing instrumentation is activated only when
// <i>the macro Q_SPY is defined on the command-line to the compiler.
// <i>Typically, Q_SPY is defined only in the "spy" build configuration.

// <o>QS timestamp size (QS_TIME_SIZE)
//   <1U=>1
//   <2U=>2
//   <4U=>4 (default)
// <i>Size of the timestamp in QS [bytes]
// <i>Default: 4 (2^32 dynamic range)
#define QS_TIME_SIZE 4U

// <o>QS buffer counter size (QS_CTR_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of the counter in the internal QS buffer [bytes]
// <i>Default: 2 (64K bytes in QS buffer)
#define QS_CTR_SIZE 2U

// </h>

//------------- <<< end of configuration section >>> -----------------------

#endif // QP_CONFIG_H_
