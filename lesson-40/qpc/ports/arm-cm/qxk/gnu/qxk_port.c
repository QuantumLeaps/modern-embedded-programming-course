//============================================================================
// QP/C Real-Time Embedded Framework (RTEF)
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
//! @date Last updated on: 2023-08-16
//! @version Last updated for: @ref qpc_7_3_0
//!
//! @file
//! @brief QXK/C port to ARM Cortex-M, GNU-ARM

#define QP_IMPL 1U
#include "qp_port.h"
#include "qp_pkg.h"
#include "qsafe.h"        // QP Functional Safety (FuSa) System

#include <stddef.h>       // for offsetof()

//============================================================================
// NOTE: keep in synch with struct QXK_Attr in "qxk.h" !!!
#define QXK_CURR       0
#define QXK_NEXT       4
#define QXK_ACT_PRIO   12

// make sure that the offsets match the QXK declaration in "qxk.h"
_Static_assert(QXK_CURR == offsetof(QXK_Attr, curr),
               "QXK_Attr.curr at unexpected offset");
_Static_assert(QXK_NEXT == offsetof(QXK_Attr, next),
               "QXK_Attr.next at unexpected offset");
_Static_assert(QXK_ACT_PRIO == offsetof(QXK_Attr, actPrio),
               "QXK_Attr.actPrio at unexpected offset");

// offsets within struct QActive; NOTE: keep in synch with "qp.h" !!!
#define QACTIVE_OSOBJ  28
#define QACTIVE_PRIO   36

// make sure that the offsets match the QActvie declaration in "qp.h"
_Static_assert(QACTIVE_OSOBJ == offsetof(QActive, osObject),
               "QActive.osObject at unexpected offset");
_Static_assert(QACTIVE_PRIO  == offsetof(QActive, prio),
               "QActive.prio at unexpected offset");

// helper macros to "stringify" values
#define VAL(x) #x
#define STRINGIFY(x) VAL(x)

//============================================================================
// Initialize the private stack of an extended QXK thread.
//
// NOTE
// The function aligns the stack to the 8-byte boundary for compatibility
// with the AAPCS. Additionally, the function pre-fills the stack with the
// known bit pattern (0xDEADBEEF).
//
// NOTE: QXK_stackInit_() must be called before the QXK kernel is made aware
// of this thread. In that case the kernel cannot use the thread yet, so no
// critical section is needed.
void QXThread_stackInit_(QActive *me, QXThreadHandler const handler,
                    void * const stkSto, uint_fast16_t const stkSize)
{
    // round down the stack top to the 8-byte boundary
    // NOTE: ARM Cortex-M stack grows down from hi -> low memory
    uint32_t *sp =
        (uint32_t *)((((uint32_t)stkSto + stkSize) >> 3U) << 3U);
    uint32_t *sp_limit;

    // synthesize the ARM Cortex-M exception stack frame...
    *(--sp) = (1U << 24);    // xPSR  (just the THUMB bit)
    *(--sp) = (uint32_t)handler;          // PC (the thread handler)
    *(--sp) = (uint32_t)&QXK_threadExit_; // LR (exit from thread)
    *(--sp) = 0x0000000CU;   // R12
    *(--sp) = 0x00000003U;   // R3
    *(--sp) = 0x00000002U;   // R2
    *(--sp) = 0x00000001U;   // R1
    *(--sp) = (uint32_t)me;  // R0 parameter to handler (thread object)
    *(--sp) = 0x0000000BU;   // R11
    *(--sp) = 0x0000000AU;   // R10
    *(--sp) = 0x00000009U;   // R9
    *(--sp) = 0x00000008U;   // R8
    *(--sp) = 0x00000007U;   // R7
    *(--sp) = 0x00000006U;   // R6
    *(--sp) = 0x00000005U;   // R5
    *(--sp) = 0x00000004U;   // R4

#ifdef __ARM_FP         //--------- if VFP available...
    *(--sp) = 0xFFFFFFFDU;   // exception return with VFP state
    *(--sp) = 0xAAAAAAAAU;   // stack "aligner"
#endif                       // VFP available

    // save the top of the stack in the thread's attribute
    me->osObject = sp;

    // pre-fill the unused part of the stack with 0xDEADBEEF
    sp_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) >> 3U) + 1U) << 3U);
    for (; sp >= sp_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }
}

