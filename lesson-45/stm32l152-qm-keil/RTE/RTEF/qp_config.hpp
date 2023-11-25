//============================================================================
// QP configuration file (QV on ARM Cortex-M)
// Last updated for version: 7.3.0
// Last updated on: 2023-10-30
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef QP_CONFIG_HPP_
#define QP_CONFIG_HPP_

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

// <n>NOTE: Requires command-line macro: QP_CONFIG
// <i>This qp_config.h header file is activated only when the macro
// <i>QP_CONFIG is defined on the command-line to the compiler
// <n>-------------------------------------------

// <o>QP API compatibility version (QP_API_VERSION)
//   <0=>  0   (Maximum compatibility)
//   <580=>580 (QP 5.8.0 or newer)
//   <660=>660 (QP 6.6.0 or newer)
//   <691=>691 (QP 6.9.1 or newer)
//   <700=>700 (QP 7.0.0 or newer)
//   <9999=>9999 (Latest only)
// <i>QP API backwards compatibility with the QP/C API version.
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

// <c3>Disable QP FuSa in development
// <i>Disable assertions and other self monitoring features
// <i>in development build configurations (NDEBUG undefined).
// <i>VIOLATES functional safety standards. NOT recommended !!!
//#ifndef NDEBUG
//#define Q_UNSAFE
//#endif
// </c>

// <c3>Disable QP FuSa in production release
// <i>Disable assertions and other self monitoring features
// <i>in the release build configurations (NDEBUG defined).
// <i>VIOLATES functional safety standards. NOT recommended !!!
//#ifdef NDEBUG
//#define Q_UNSAFE
//#endif
// </c>

// </h>

//..........................................................................
// <h>QEP Event Processor
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
// <h>QF Framework
// <i>Active Object framework

// <o>Maximum # Active Objects (QF_MAX_ACTIVE) <1-64>
// <i>Maximum # Active Objects in the system <1..64>
// <i>Default: 32
#define QF_MAX_ACTIVE  32U

// <o>Maximum # event pools (QF_MAX_EPOOL)
// <0=>0 no event pools
// <1=>1 <2=>2 <3=>3 (deafult) <4=>4 <5=>5
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

// <c1>Dynamic Event Constructor (QEVT_DYN_CTOR)
// <i>Dynamic Event Constructor (RAII)
//#define QEVT_DYN_CTOR
// </c>

// <c1>Active Object stop API (QACTIVE_CAN_STOP)
// <i>Enable Active Object stop API (Not recommended)
//#define QACTIVE_CAN_STOP
// </c>

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
//   <4U=>4
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

// </h>

//..........................................................................
// <h>QS Software Tracing
// <i>Target-resident component of QP/Spy software tracing system
// <i>(tracing instrumenation and command-input).

// <n>NOTE: Requires command-line macro: Q_SPY
// <i>The QS software tracing instrumenation is activated only when
// <i>the macro Q_SPY is defined on the command-line to the compiler.
// <i>Typically, Q_SPY is defined only in the "spy" build configuration.

// <o>QS timestamp size (QS_TIME_SIZE)
//   <1U=>1
//   <2U=>2
//   <4U=>4 (default)
// <i>Size of the timestamp in QS [bytes]
// <i>Default: 4 (2^32 dynamic range)
#define QS_TIME_SIZE 4U

// <o>Object pointer size (QS_OBJ_PTR_SIZE)
//   <2U=>2
//   <4U=>4 (default)
//   <8U=>8
// <i>Size of object pointer for QS [bytes]
// <i>Default: 4 (4G address space)
#define QS_OBJ_PTR_SIZE 4U

// <o>Function pointer size (QS_FUN_PTR_SIZE)
//   <2U=>2
//   <4U=>4 (default)
//   <8U=>8
// <i>Size of function pointer for QS [bytes]
// <i>Default: 4 (4G address space)
#define QS_FUN_PTR_SIZE 4U

// <o>QS buffer counter size (QS_CTR_SIZE)
//   <1U=>1
//   <2U=>2 (default)
//   <4U=>4
// <i>Size of the counter in the internal QS buffer [bytes]
// <i>Default: 2 (64K bytes in QS buffer)
#define QS_CTR_SIZE 2U

// </h>

//..........................................................................
// <h>QV Cooperative Kernel
// <i>Cooperative run-to-completion kernel.

// <h>Context switch callback (QF_ON_CONTEXT_SW)

// <c2>Context switch callback WITHOUT QS
// <i>Enable context switch callback QF_onContextSw()
// <i>When Q_SPY is undefined.
//#ifndef Q_SPY
//#define QF_ON_CONTEXT_SW
//#endif
// </c>

// <c2>Context switch callback WITH QS
// <i>Enable context switch callback QF_onContextSw()
// <i>When Q_SPY is defined.
//#ifdef Q_SPY
//#define QF_ON_CONTEXT_SW
//#endif
// </c>

// </h>

// <c2>MPU memory isolation (QF_MEM_ISOLATE)
// <i>Enable memory isolation (requires MPU)
// <i>NOTE: implies QF_ON_CONTEXT_SW.
//#define QF_MEM_ISOLATE
// </c>

// </h>

//------------- <<< end of configuration section >>> -----------------------

#endif // QP_CONFIG_HPP_
