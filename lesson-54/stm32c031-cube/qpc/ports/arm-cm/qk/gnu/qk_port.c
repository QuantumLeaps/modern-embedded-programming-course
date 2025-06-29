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

// prototypes ----------------------------------------------------------------
void PendSV_Handler(void);
#ifdef QK_USE_IRQ_HANDLER           // if use IRQ...
void QK_USE_IRQ_HANDLER(void);
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

//int32_t volatile QF_int_lock_nest_; // not used
extern char const QF_port_module_[];
char const QF_port_module_[] = "qk_port";

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
__attribute__ ((naked, weak))
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
__attribute__ ((naked, weak))
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
__attribute__ ((naked, weak))
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
__attribute__ ((naked, weak))
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
// NOTE: The IRQ priorities established in QK_init() can be later changed
// by the application-level code.
void QK_init(void) {

#ifdef QF_USE_BASEPRI   //--------- use BASEPRI for critical section?

    // SCB_SYSPRI[2]:  SysTick
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (QF_USE_BASEPRI << 24U));

    // set all 240 possible IRQ priories to QF_USE_BASEPRI...
    for (uint_fast8_t n = 0U; n < (240U/sizeof(uint32_t)); ++n) {
        NVIC_IP[n] = (QF_USE_BASEPRI << 24U) | (QF_USE_BASEPRI << 16U)
                     | (QF_USE_BASEPRI << 8U) | QF_USE_BASEPRI;
    }

#endif                  //--------- use BASEPRI for critical section

    // SCB_SYSPRI[2]: PendSV set to priority 0xFF (lowest)
    SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (0xFFU << 16U));

#ifdef QK_USE_IRQ_NUM   //--------- QK IRQ specified?
    // The QK port is configured to use a given ARM Cortex-M IRQ #
    // to return to thread mode (default is to use the NMI exception)

    // clear the IRQ priority to 0 (highest)
    NVIC_IP[QK_USE_IRQ_NUM >> 2U] &= ~(0xFFU << (8U*(QK_USE_IRQ_NUM & 3U)));

    // enable the IRQ
    NVIC_EN[QK_USE_IRQ_NUM >> 5U] = (1U << (QK_USE_IRQ_NUM & 0x1FU));
#endif                  //--------- QK IRQ specified

#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the FPU is enabled by setting CP10 & CP11 Full Access
    SCB_CPACR = (SCB_CPACR | ((3UL << 20U) | (3UL << 22U)));

    // FPU automatic state preservation (ASPEN) lazy stacking (LSPEN)
    FPU_FPCCR = (FPU_FPCCR | (1U << 30U) | (1U << 31U));
#endif                  //--------- VFP available
}

//============================================================================
// The PendSV exception is used for handling asynchronous preemption in QK.
// The use of the PendSV exception is the recommended and most efficient
// method for performing context switches in ARM Cortex-M.
//
// The PendSV exception should have the lowest priority in the whole system
// (0xFF, see QK_init). All other exceptions and interrupts should have higher
// priority.
//
// Also, *all* "kernel aware" ISRs in the QK application must call the
// QK_ISR_EXIT() macro, which triggers PendSV when it detects a need for
// a context switch or asynchronous preemption.
//
// Due to tail-chaining and its lowest priority, the PendSV exception will be
// entered immediately after the exit from the *last* nested interrupt (or
// exception). In QK, this is exactly the time when the QK activator needs to
// handle the asynchronous preemption.
//
// NOTE:
// The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
// but for ARMv6-M the mnemonics MOV, LSR and ADD always set the condition
// flags in the PSR.
__attribute__ ((naked, optimize("-fno-stack-protector")))
void PendSV_Handler(void) {
__asm volatile (
    "  PUSH    {r0,lr}          \n" // save stack-aligner + EXC_RETURN

    //<<<<<<<<<<<<<<<<<<<<<<< CRITICAL SECTION BEGIN <<<<<<<<<<<<<<<<<<<<<<<<
    "  LDR     r0,=QF_int_disable_ \n"
    "  BLX     r0               \n" // call QF_int_disable_()

#ifdef QF_MEM_ISOLATE
    "  LDR     r0,=QF_onMemSys  \n"
    "  BLX     r0               \n" // call QF_onMemSys()
#endif

    // The PendSV exception handler can be preempted by an interrupt,
    // which might pend PendSV exception again. The following write to
    // ICSR[27] un-pends any such spurious instance of PendSV.
    "  LDR     r2,=" STRINGIFY(SCB_ICSR) "\n" // Interrupt Control and State
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#27        \n" // r1 := (1 << 27) (UNPENDSVSET bit)
    "  STR     r1,[r2]          \n" // ICSR[27] := 1 (unpend PendSV)

    // The QK activator must be called in a Thread mode, while this code
    // executes in the Handler mode of the PendSV exception. The switch
    // to the Thread mode is accomplished by returning from PendSV using
    // a fabricated exception stack frame, where the return address is
    // QK_activate_().
    //
    // NOTE: the QK activator is called with interrupts DISABLED and also
    // returns with interrupts DISABLED.
    "  LSR     r3,r1,#3         \n" // r3 := (r1 >> 3), set T bit (new xpsr)
    "  LDR     r2,=QK_activate_ \n" // address of QK_activate_
    "  SUB     r2,r2,#1         \n" // align Thumb-address at halfword (new pc)
    "  LDR     r1,=QK_thread_ret \n" // return address after the call  (new lr)

    "  SUB     sp,sp,#8*4       \n" // reserve space for exception stack frame
    "  ADD     r0,sp,#5*4       \n" // r0 := 5 registers below the SP
    "  STM     r0!,{r1-r3}      \n" // save xpsr,pc,lr

    "  MOV     r0,#6            \n"
    "  MVN     r0,r0            \n" // r0 := ~6 == 0xFFFFFFF9
#if (__ARM_ARCH != 6)   //--------- if ARMv7-M and higher...
    "  DSB                      \n" // ARM Erratum 838869
#endif                  //--------- ARMv7-M and higher
    "  BX      r0               \n" // exception-return to the QK activator
    );
}