// prototypes ----------------------------------------------------------------
void PendSV_Handler(void);
#ifdef QXK_USE_IRQ_HANDLER          // if use IRQ...
void QXK_USE_IRQ_HANDLER(void);
#else                               // use default (NMI)
void NMI_Handler(void);
#endif

#define SCB_SYSPRI   ((uint32_t volatile *)0xE000ED18U)
#define NVIC_EN      ((uint32_t volatile *)0xE000E100U)
#define NVIC_IP      ((uint32_t volatile *)0xE000E400U)
#define SCB_CPACR   *((uint32_t volatile *)0xE000ED88U)
#define FPU_FPCCR   *((uint32_t volatile *)0xE000EF34U)
#define NVIC_PEND    0xE000E200
#define SCB_ICSR     0xE000ED04

//............................................................................
// Initialize the exception priorities and IRQ priorities to safe values.
//
// Description:
// On ARMv7-M or higher, this QXK port disables interrupts by means of the
// BASEPRI register. However, this method cannot disable interrupt
// priority zero, which is the default for all interrupts out of reset.
// The following code changes the SysTick priority and all IRQ priorities
// to the safe value QF_BASEPRI, which the QF critical section can disable.
// This avoids breaching of the QF critical sections in case the
// application programmer forgets to explicitly set priorities of all
// "kernel aware" interrupts.
//
// The interrupt priorities established in QXK_init() can be later
// changed by the application-level code.
void QXK_init(void) {

#if (__ARM_ARCH != 6)   //--------- if ARMv7-M and higher...

    // SCB_SYSPRI[2]:  SysTick
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (QF_BASEPRI << 24U));

    // set all 240 possible IRQ priories to QF_BASEPRI...
    for (uint_fast8_t n = 0U; n < (240U/sizeof(uint32_t)); ++n) {
        NVIC_IP[n] = (QF_BASEPRI << 24U) | (QF_BASEPRI << 16U)
                     | (QF_BASEPRI << 8U) | QF_BASEPRI;
    }

#endif                  //--------- ARMv7-M or higher

    // SCB_SYSPRI[2]: PendSV set to priority 0xFF (lowest)
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (0xFFU << 16U));

#ifdef QXK_USE_IRQ_NUM   //--------- QXK IRQ specified?
    // The QXK port is configured to use a given ARM Cortex-M IRQ #
    // to return to thread mode (default is to use the NMI exception)
    NVIC_IP[QXK_USE_IRQ_NUM] = 0U; // priority 0 (highest)
    NVIC_EN[QXK_USE_IRQ_NUM >> 5U] = (1U << (QXK_USE_IRQ_NUM & 0x1FU));
#endif                  //--------- QXK IRQ specified

#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the FPU is enabled by seting CP10 & CP11 Full Access
    SCB_CPACR = (SCB_CPACR | ((3UL << 20U) | (3UL << 22U)));

    // FPU automatic state preservation (ASPEN) lazy stacking (LSPEN)
    FPU_FPCCR = (FPU_FPCCR | (1U << 30U) | (1U << 31U));
#endif                  //--------- VFP available
}

