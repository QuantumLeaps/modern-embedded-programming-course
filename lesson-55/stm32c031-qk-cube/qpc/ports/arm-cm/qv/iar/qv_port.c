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
#define QP_IMPL 1U
#include "qp_port.h"
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem

#define SCB_SYSPRI   ((uint32_t volatile *)0xE000ED18U)
#define NVIC_IP      ((uint32_t volatile *)0xE000E400U)
#define SCB_CPACR   *((uint32_t volatile *)0xE000ED88U)
#define FPU_FPCCR   *((uint32_t volatile *)0xE000EF34U)

// helper macros to "stringify" values
#define VAL(x) #x
#define STRINGIFY(x) VAL(x)

//============================================================================
// interrupts and critical section...
//
// NOTE:
// The following interrupt disabling/enabling as well as critical section
// entry/exit functions are defined as "weak" so that they can be
// re-implemented differently at the application level.
//
// NOTE:
// For best performance, these functions are implemented in assembly,
// but they can be implemented in C as well.

#pragma weak QF_int_disable_
#pragma weak QF_int_enable_
#pragma weak QF_crit_entry_
#pragma weak QF_crit_exit_

//int32_t volatile QF_int_lock_nest_; // not used
extern char const QF_port_module_[];
char const QF_port_module_[] = "qv_port";

//............................................................................
// Unconditionally disable interrupts.
// NOTE: this function must NOT use the stack.
//
// Description:
// When QF_USE_BASEPRI is defined, interrupts are disabled with BASEPRI.
// Otherwise interrupts are disabled with the PRIMASK register.
// Additionally, the function also asserts that the interrupts are
// NOT disabled upon the entry to the function, which means that
// this interrupt management policy CANNOT nest.
__stackless
void QF_int_disable_(void) {
__asm volatile (
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for interrupt disabling?
    "  MRS     r0,BASEPRI       \n" // r0 <- previous BASEPRI
    "  MOVS    r1,#" STRINGIFY(QF_USE_BASEPRI) "\n"
    "  MSR     BASEPRI,r1       \n" // NOTE: Cortes-M7 erratum 837070 is OK
#else                   //--------- use PRIMASK for disabling interrupts
    "  MRS     r0,PRIMASK       \n" // r0 <- previous PRIMASK
    "  CPSID   i                \n" // set PRIMASK
#endif                  //--------- use PRIMASK for disabling interrupts
    "  CMP     r0,#0            \n" // assert(PRIMASK/BASEPRI == 0)
    "  BNE     QF_int_disable_error\n"
    "  BX      lr               \n"
    "QF_int_disable_error:      \n"
    "  LDR     r0,=QF_port_module_ \n"
    "  MOVS    r1,#100          \n"
    "  LDR     r2,=Q_onError    \n"
    "  BX      r2               \n"
    );
}
//............................................................................
// Unconditionally enable interrupts.
// NOTE: this function must NOT use the stack.
//
// Description:
// When QF_USE_BASEPRI is defined, interrupts are enabled with BASEPRI.
// Otherwise interrupts are enabled with the PRIMASK register.
// Additionally, the function also asserts that the interrupts ARE
// disabled upon the entry to the function, which means that
// this interrupt management policy CANNOT nest.
__stackless
void QF_int_enable_(void) {
__asm volatile (
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for enabling interrupts?
    "  MRS     r0,BASEPRI       \n" // r0 <- previous BASEPRI
#else                   //--------- use PRIMASK for enabling interrupts
    "  MRS     r0,PRIMASK       \n" // r0 <- previous PRIMASK
#endif                  //--------- use PRIMASK for enabling interrupts

    "  CMP     r0,#0            \n" // assert(PRIMASK/BASEPRI != 0)
    "  BEQ     QF_int_enable_error\n"
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for enabling interrupts?
    "  MOVS    r1,#0            \n"
    "  MSR     BASEPRI,r1       \n" // NOTE: Cortes-M7 erratum 837070 is OK
#else                   //--------- use PRIMASK for enabling interrupts
    "  CPSIE   i                \n" // clear PRIMASK
#endif                  //--------- use PRIMASK for enabling interrupts
    "  BX      lr               \n"
    "QF_int_enable_error:       \n"
    "  LDR     r0,=QF_port_module_ \n"
    "  MOVS    r1,#101          \n"
    "  LDR     r2,=Q_onError    \n"
    "  BX      r2               \n"
    );
}
//............................................................................
// Enter QF critical section.
// NOTE: this function must NOT use the stack.
//
// Description:
// When QF_USE_BASEPRI is defined, critical section uses BASEPRI.
// Otherwise, critical section uses the PRIMASK register.
// Additionally, the function also asserts that the interrupts are
// NOT disabled upon the entry to the function, which means that
// this critical section CANNOT nest.
__stackless
void QF_crit_entry_(void) {
__asm volatile (
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for critical section?
    "  MRS     r0,BASEPRI       \n" // r0 <- previous BASEPRI
    "  MOVS    r1,#" STRINGIFY(QF_USE_BASEPRI) "\n"
    "  MSR     BASEPRI,r1       \n" // NOTE: Cortes-M7 erratum 837070 is OK
#else                   //--------- use PRIMASK for critical section
    "  MRS     r0,PRIMASK       \n" // r0 <- previous PRIMASK
    "  CPSID   i                \n" // set PRIMASK
#endif                  //--------- use PRIMASK for critical section
    "  CMP     r0,#0            \n" // assert(PRIMASK/BASEPRI == 0)
    "  BNE     QF_crit_entry_error\n"
    "  BX      lr               \n"
    "QF_crit_entry_error:       \n"
    "  LDR     r0,=QF_port_module_ \n"
    "  MOVS    r1,#110          \n"
    "  LDR     r2,=Q_onError    \n"
    "  BX      r2               \n"
    );
}
//............................................................................
// Exit QF critical section.
// NOTE: this function must NOT use the stack.
//
// description:
// When QF_USE_BASEPRI is defined, critical section uses BASEPRI.
// Otherwise, critical section uses the PRIMASK register.
// Additionally, the function also asserts that the interrupts ARE
// disabled upon the entry to the function, which means that
// this critical section CANNOT nest.
__stackless
void QF_crit_exit_(void) {
__asm volatile (
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for critical section?
    "  MRS     r0,BASEPRI       \n" // r0 <- previous BASEPRI
#else                   //--------- use PRIMASK for critical section
    "  MRS     r0,PRIMASK       \n" // r0 <- previous PRIMASK
#endif                  //--------- use PRIMASK for critical section
    "  CMP     r0,#0            \n" // assert(PRIMASK/BASEPRI != 0)
    "  BEQ     QF_crit_exit_error\n"
#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for critical section?
    "  MOVS    r1,#0            \n"
    "  MSR     BASEPRI,r1       \n" // NOTE: Cortes-M7 erratum 837070 is OK
#else                   //--------- use PRIMASK
    "  CPSIE   i                \n" // clear PRIMASK
#endif                  //--------- use PRIMASK
    "  BX      lr               \n"
    "QF_crit_exit_error:        \n"
    "  LDR     r0,=QF_port_module_ \n"
    "  MOVS    r1,#111          \n"
    "  LDR     r2,=Q_onError    \n"
    "  BX      r2               \n"
    );
}