//============================================================================
// QK_thread_ret is a helper function executed when the QXK activator returns.
//
// After the QK activator returns, we need to resume the preempted
// thread. However, this must be accomplished by a return-from-exception,
// while we are still in the *thread* context. The switching to the
// exception context is accomplished by triggering the NMI exception or
// the selected IRQ (if macro #QK_USE_IRQ_NUM is defined).
//
// NOTE: QK_thread_ret is entered with interrupts DISABLED.
// NOTE: any function calls made from this code corrupt lr, but it is
// NOT used to return from QK_thread_ret(). Instead QK_thread_ret()
// "returns" by entering an exception (either NMI or IRQ).
//
// NOTE:
// The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
// but for ARMv6-M the mnemonics MOV, LSR and ADD always set the condition
// flags in the PSR.
__attribute__ ((naked, used))
void QK_thread_ret(void) {
    __asm volatile (
#ifdef QF_MEM_ISOLATE
    "  LDR     r0,=QF_onMemApp  \n"
    "  BLX     r0               \n" // call QF_onMemApp()
#endif

#if (__ARM_ARCH == 6)   //--------- if ARMv6-M...

#ifdef QK_USE_IRQ_NUM   //--------- IRQ defined, use the specified IRQ
    "  LDR     r0,=" STRINGIFY(NVIC_PEND + ((QK_USE_IRQ_NUM >> 5) << 2)) "\n"
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#" STRINGIFY(QK_USE_IRQ_NUM & 0x1F) "\n" // r1 := IRQ bit
    "  STR     r1,[r0]          \n" // pend the IRQ

    "  LDR     r0,=QF_int_enable_ \n"
    "  BLX     r0               \n" // enable interrupts with PRIMASK
    // This code stops here and continues in the IRQ handler

#else                   //--------- use the NMI (default)
    "  LDR     r0,=" STRINGIFY(SCB_ICSR) "\n" // Interrupt Control and State
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#31        \n" // r1 := (1 << 31) (NMI bit)
    "  STR     r1,[r0]          \n" // ICSR[31] := 1 (pend NMI)
    // This code stops here and continues in the NMI handler
    // NOTE: interrupts remain *disabled*.

#endif                  //--------- use the NMI

#else                   //--------- ARMv7-M and higher...

#ifdef QF_USE_BASEPRI   //--------- QF_USE_BASEPRI
    "  CPSID   i                \n" // disable interrupts with PRIMASK
    "  LDR     r0,=QF_int_enable_ \n"
    "  BLX     r0               \n" // enable interrupts with BASEPRI
#endif                  //--------- QF_USE_BASEPRI
    // NOTE: interrupts remain disabled with PRIMASK

#ifdef __ARM_FP         //--------- if VFP available...
    // make sure that the following IRQ/NMI will NOT use the VFP stack frame
    "  MRS     r0,CONTROL       \n" // r0 := CONTROL
    "  BIC     r0,r0,#4         \n" // r0 := r0 & ~4 (FPCA bit)
    "  MSR     CONTROL,r0       \n" // CONTROL := r0 (clear CONTROL[2] FPCA bit)
    "  ISB                      \n" // ISB after MSR CONTROL (ARM AN321,Sect.4.16)
#endif                  //--------- VFP available

#ifdef QK_USE_IRQ_NUM   //--------- IRQ defined, use the specified IRQ
    "  LDR     r0,=" STRINGIFY(NVIC_PEND + ((QK_USE_IRQ_NUM >> 5) << 2)) "\n"
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#" STRINGIFY(QK_USE_IRQ_NUM & 0x1F) "\n" // r1 := IRQ bit
    "  STR     r1,[r0]          \n" // pend the IRQ
#ifdef QF_USE_BASEPRI  //--------- QF_USE_BASEPRI
    "  CPSIE   i                \n" // enable interrupts with PRIMASK
#else                  //--------- interrupt disabling with PRIMASK
    "  LDR     r0,=QF_int_enable_ \n"
    "  BLX     r0               \n" // enable interrupts with BASEPRI
#endif                  //--------- interrupt disabling with PRIMASK
    // This code stops here and continues in the IRQ handler

#else                   //--------- use the NMI (default)
    "  LDR     r0,=" STRINGIFY(SCB_ICSR) "\n" // Interrupt Control and State
    "  MOV     r1,#1            \n"
    "  LSL     r1,r1,#31        \n" // r1 := (1 << 31) (NMI bit)
    "  STR     r1,[r0]          \n" // ICSR[31] := 1 (pend NMI)
    // NOTE: this code stops here and continues in the NMI handler
    // NOTE: PRIMASK remains *disabled*.

#endif                  //--------- use the NMI

#endif                  //--------- ARMv7-M and higher

    // execution should *never* reach here, so the following code
    // enters the Q_onError() error handler
    "  LDR     r0,=QF_int_disable_ \n"
    "  BLX     r0               \n" // call QF_int_disable_()
    "  LDR     r0,=QF_port_module_ \n"
    "  MOV     r1,#121          \n"
    "  LDR     r2,=Q_onError    \n"
    "  BX      r2               \n"
    );
}

