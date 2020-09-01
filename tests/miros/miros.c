/****************************************************************************
* MInimal Real-time Operating System (MIROS)
* version 0.23.2 (matching lesson 23 step 2)
*
* This software is a teaching aid to illustrate the concepts underlying
* a Real-Time Operating System (RTOS). The main goal of the software is
* simplicity and clear presentation of the concepts, but without dealing
* with various corner cases, portability, or error handling. For these
* reasons, the software is generally NOT intended or recommended for use
* in commercial applications.
*
* Copyright (C) 2018 Miro Samek. All Rights Reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Contact Information:
* https://www.state-machine.com
****************************************************************************/
#include <stdint.h>
#include "miros.h" /* OS API */

OSThread * volatile OS_curr; /* the currently runnig thread */
OSThread * volatile OS_next; /* the next thread to run */

#define OS_INT_DISABLE()    __asm volatile ("cpsid i")
#define OS_INT_ENABLE()     __asm volatile ("cpsie i")

#define SCB_ICSR    (*(uint32_t volatile *)0xE000ED04)
#define SCB_SYSPRI3 (*(uint32_t volatile *)0xE000ED20)

#define OS_CONTEXT_SWITCH() (SCB_ICSR = (uint32_t)(1U << 28))

#if (OS_SCHED_TYPE == 0) /* manual scheduler? */
    void OS_sched(void) {
        /* set the next thread OS_next manually */
        /* ... */
        if (OS_curr != OS_next) {
            OS_CONTEXT_SWITCH();
        }
}
#elif (OS_SCHED_TYPE == 1) /* round-robin scheduler? */
    static OSThread *l_threadFirst;
    static OSThread *l_threadLast;

    void OS_sched(void) {
        /* set next thread QS_next as the next in the list */
        OS_next = OS_next->next;
        if (OS_curr != OS_next) {
            OS_CONTEXT_SWITCH();
        }
    }
#else /* unsupported scheduler */
    #error "Scheduler type not supported in this MIROS version."
#endif

void OS_init(void) {
    OS_INT_DISABLE();
    /* SCB_SYSPRI3[16:23]: PendSV set to the lowest priority 0xFF */
    SCB_SYSPRI3 |= (0xFFU << 16);
}

void OS_run() {
    OS_curr = (OSThread *)0;
#if OS_SCHED_TYPE == 0
    OS_next = (OSThread *)0;
#elif OS_SCHED_TYPE == 1
    OS_next = l_threadFirst;
#endif

    OS_sched();
    OS_INT_ENABLE();
}

static void OS_threadRet(void) {
    while (1) {
    }
}

void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize)
{
    /* round down the stack top to the 8-byte boundary
    * NOTE: ARM Cortex-M stack grows down from hi -> low memory
    */
    uint32_t *sp =
        (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    /* synthesize the ARM Cortex-M exception stack frame...*/
    *(--sp) = (1U << 24);   /* xPSR  (just the THUMB bit) */
    *(--sp) = (uint32_t)threadHandler;  /* PC (the thread routine) */
    *(--sp) = (uint32_t)&OS_threadRet;  /* LR (return from thread) */
    *(--sp) = 0x0000000CU;  /* R12 */
    *(--sp) = 0x00000003U;  /* R3  */
    *(--sp) = 0x00000002U;  /* R2  */
    *(--sp) = 0x00000001U;  /* R1  */
    *(--sp) = (uint32_t)me; /* R0 (argument to the thread routine */
    *(--sp) = 0x0000000BU;  /* R11 */
    *(--sp) = 0x0000000AU;  /* R10 */
    *(--sp) = 0x00000009U;  /* R9  */
    *(--sp) = 0x00000008U;  /* R8  */
    *(--sp) = 0x00000007U;  /* R7  */
    *(--sp) = 0x00000006U;  /* R6  */
    *(--sp) = 0x00000005U;  /* R5  */
    *(--sp) = 0x00000004U;  /* R4  */

    /* save the top of the stack in the thread's attibute */
    me->sp = sp;

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

#if OS_SCHED_TYPE == 1
    if (l_threadFirst == (OSThread *)0) {
        l_threadFirst = me;
        l_threadLast = me;
    }
    /* insert the thread into a circular linked list... */
    me->next = l_threadLast;
    l_threadLast = me;
    l_threadFirst->next = l_threadLast;
#endif
}

/*****************************************************************************
* The PendSV exception handler is used for handling context switch and is
* the recommended context switch mechansism for ARM Cortex-M CPUs.
*
* The PendSV exception should have the lowest interrupt priority in the
* system (0xFF, see OS_init). All other exceptions and interrupts should
* have higher priority (lower numerical value of priority).
*
* Due to tail-chaining and its lowest priority, the PendSV exception will be
* entered very efficiently immediately after the exit from the *last* nested
* interrupt (or exception).
*****************************************************************************/
__asm
void PendSV_Handler(void) {
    IMPORT  OS_curr           /* extern variable */
    IMPORT  OS_next           /* extern variable */

    PRESERVE8                 /* preserve the 8-byte stack alignment */

    LDR     r1,=OS_curr       /* r1 := &OS_curr */
    LDR     r2,=OS_next       /* r2 := &OS_next */

    CPSID   i                 /* disable interrupts (set PRIMASK) */
    LDR     r3,[r1]           /* r3 := OS_curr */
    CMP     r3,#0             /* (r3 == 0)? */
    BEQ     PendSV_restore    /* branch if (r3 == 0) */

    MRS     r0,PSP            /* r0 := Process Stack Pointer */
    STMDB   r0!,{r4-r11}      /* save r4-r11 on top of the stack */
    STR     r0,[r3, #0]       /* OS_curr.sp := sp */

PendSV_restore
    LDR     r3,[r2]           /* r3 := OS_next */
    LDR     r0,[r3, #0]       /* r0 := OS_next.sp */
    STR     r3,[r1]           /* QS_curr := OS_next */
    CPSIE   i                 /* enable interrupts (clear PRIMASK) */

    LDMIA   r0!,{r4-r11}      /* restore r4-r11 */
    MSR     PSP,r0            /* Process Stack Pointer := r0 */
    ORR     lr,lr,#4          /* make sure PSP is used on return */
    BX      lr                /* return to the next thread */
}