//============================================================================
// The PendSV_Handler exception handler is used for performing asynchronous
// preemption in QXK. The use of the PendSV exception is the recommended and
// most efficient method for performing context switches in ARM Cortex-M.
//
// The PendSV exception should have the lowest interrupt priority in the system
// (0xFF, see QXK_init()). All other exceptions and interrupts should have
// higher interrupt priority.
//
// Also, *all* "kernel aware" ISRs in the QXK application must call the
// QXK_ISR_EXIT() macro, which triggers PendSV when it detects a need for
// asynchronous preemption.
//
// Due to tail-chaining and its lowest priority, the PendSV exception will be
// entered immediately after the exit from the *last* nested interrupt (or
// exception). In QXK, this is exactly the time when the QXK activator needs to
// handle the asynchronous preemption.
//
// NOTE:
// The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
// but for Cortex-M0/M0+/M1 the mnemonics MOV, LSR and ADD always set the
// condition flags in the PSR.
__attribute__ ((naked, optimize("-fno-stack-protector")))
void PendSV_Handler(void) {
__asm volatile (

    //<<<<<<<<<<<<<<<<<<<<<<< CRITICAL SECTION BEGIN <<<<<<<<<<<<<<<<<<<<<<<<
#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  CPSID   i                \n" // disable interrupts (set PRIMASK)
#else                               // ARMv7-M or higher
    "  MOV     r0,#" STRINGIFY(QF_BASEPRI) "\n"
    "  CPSID   i                \n" // selectively disable interrupts with BASEPRI
    "  MSR     BASEPRI,r0       \n" // apply the workaround the Cortex-M7 erratum
    "  CPSIE   i                \n" // 837070, see SDEN-1068427.
#endif                              // ARMv7-M or higher

#ifdef QF_MEM_ISOLATE
    "  PUSH    {r0,lr}          \n" // save the "aligner" and the EXC_RETURN value
    "  LDR     r0,=QF_onMemSys  \n"
    "  BLX     r0               \n" // call QF_onMemSys()
    "  POP     {r0,r1}          \n" // restore the aligner + lr into r1
    "  MOV     lr,r1            \n" // restore EXC_RETURN into lr
#endif

    // The PendSV exception handler can be preempted by an interrupt,
    // which might pend PendSV exception again. The following write to
    // ICSR[27] un-pends any such spurious instance of PendSV.
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#27        \n" // r0 := (1 << 27) (UNPENDSVSET bit)
    "  LDR     r2,=" STRINGIFY(SCB_ICSR) "\n" // Interrupt Control and State
    "  STR     r1,[r2]          \n" // ICSR[27] := 1 (unpend PendSV)

    // Check QXK_priv_.next, which contains the pointer to the next thread
    // to run, which is set in QXK_ISR_EXIT(). This pointer must not be NULL.
    "  LDR     r3,=QXK_priv_    \n"
    "  LDR     r0,[r3,#" STRINGIFY(QXK_NEXT) "]\n" // r1 := QXK_priv_.next
    "  CMP     r0,#0            \n" // is (QXK_priv_.next == 0)?
    "  BEQ     PendSV_return    \n" // branch if (QXK_priv_.next == 0)

    // Load pointers into registers...
    "  MOV     r12,r0           \n" // save QXK_priv_.next in r12
    "  LDR     r2,[r0,#" STRINGIFY(QACTIVE_OSOBJ) "]\n" // r2 := QXK_priv_.next->osObject
    "  LDR     r1,[r3,#" STRINGIFY(QXK_CURR) "]\n" // r1 := QXK_priv_.curr

    "  CMP     r1,#0            \n" // (QXK_priv_.curr != 0)?
    "  BNE     PendSV_save_ex   \n" // branch if (current thread is extended)

    "  CMP     r2,#0            \n" // (QXK_priv_.next->osObject != 0)?
    "  BNE     PendSV_save_ao   \n" // branch if (next tread is extended)

    "PendSV_activate:           \n"
#ifdef __ARM_FP         //--------- if VFP available...
    "  TST     lr,#(1 << 4)     \n" // is it return with the VFP exception frame?
    "  IT      EQ               \n" // if lr[4] is zero...
    "  VSTMDBEQ sp!,{s16-s31}   \n" // ... save VFP registers s16..s31
    "  PUSH    {r0,lr}          \n" // save the "aligner" and the EXC_RETURN value
#endif                              // VFP available
    // The QXK activator must be called in a thread context, while this code
    // executes in the handler context of the PendSV exception. The switch
    // to the Thread mode is accomplished by returning from PendSV using
    // a fabricated exception stack frame, where the return address is
    // QXK_activate_().
    //
    // NOTE: the QXK activator is called with interrupts DISABLED and also
    // it returns with interrupts DISABLED.
    "  MOV     r3,#1            \n"
    "  LSL     r3,r3,#24        \n" // r3 := (1 << 24), set T bit  (new xpsr)
    "  LDR     r2,=QXK_activate_\n" // address of QXK_activate_
    "  SUB     r2,r2,#1         \n" // align Thumb-address at half-word (new pc)
    "  LDR     r1,=QXK_thread_ret\n" // return address after the call   (new lr)

    "  SUB     sp,sp,#(8*4)     \n" // reserve space for exception stack frame
    "  ADD     r0,sp,#(5*4)     \n" // r0 := 5 registers below the top of stack
    "  STM     r0!,{r1-r3}      \n" // save xpsr,pc,lr

    "  MOV     r0,#6            \n"
    "  MVN     r0,r0            \n" // r0 := ~6 == 0xFFFFFFF9
#if (__ARM_ARCH != 6)               // ARMv7-M or higher
    "  DSB                      \n" // ARM Erratum 838869
#endif                              // ARMv7-M or higher
    "  BX      r0               \n" // exception-return to the QXK activator

    //========================================================================
    // Saving AO-thread before crossing to eXtended-thread
    // expected register contents:
    // r0  -> QXK_priv_.next / basic-thread
    // r1  -> QXK_priv_.curr / basic-thread
    // r2  -> QXK_priv_.next->osObject (SP)
    // r3  -> &QXK_priv_
    // r12 -> QXK_priv_.next / basic-thread
    "PendSV_save_ao:            \n"
#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  SUB     sp,sp,#(8*4)     \n" // make room for 8 registers r4-r11
    "  MOV     r0,sp            \n" // r0 := temporary stack pointer
    "  STMIA   r0!,{r4-r7}      \n" // save the low registers
    "  MOV     r4,r8            \n" // move the high registers to low registers...
    "  MOV     r5,r9            \n"
    "  MOV     r6,r10           \n"
    "  MOV     r7,r11           \n"
    "  STMIA   r0!,{r4-r7}      \n" // save the high registers
    "  MOV     r0,r12           \n" // restore QXK_priv_.next in r0
#else                               // ARMv7-M or higher
    "  PUSH    {r4-r11}         \n" // save r4-r11 on top of the exception frame
#ifdef __ARM_FP         //--------- if VFP available...
    "  TST     lr,#(1 << 4)     \n" // is it return with the VFP exception frame?
    "  IT      EQ               \n" // if lr[4] is zero...
    "  VSTMDBEQ sp!,{s16-s31}   \n" // ... save VFP registers s16..s31
    "  PUSH    {r0,lr}          \n" // save the "aligner" and the EXC_RETURN value
#endif                              // VFP available
#endif                              // ARMv7-M or higher

    "  CMP     r2,#0            \n"
    "  BNE     PendSV_restore_ex\n" // branch if (QXK_priv_.next->osObject != 0)
    // otherwise continue to restoring next AO-thread...

    //------------------------------------------------------------------------
    // Restoring AO-thread after crossing from eXtended-thread
    // expected register contents:
    // r1  -> QXK_priv_.curr / basic-thread
    // r2  -> QXK_priv_.next->osObject (SP)
    // r3  -> &QXK_priv_
    // r12 -> QXK_priv_.next / basic-thread
    "PendSV_restore_ao:         \n"
    // don NOT clear QXK_priv_.curr or QXK_priv_.next,
    // as they might be needed for AO activation

#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  MOV     r0,sp            \n" // r0 := top of stack
    "  MOV     r2,r0            \n"
    "  ADD     r2,r2,#(4*4)     \n" // point r2 to the 4 high registers r7-r11
    "  LDMIA   r2!,{r4-r7}      \n" // pop the 4 high registers into low registers
    "  MOV     r8,r4            \n" // move low registers into high registers
    "  MOV     r9,r5            \n"
    "  MOV     r10,r6           \n"
    "  MOV     r11,r7           \n"
    "  LDMIA   r0!,{r4-r7}      \n" // pop the low registers
    "  ADD     sp,sp,#(8*4)     \n" // remove 8 registers from the stack

    "  MOV     r2,#6            \n"
    "  MVN     r2,r2            \n" // r2 := ~6 == 0xFFFFFFF9
    "  MOV     lr,r2            \n" // make sure MSP is used
#else                               // ARMv7-M or higher
#ifdef __ARM_FP         //--------- if VFP available...
    "  POP     {r0,lr}          \n" // restore alighner and EXC_RETURN into lr
    "  DSB                      \n" // ARM Erratum 838869
    "  TST     lr,#(1 << 4)     \n" // is it return to the VFP exception frame?
    "  IT      EQ               \n" // if EXC_RETURN[4] is zero...
    "  VLDMIAEQ sp!,{s16-s31}   \n" // ... restore VFP registers s16..s31
#else
    "  BIC     lr,lr,#(1 << 2)  \n" // make sure MSP is used
#endif                              // VFP available
    "  POP     {r4-r11}         \n" // restore r4-r11 from the next thread's stack
#endif                              // ARMv7-M or higher

    "  MOV     r0,r12           \n" // r0 := QXK_priv_.next
    "  MOV     r2,#" STRINGIFY(QACTIVE_PRIO) "\n" // r2 := offset of .prio
    "  LDRB    r0,[r0,r2]       \n" // r0 := QXK_priv_.next->prio
    "  LDR     r2,[r3,#" STRINGIFY(QXK_ACT_PRIO) "]\n" // r2 := QXK_priv_.actPrio
    "  CMP     r2,r0            \n"
    "  BCC     PendSV_activate  \n" // if (next->prio > actPrio) activate the next AO

    // otherwise no activation needed...
    "  MOV     r0,#0            \n"
    "  STR     r0,[r3,#" STRINGIFY(QXK_CURR) "]\n" // QXK_priv_.curr := 0
    "  STR     r0,[r3,#" STRINGIFY(QXK_NEXT) "]\n" // QXK_priv_.next := 0

#if defined(Q_SPY) || defined(QF_ON_CONTEXT_SW)
    "  MOV     r0,#0            \n" // r0 := 0 (next is basic)
    "  PUSH    {r0,lr}          \n" // save the aligner + EXC_RETURN
    "  LDR     r3,=QXK_contextSw_ \n"
    "  BLX     r3               \n" // call QXK_contextSw_()
#ifdef QF_MEM_ISOLATE
    "  LDR     r3,=QF_onMemApp  \n"
    "  BLX     r3               \n" // call QF_onMemApp()
#endif
    "  POP     {r0,r1}          \n" // restore the aligner + EXC_RETURN
    "  MOV     lr,r1            \n" // restore EXC_RETURN into lr
#endif // defined(Q_SPY) || defined(QF_ON_CONTEXT_SW)

    // re-enable interrupts and return from PendSV
    "PendSV_return:             \n"
#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  CPSIE   i                \n" // enable interrupts (clear PRIMASK)
#else                               // ARMv7-M or higher
    "  MOV     r0,#0            \n"
    "  MSR     BASEPRI,r0       \n" // enable interrupts (clear BASEPRI)
    "  DSB                      \n" // ARM Erratum 838869
#endif                              // ARMv7-M or higher
    //>>>>>>>>>>>>>>>>>>>>>>>> CRITICAL SECTION END >>>>>>>>>>>>>>>>>>>>>>>>>
    "  BX      lr               \n" // return to the preempted AO-thread

    //------------------------------------------------------------------------
    // Saving extended-thread
    // expected register contents:
    // r0  -> QXK_priv_.next / basic-thread
    // r1  -> QXK_priv_.curr / basic-thread
    // r2  -> QXK_priv_.next->osObject (SP)
    // r3  -> &QXK_priv_
    // r12 -> QXK_priv_.next / basic-thread
    "PendSV_save_ex:            \n"
    "  MRS     r0,PSP           \n" // r0 := Process Stack Pointer
#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  SUB     r0,r0,#(8*4)     \n" // make room for 8 registers r4-r11
    "  MOV     r1,r0            \n" // r1 := temporary PSP (do not clobber r0!)
    "  STMIA   r1!,{r4-r7}      \n" // save the low registers
    "  MOV     r4,r8            \n" // move the high registers to low registers...
    "  MOV     r5,r9            \n"
    "  MOV     r6,r10           \n"
    "  MOV     r7,r11           \n"
    "  STMIA   r1!,{r4-r7}      \n" // save the high registers
    // NOTE: at this point r0 holds the top of stack

    "  LDR     r1,[r3,#" STRINGIFY(QXK_CURR) "]\n" // r1 := QXK_priv_.curr (restore value)
#else                               // ARMv7-M or higher
    "  ISB                      \n" // reset pipeline after fetching PSP
    "  STMDB   r0!,{r4-r11}     \n" // save r4-r11 on top of the exception frame
#ifdef __ARM_FP         //--------- if VFP available...
    "  TST     lr,#(1 << 4)     \n" // is it return with the VFP exception frame?
    "  IT      EQ               \n" // if lr[4] is zero...
    "  VSTMDBEQ r0!,{s16-s31}   \n" // ... save VFP registers s16..s31
    "  STMDB   r0!,{r1,lr}      \n" // save the "aligner" and the EXC_RETURN value
#endif                              // VFP available
#endif                              // ARMv7-M or higher

    // store the SP of the current extended-thread
    "  STR     r0,[r1,#" STRINGIFY(QACTIVE_OSOBJ) "]\n" // QXK_priv_.curr->osObject := r0
    "  MOV     r0,r12           \n" // QXK_priv_.next (restore value)

    "  CMP     r2,#0            \n"
    "  BEQ     PendSV_restore_ao\n" // branch if (QXK_priv_.next->osObject == 0)
    // otherwise continue to restoring next extended-thread...

    //------------------------------------------------------------------------
    // Restoring extended-thread
    // expected register contents:
    // r0  -> QXK_priv_.next / basic-thread
    // r1  -> QXK_priv_.curr / basic-thread
    // r2  -> QXK_priv_.next->osObject (SP)
    // r3  -> &QXK_priv_
    // r12 -> QXK_priv_.next / basic-thread
    "PendSV_restore_ex:         \n"
    "  STR     r0,[r3,#" STRINGIFY(QXK_CURR) "]\n" // QXK_priv_.curr := r0 (QXK_priv_.next)
    "  MOV     r0,#0            \n"
    "  STR     r0,[r3,#" STRINGIFY(QXK_NEXT) "]\n" // QXK_priv_.next := 0

#if defined(Q_SPY) || defined(QF_ON_CONTEXT_SW)
    "  MOV     r0,r12           \n" // r0 := next
    "  PUSH    {r0-r2,lr}    \n" // save next, osObject, EXC_RETURN
    "  LDR     r3,=QXK_contextSw_ \n"
    "  BLX     r3               \n" // call QXK_contextSw_()
#ifdef QF_MEM_ISOLATE
    "  LDR     r3,=QF_onMemApp  \n"
    "  BLX     r3               \n" // call QF_onMemApp()
#endif
    "  POP     {r0-r3}          \n" // restore next, osObject, EXC_RETURN
    "  MOV     lr,r3            \n" // restore the EXC_RETURN into lr
#endif // defined(Q_SPY) || defined(QF_ON_CONTEXT_SW)


    // exit the critical section
#if (__ARM_ARCH == 6)               // if ARMv6-M...
    "  CPSIE   i                \n" // enable interrupts (clear PRIMASK)

    "  MOV     r0,r2            \n" // r2 := top of stack
    "  ADD     r0,r0,#(4*4)     \n" // point r0 to the 4 high registers r7-r11
    "  LDMIA   r0!,{r4-r7}      \n" // pop the 4 high registers into low registers
    "  MOV     r8,r4            \n" // move low registers into high registers
    "  MOV     r9,r5            \n"
    "  MOV     r10,r6           \n"
    "  MOV     r11,r7           \n"
    "  LDMIA   r2!,{r4-r7}      \n" // pop the low registers
    "  MOV     r2,r0            \n" // r2 := holds the new top of stack

    "  MOV     r1,#2            \n"
    "  MVN     r1,r1            \n" // r1 := ~2 == 0xFFFFFFFD
    "  MOV     lr,r1            \n" // make sure PSP is used
#else                               // ARMv7-M or higher
    "  MOV     r1,#0            \n"
    "  MSR     BASEPRI,r1       \n" // enable interrupts (clear BASEPRI)
#ifdef __ARM_FP         //--------- if VFP available...
    "  LDMIA   r2!,{r1,lr}      \n" // restore aligner and EXC_RETURN into lr
    "  TST     lr,#(1 << 4)     \n" // is it return to the VFP exception frame?
    "  IT      EQ               \n" // if lr[4] is zero...
    "  VLDMIAEQ r2!,{s16-s31}   \n" // ... restore VFP registers s16..s31
#else
    "  ORR     lr,lr,#(1 << 2)  \n" // make sure PSP is used
#endif                              // VFP available
    "  LDMIA   r2!,{r4-r11}     \n" // restore r4-r11 from the next thread's stack
#endif                              // ARMv7-M or higher

    // set the PSP to the next thread's SP
    "  MSR     PSP,r2           \n" // Process Stack Pointer := r2

#if (__ARM_ARCH != 6)               // if ARMv7-M or higher...
    "  DSB                      \n" // ARM Erratum 838869
#endif                              // ARMv7-M or higher
    "  BX      lr               \n" // return to the next extended-thread
    );
}