//============================================================================
// The following exception handlers are used for returning back to the
// preempted *thread* context.

#ifdef QK_USE_IRQ_NUM   //--------- IRQ defined, use the specified IRQ

// NOTE: The IRQ Handler is entered with interrupts already ENABLED
__attribute__ ((naked))
void QK_USE_IRQ_HANDLER(void) {
__asm volatile (
    "  ADD     sp,sp,#(8*4)     \n" // remove one 8-register exception frame
    "  POP     {r0,pc}          \n" // pop stack aligner and EXC_RETURN to pc
    // NOTE: this causes exception-return to the preempted *thread* context
    );
}

#else                   //--------- IRQ NOT defined, use the NMI (default)

// NOTE: The NMI_Handler() is entered with interrupts still DISABLED!
__attribute__ ((naked))
void NMI_Handler(void) {
__asm volatile (
    "  ADD     sp,sp,#(8*4)     \n" // remove one 8-register exception frame

#ifdef QF_USE_BASEPRI  //--------- QF_USE_BASEPRI
    // NOTE: QF_int_enable_() already called
    "  CPSIE   i                \n" // enable interrupts with PRIMASK
#else                  //--------- interrupt disabling with PRIMASK
    "  LDR     r0,=QF_int_enable_ \n"
    "  BLX     r0               \n" // enable interrupts with PRIMASK

    // NOTE: calling QF_int_enable_() corrupted the lr (EXC_RETURN), but
    // it is NOT used to used to return from the exception. (See POP {r0,pc})
#endif                  //--------- interrupt disabling with PRIMASK
    "  POP     {r0,pc}          \n" // pop stack aligner and EXC_RETURN to pc
    );
    // NOTE: this causes exception-return to the preempted *thread* context
}

#endif                  //--------- use the NMI

//============================================================================
#if (__ARM_ARCH == 6) // if ARMv6-M...

// hand-optimized quick LOG2 in assembly (no CLZ instruction in ARMv6-M)
// NOTE:
// The inline GNU assembler does not accept mnemonics MOVS, LSRS and ADDS,
// but for ARMv6-M the mnemonics MOV, LSR and ADD always set the condition
// flags in the PSR.
__attribute__ ((naked, optimize("-fno-stack-protector")))
uint_fast8_t QF_qlog2(uint32_t x) {
    Q_UNUSED_PAR(x);
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

//============================================================================
#ifndef QF_NDUMMY_SYS
// dummy definition of stdio functions that cause
// "warning: xxxx is not implemented and will always fail"

int _close(int fd) {
    (void)fd;
    return 0;
}
long _lseek(int fd, long offset, int origin) {
    (void)fd;
    (void)offset;
    (void)origin;
    return 0L;
}
int _read(int const fd, void * const buffer, unsigned const buffer_size) {
    (void)fd;
    (void)buffer;
    (void)buffer_size;
    return 0;
}
int _write(int fd, const void *buffer, unsigned int count) {
    (void)fd;
    (void)buffer;
    (void)count;
    return 0;
}

#endif // QF_NDUMMY_SYS