//============================================================================
// Initialize the exception priorities and IRQ priorities to safe values.
//
// description:
// On ARMv7-M or higher, this QF port disables interrupts by means of the
// BASEPRI register. However, this method cannot disable interrupt
// priority zero, which is the default for all interrupts out of reset.
// The following code changes the SysTick priority and all IRQ priorities
// to the safe value QF_USE_BASEPRI, which the QF critical section can disable.
// This avoids breaching of the QF critical sections in case the
// application programmer forgets to explicitly set priorities of all
// "kernel aware" interrupts.
//
// NOTE: The IRQ priorities established in QV_init() can be later changed
// by the application-level code.
void QV_init(void) {

#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for critical section?

    // SCB_SYSPRI[2]:  SysTick
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (QF_USE_BASEPRI << 24U));

    // set all 240 possible IRQ priories to QF_USE_BASEPRI...
    for (uint_fast8_t n = 0U; n < (240U/sizeof(uint32_t)); ++n) {
        NVIC_IP[n] = (QF_USE_BASEPRI << 24U) | (QF_USE_BASEPRI << 16U)
                     | (QF_USE_BASEPRI << 8U) | QF_USE_BASEPRI;
    }

#endif                  //--------- use BASEPRI for critical section

#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the FPU is enabled by setting CP10 & CP11 Full Access
    SCB_CPACR = (SCB_CPACR | ((3UL << 20U) | (3UL << 22U)));

    // FPU automatic state preservation (ASPEN) lazy stacking (LSPEN)
    FPU_FPCCR = (FPU_FPCCR | (1U << 30U) | (1U << 31U));
#endif                  //--------- VFP available
}

//============================================================================
#if (__ARM_ARCH == 6) // if ARMv6-M...

// hand-optimized quick LOG2 in assembly (no CLZ instruction in ARMv6-M)
__stackless
uint_fast8_t QF_qlog2(uint32_t x) {
    static uint8_t const log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    uint_fast8_t n;

__asm volatile (
    "  MOVS    %[n],#0           \n"
#if (QF_MAX_ACTIVE > 16U)
    "  LSRS    r2,r0,#16        \n"
    "  BEQ     QF_qlog2_1       \n"
    "  MOVS    %[n],#16         \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_1:                \n"
#endif
#if (QF_MAX_ACTIVE > 8U)
    "  LSRS    r2,r0,#8         \n"
    "  BEQ     QF_qlog2_2       \n"
    "  ADDS    %[n],%[n],#8     \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_2:                \n"
#endif
    "  LSRS    r2,r0,#4         \n"
    "  BEQ.N   QF_qlog2_3       \n"
    "  ADDS    %[n],%[n],#4     \n"
    "  MOVS    r0,r2            \n"
    "QF_qlog2_3:" : [n]"=r"(n)
    );
    return n + log2LUT[x];
}

#endif // ARMv6-M