//============================================================================
// QXK_thread_ret is a helper function executed when the QXK activator returns.
//
// NOTE: QXK_thread_ret does not execute in the PendSV context!
// NOTE: QXK_thread_ret is entered with interrupts DISABLED.
__attribute__ ((naked, used, optimize("-fno-stack-protector")))
void QXK_thread_ret(void) {
__asm volatile (

    // After the QXK activator returns, we need to resume the preempted
    // thread. However, this must be accomplished by a return-from-exception,
    // while we are still in the thread context. The switch to the exception
    // context is accomplished by triggering the NMI exception or the selected
    // IRQ (if macro #QXK_USE_IRQ_NUM is defined).

    // before triggering the NMI/IRQ, make sure that the VFP stack frame
    // will NOT be used...
#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the VFP stack frame will NOT be used
    "  MRS     r0,CONTROL       \n" // r0 := CONTROL
    "  BIC     r0,r0,#4         \n" // r0 := r0 & ~4 (FPCA bit)
    "  MSR     CONTROL,r0       \n" // CONTROL := r0 (clear CONTROL[2] FPCA bit)
    "  ISB                      \n" // ISB after MSR CONTROL (ARM AN321,Sect.4.16)
#endif                  //--------- VFP available

#ifdef QF_MEM_ISOLATE
    "  PUSH    {r0,lr}          \n" // save the aligner + EXC_RETURN
    "  LDR     r0,=QF_onMemApp  \n"
    "  BLX     r0               \n" // call QF_onMemApp()
    "  POP     {r0,r1}          \n" // restore the aligner + EXC_RETURN
    "  MOV     lr,r1            \n" // restore EXC_RETURN into lr
#endif

#ifndef QXK_USE_IRQ_NUM //--------- IRQ NOT defined, use NMI by default
    "  LDR     r0,=" STRINGIFY(SCB_ICSR) "\n" // Interrupt Control and State
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#31        \n" // r1 := (1 << 31) (NMI bit)
    "  STR     r1,[r0]          \n" // ICSR[31] := 1 (pend NMI)

#else                   //--------- use the selected IRQ
    "  LDR     r0,=" STRINGIFY(NVIC_PEND + ((QXK_USE_IRQ_NUM >> 5) << 2)) "\n"
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#" STRINGIFY(QXK_USE_IRQ_NUM & 0x1F) "\n" // r1 := IRQ bit
    "  STR     r1,[r0]          \n" // pend the IRQ

    // now enable interrupts so that pended IRQ can be entered
#if (__ARM_ARCH == 6)   //--------- if ARMv6-M...
    "  CPSIE   i                \n" // enable interrupts (clear PRIMASK)
#else                   //--------- ARMv7-M and higher
    "  MOV     r0,#0            \n"
    "  MSR     BASEPRI,r0       \n" // enable interrupts (clear BASEPRI)
#endif                  //--------- ARMv7-M and higher
#endif                  //--------- use IRQ

    // NOTE! interrupts are still disabled when NMI is used
    "  B       .                \n" // wait for preemption by NMI/IRQ
    );
}

//============================================================================
// This exception handler is used for returning back to the preempted thread.
// The exception handler simply removes its own interrupt stack frame from
// the stack (MSP) and returns to the preempted task using the interrupt
// stack frame that must be at the top of the stack.
__attribute__ ((naked, optimize("-fno-stack-protector")))
#ifndef QXK_USE_IRQ_NUM //--------- IRQ NOT defined, use NMI by default

// NOTE: The NMI_Handler() is entered with interrupts still disabled!
void NMI_Handler(void) {
__asm volatile (
    // enable interrupts
#if (__ARM_ARCH == 6)   //--------- if ARMv6-M...
    "  CPSIE   i                \n" // enable interrupts (clear PRIMASK)
#else                   //--------- ARMv7-M and higher
    "  MOV     r0,#0            \n"
    "  MSR     BASEPRI,r0       \n" // enable interrupts (clear BASEPRI)
#endif                  //--------- ARMv7-M and higher
);

#else                   //--------- use the selected IRQ

// NOTE: The IRQ Handler is entered with interrupts enabled
void QXK_USE_IRQ_HANDLER(void) {
#endif                  //--------- use IRQ
__asm volatile (
    "  ADD     sp,sp,#(8*4)     \n" // remove one 8-register exception frame

#ifdef __ARM_FP         //--------- if VFP available...
    "  POP     {r0,lr}          \n" // restore alighner and EXC_RETURN into lr
    "  DSB                      \n" // ARM Erratum 838869
    "  TST     lr,#(1 << 4)     \n" // is it return to the VFP exception frame?
    "  IT      EQ               \n" // if EXC_RETURN[4] is zero...
    "  VLDMIAEQ sp!,{s16-s31}   \n" // ... restore VFP registers s16..s31
#endif                  //--------- VFP available
    "  BX      lr               \n" // return to the preempted task
    );
}

//============================================================================
#if (__ARM_ARCH == 6) // if ARMv6-M...

// hand-optimized quick LOG2 in assembly (no CLZ instruction in ARMv6-M)
// NOTE:
// The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
// but for ARMv6-M the mnemonics MOV, LSR and ADD always set the condition
// flags in the PSR.
__attribute__ ((naked, optimize("-fno-stack-protector")))
uint_fast8_t QF_qlog2(uint32_t x) {
__asm volatile (
    "  MOV     r1,#0            \n"
#if (QF_MAX_ACTIVE > 16U)
    "  LSR     r2,r0,#16        \n"
    "  BEQ     QF_qlog2_1       \n"
    "  MOV     r1,#16           \n"
    "  MOV     r0,r2            \n"
    "QF_qlog2_1:                \n"
#endif
#if (QF_MAX_ACTIVE > 8U)
    "  LSR     r2,r0,#8         \n"
    "  BEQ     QF_qlog2_2       \n"
    "  ADD     r1, r1,#8        \n"
    "  MOV     r0, r2           \n"
    "QF_qlog2_2:                \n"
#endif
    "  LSR     r2,r0,#4         \n"
    "  BEQ     QF_qlog2_3       \n"
    "  ADD     r1,r1,#4         \n"
    "  MOV     r0,r2            \n"
    "QF_qlog2_3:                \n"
    "  LDR     r2,=QF_qlog2_LUT \n"
    "  LDRB    r0,[r2,r0]       \n"
    "  ADD     r0,r1,r0         \n"
    "  BX      lr               \n"
    "  .align                   \n"
    "QF_qlog2_LUT:              \n"
    "  .byte 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4"
    );
}

#endif // ARMv6-M

