/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*
* Modified from the original to interoperate with CMIS as follows:
* - renamed OS_CPU_SysTickHandler to CMSIS-compatible name SysTick_Handler
*
* Quantum Leaps, LLC. www.state-machine.com
* 2020-06-01
*/

/*
*********************************************************************************************************
*
*                                             ARMv7-M Port
*
* File    : os_cpu_c.c
* Version : V2.93.00
*********************************************************************************************************
* For     : ARMv7-M Cortex-M
* Mode    : Thumb-2 ISA
*********************************************************************************************************
* Note(s) : (1) This port supports the ARM Cortex-M3, Cortex-M4 and Cortex-M7 architectures.
*           (2) It has been tested with the following Hardware Floating Point Unit.
*               (a) Single-precision: FPv4-SP-D16-M and FPv5-SP-D16-M
*               (b) Double-precision: FPv5-D16-M
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  INT16U  OSTmrCtr;
#endif

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

INT32U  OS_KA_BASEPRI_Boundary;                                 /* Base Priority boundary.                              */


/*
*********************************************************************************************************
*                                          SYS TICK DEFINES
*********************************************************************************************************
*/

#define  OS_CPU_CM_SYST_CSR         (*((volatile INT32U *)0xE000E010uL)) /* SysTick Ctrl & Status Reg.                  */
#define  OS_CPU_CM_SYST_RVR         (*((volatile INT32U *)0xE000E014uL)) /* SysTick Reload  Value Reg.                  */
#define  OS_CPU_CM_SYST_CVR         (*((volatile INT32U *)0xE000E018uL)) /* SysTick Current Value Reg.                  */
#define  OS_CPU_CM_SYST_CALIB       (*((volatile INT32U *)0xE000E01CuL)) /* SysTick Cal     Value Reg.                  */
#define  OS_CPU_CM_SCB_SHPRI1       (*((volatile INT32U *)0xE000ED18uL)) /* System Handlers  4 to  7 Prio.              */
#define  OS_CPU_CM_SCB_SHPRI2       (*((volatile INT32U *)0xE000ED1CuL)) /* System Handlers  8 to 11 Prio.              */
#define  OS_CPU_CM_SCB_SHPRI3       (*((volatile INT32U *)0xE000ED20uL)) /* System Handlers 12 to 15 Prio.              */


#define  OS_CPU_CM_SYST_CSR_COUNTFLAG                     0x00010000uL   /* Count flag.                                 */
#define  OS_CPU_CM_SYST_CSR_CLKSOURCE                     0x00000004uL   /* Clock Source.                               */
#define  OS_CPU_CM_SYST_CSR_TICKINT                       0x00000002uL   /* Interrupt enable.                           */
#define  OS_CPU_CM_SYST_CSR_ENABLE                        0x00000001uL   /* Counter mode.                               */


/*
*********************************************************************************************************
*                                         FLOATING POINT DEFINES
*
* Note(s) : 1) Enabled FP lazy stacking and enable automatic state saving.
*********************************************************************************************************
*/

#define  OS_CPU_CM_FP_FPCCR         (*((volatile INT32U *)0xE000EF34uL)) /* Floating-Point Context Control Reg.         */
#define  OS_CPU_CM_FPCCR_LAZY_STK                         0xC0000000uL   /* See Note 1.                                 */


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*              2) When using hardware floating point please do the following during the reset handler:
*                 a) Set full access for CP10 & CP11 bits in CPACR register.
*                 b) Set bits ASPEN and LSPEN in FPCCR register.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookBegin (void)
{
    INT32U   size;
    OS_STK  *pstk;
#if (OS_CPU_ARM_FP_EN > 0u)
    INT32U   reg_val;
#endif
                                                                /* Clear exception stack for stack checking.            */
    pstk = &OS_CPU_ExceptStk[0];
    size = OS_CPU_EXCEPT_STK_SIZE;
    while (size > 0u) {
        size--;
       *pstk++ = (OS_STK)0;
    }

                                                                /* Align the ISR stack to 8-bytes                       */
    OS_CPU_ExceptStkBase = (OS_STK *)&OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE];
    OS_CPU_ExceptStkBase = (OS_STK *)((OS_STK)(OS_CPU_ExceptStkBase) & 0xFFFFFFF8);

#if (OS_CPU_ARM_FP_EN > 0u)
    reg_val = OS_CPU_CM_FP_FPCCR;                               /* Check the floating point mode.                       */
    if ((reg_val & OS_CPU_CM_FPCCR_LAZY_STK) != OS_CPU_CM_FPCCR_LAZY_STK) {
        while (1u) {                                            /* See Note (2).                                        */
            ;
        }
    }
#endif

#if OS_TMR_EN > 0u
    OSTmrCtr = 0u;
#endif

                                                                /* Set BASEPRI boundary from the configuration.         */
    OS_KA_BASEPRI_Boundary = (INT32U)(CPU_CFG_KA_IPL_BOUNDARY << (8u - CPU_CFG_NVIC_PRIO_BITS));
}
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookEnd (void)
{

}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskDelHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskIdleHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : ptcb      is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskReturnHook (OS_TCB  *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(ptcb);
#else
    (void)ptcb;
#endif
}
#endif


/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskStatHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskStatHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : (1) Interrupts are enabled when task starts executing.
*
*              (2) All tasks run in Thread mode, using process stack.
*
*              (3) There are two different stack frames depending on whether the Floating-Point(FP)
*                  co-processor is enabled or not.
*
*                  (a) The stack frame shown in the diagram is used when the Co-processor Access Control
*                      Register(CPACR) is disabling the Floating Point Unit. In this case, the FP
*                      registers(S0- S31) & FP Status Control(FPSCR) register are not saved in the stack frame.
*
*                  (b) The stack frame shown in the diagram is used when the Floating Point Unit is enabled,
*                      that is, CP10 and CP11 field in CPACR are ones and FPCCR sets bits ASPEN and LSPEN to 1.
*
*                      (1) When enabling the FPU through CPACR, make sure to set bits ASPEN and LSPEN in the
*                          Floating-Point Context Control Register (FPCCR).
*
*                                          +-------------+
*                                          |             |
*                                          +-------------+
*                                          |             |
*                                          +-------------+
*                                          |    FPSCR    |
*                                          +-------------+
*                                          |     S15     |
*                                          +-------------+
*                                          |     S14     |
*                                          +-------------+
*                                          |     S13     |
*                                          +-------------+
*                                                .
*                                                .
*                                                .
*                                          +-------------+
*                                          |     S2      |
*                                          +-------------+
*                                          |     S1      |
*                    +-------------+       +-------------+
*                    |             |       |     S0      |
*                    +-------------+       +-------------+
*                    |    xPSR     |       |    xPSR     |
*                    +-------------+       +-------------+
*                    | Return Addr |       | Return Addr |
*                    +-------------+       +-------------+
*                    |  LR(R14)    |       |   LR(R14)   |
*                    +-------------+       +-------------+
*                    |    R12      |       |     R12     |
*                    +-------------+       +-------------+
*                    |    R3       |       |     R3      |
*                    +-------------+       +-------------+
*                    |    R2       |       |     R0      |
*                    +-------------+       +-------------+
*                    |    R1       |       |     R1      |
*                    +-------------+       +-------------+
*                    |    R0       |       |     R0      |
*                    +-------------+       +-------------+
*                    | EXEC_RETURN |       | EXEC_RETURN |
*                    +-------------+       +-------------+
*                    |    R11      |       |     R11     |
*                    +-------------+       +-------------+
*                    |    R10      |       |     R10     |
*                    +-------------+       +-------------+
*                    |    R9       |       |     R9      |
*                    +-------------+       +-------------+
*                    |    R8       |       |     R8      |
*                    +-------------+       +-------------+
*                    |    R7       |       |     R7      |
*                    +-------------+       +-------------+
*                    |    R6       |       |     R6      |
*                    +-------------+       +-------------+
*                    |    R5       |       |     R5      |
*                    +-------------+       +-------------+
*                    |    R4       |       |     R4      |
*                    +-------------+       +-------------+
*                          (a)             |     S31     |
*                                          +-------------+
*                                          |     S30     |
*                                          +-------------+
*                                          |     S29     |
                                           +-------------+
*                                                .
*                                                .
*                                                .
*                                          +-------------+
*                                          |     S17     |
                                           +-------------+
*                                          |     S16     |
*                                          +-------------+
*                                               (b)
*
*             (4) The SP must be 8-byte aligned in conforming to the Procedure Call Standard for the ARM architecture
*
*                    (a) Section 2.1 of the  ABI for the ARM Architecture Advisory Note. SP must be 8-byte aligned
*                        on entry to AAPCS-Conforming functions states :
*
*                        The Procedure Call Standard for the ARM Architecture [AAPCS] requires primitive
*                        data types to be naturally aligned according to their sizes (for size = 1, 2, 4, 8 bytes).
*                        Doing otherwise creates more problems than it solves.
*
*                        In return for preserving the natural alignment of data, conforming code is permitted
*                        to rely on that alignment. To support aligning data allocated on the stack, the stack
*                        pointer (SP) is required to be 8-byte aligned on entry to a conforming function. In
*                        practice this requirement is met if:
*
*                           (1) At each call site, the current size of the calling function's stack frame is a multiple of 8 bytes.
*                               This places an obligation on compilers and assembly language programmers.
*
*                           (2) SP is a multiple of 8 when control first enters a program.
*                               This places an obligation on authors of low level OS, RTOS, and runtime library
*                               code to align SP at all points at which control first enters
*                               a body of (AAPCS-conforming) code.
*
*                       In turn, this requires the value of SP to be aligned to 0 modulo 8:
*
*                           (3) By exception handlers, before calling AAPCS-conforming code.
*
*                           (4) By OS/RTOS/run-time system code, before giving control to an application.
*
*                 (b) Section 2.3.1 corrective steps from the the SP must be 8-byte aligned on entry
*                     to AAPCS-conforming functions advisory note also states.
*
*                     " This requirement extends to operating systems and run-time code for all architecture versions
*                       prior to ARMV7 and to the A, R and M architecture profiles thereafter. Special considerations
*                       associated with ARMV7M are discussed in section 2.3.3"
*
*                     (1) Even if the SP 8-byte aligment is not a requirement for the ARMv7M profile, the stack is aligned
*                         to 8-byte boundaries to support legacy execution enviroments.
*
*                 (c) Section 5.2.1.2 from the Procedure Call Standard for the ARM
*                     architecture states :  "The stack must also conform to the following
*                     constraint at a public interface:
*
*                     (1) SP mod 8 = 0. The stack must be double-word aligned"
*
*                 (d) From the ARM Technical Support Knowledge Base. 8 Byte stack aligment.
*
*                     "8 byte stack alignment is a requirement of the ARM Architecture Procedure
*                      Call Standard [AAPCS]. This specifies that functions must maintain an 8 byte
*                      aligned stack address (e.g. 0x00, 0x08, 0x10, 0x18, 0x20) on all external
*                      interfaces. In practice this requirement is met if:
*
*                      (1) At each external interface, the current stack pointer
*                          is a multiple of 8 bytes.
*
*                      (2) Your OS maintains 8 byte stack alignment on its external interfaces
*                          e.g. on task switches"
*
*              (5) Exception Return Behavior(EXEC_RETURN)
*                  0xFFFFFFFD      Return to Thread mode, exception return uses non-floating point state
*                                  from the PSP and execution uses PSP after return.
*
*                  0xFFFFFFED      Return to Thread mode, exception return uses floating point state
*                                  from the PSP and execution uses PSP after return.
**********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void    (*task)(void *p_arg),
                        void     *p_arg,
                        OS_STK   *ptos,
                        INT16U    opt)
{
    OS_STK  *p_stk;


    (void)opt;                                                  /* 'opt' is not used, prevent warning                   */
    p_stk      = ptos + 1u;                                     /* Load stack pointer                                   */
                                                                /* Align the stack to 8-bytes.                          */
    p_stk      = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFF8u);
                                                                /* Registers stacked as if auto-saved on exception      */
#if (OS_CPU_ARM_FP_EN > 0u)                                     /* FPU auto-saved registers.                            */
     --p_stk;
    *(--p_stk) = (OS_STK)0x02000000u;                           /* FPSCR                                                */
                                                                /* Initialize S0-S15 floating point registers           */
    *(--p_stk) = (OS_STK)0x41700000u;                           /* S15                                                  */
    *(--p_stk) = (OS_STK)0x41600000u;                           /* S14                                                  */
    *(--p_stk) = (OS_STK)0x41500000u;                           /* S13                                                  */
    *(--p_stk) = (OS_STK)0x41400000u;                           /* S12                                                  */
    *(--p_stk) = (OS_STK)0x41300000u;                           /* S11                                                  */
    *(--p_stk) = (OS_STK)0x41200000u;                           /* S10                                                  */
    *(--p_stk) = (OS_STK)0x41100000u;                           /* S9                                                   */
    *(--p_stk) = (OS_STK)0x41000000u;                           /* S8                                                   */
    *(--p_stk) = (OS_STK)0x40E00000u;                           /* S7                                                   */
    *(--p_stk) = (OS_STK)0x40C00000u;                           /* S6                                                   */
    *(--p_stk) = (OS_STK)0x40A00000u;                           /* S5                                                   */
    *(--p_stk) = (OS_STK)0x40800000u;                           /* S4                                                   */
    *(--p_stk) = (OS_STK)0x40400000u;                           /* S3                                                   */
    *(--p_stk) = (OS_STK)0x40000000u;                           /* S2                                                   */
    *(--p_stk) = (OS_STK)0x3F800000u;                           /* S1                                                   */
    *(--p_stk) = (OS_STK)0x00000000u;                           /* S0                                                   */
#endif
    *(--p_stk) = (OS_STK)0x01000000uL;                          /* xPSR                                                 */
    *(--p_stk) = (OS_STK)task;                                  /* Entry Point                                          */
    *(--p_stk) = (OS_STK)OS_TaskReturn;                         /* R14 (LR)                                             */
    *(--p_stk) = (OS_STK)0x12121212uL;                          /* R12                                                  */
    *(--p_stk) = (OS_STK)0x03030303uL;                          /* R3                                                   */
    *(--p_stk) = (OS_STK)0x02020202uL;                          /* R2                                                   */
    *(--p_stk) = (OS_STK)0x01010101uL;                          /* R1                                                   */
    *(--p_stk) = (OS_STK)p_arg;                                 /* R0 : argument                                        */

#if (OS_CPU_ARM_FP_EN > 0u)
    *(--p_stk) = (OS_STK)0xFFFFFFEDuL;                          /* R14: EXEC_RETURN; See Note 5                         */
#else
    *(--p_stk) = (OS_STK)0xFFFFFFFDuL;                          /* R14: EXEC_RETURN; See Note 5                         */
#endif
                                                                /* Remaining registers saved on process stack           */
    *(--p_stk) = (OS_STK)0x11111111uL;                          /* R11                                                  */
    *(--p_stk) = (OS_STK)0x10101010uL;                          /* R10                                                  */
    *(--p_stk) = (OS_STK)0x09090909uL;                          /* R9                                                   */
    *(--p_stk) = (OS_STK)0x08080808uL;                          /* R8                                                   */
    *(--p_stk) = (OS_STK)0x07070707uL;                          /* R7                                                   */
    *(--p_stk) = (OS_STK)0x06060606uL;                          /* R6                                                   */
    *(--p_stk) = (OS_STK)0x05050505uL;                          /* R5                                                   */
    *(--p_stk) = (OS_STK)0x04040404uL;                          /* R4                                                   */

#if (OS_CPU_ARM_FP_EN > 0u)
                                                                /* Initialize S16-S31 floating point registers          */
    *(--p_stk) = (OS_STK)0x41F80000u;                           /* S31                                                  */
    *(--p_stk) = (OS_STK)0x41F00000u;                           /* S30                                                  */
    *(--p_stk) = (OS_STK)0x41E80000u;                           /* S29                                                  */
    *(--p_stk) = (OS_STK)0x41E00000u;                           /* S28                                                  */
    *(--p_stk) = (OS_STK)0x41D80000u;                           /* S27                                                  */
    *(--p_stk) = (OS_STK)0x41D00000u;                           /* S26                                                  */
    *(--p_stk) = (OS_STK)0x41C80000u;                           /* S25                                                  */
    *(--p_stk) = (OS_STK)0x41C00000u;                           /* S24                                                  */
    *(--p_stk) = (OS_STK)0x41B80000u;                           /* S23                                                  */
    *(--p_stk) = (OS_STK)0x41B00000u;                           /* S22                                                  */
    *(--p_stk) = (OS_STK)0x41A80000u;                           /* S21                                                  */
    *(--p_stk) = (OS_STK)0x41A00000u;                           /* S20                                                  */
    *(--p_stk) = (OS_STK)0x41980000u;                           /* S19                                                  */
    *(--p_stk) = (OS_STK)0x41900000u;                           /* S18                                                  */
    *(--p_stk) = (OS_STK)0x41880000u;                           /* S17                                                  */
    *(--p_stk) = (OS_STK)0x41800000u;                           /* S16                                                  */
#endif

    return (p_stk);
}


/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TASK_SW_HOOK_EN > 0u)
void  OSTaskSwHook (void)
{

#if (OS_CPU_ARM_FP_EN > 0u)
    OS_CPU_FP_Reg_Push(OSTCBCur->OSTCBStkPtr);                  /* Push the FP registers of the current task.           */
#endif

#if OS_APP_HOOKS_EN > 0u
    App_TaskSwHook();
#endif

    OS_TRACE_TASK_SWITCHED_IN(OSTCBHighRdy);

#if (OS_CPU_ARM_FP_EN > 0u)
    OS_CPU_FP_Reg_Pop(OSTCBHighRdy->OSTCBStkPtr);               /* Pop the FP registers of the highest ready task.      */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTCBInitHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TimeTickHook();
#endif

#if OS_TMR_EN > 0u
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0u;
        OSTmrSignal();
    }
#endif
}
#endif


/*
*********************************************************************************************************
*                                          SYS TICK HANDLER
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-II tick
*              interrupt.
*
* Arguments  : None.
*
* Note(s)    : 1) This function MUST be placed on entry 15 of the Cortex-M vector table.
*********************************************************************************************************
*/

void SysTick_Handler (void) /* QL was: void  OS_CPU_SysTickHandler (void) */
{
#if OS_CRITICAL_METHOD == 3u                                    /* Allocate storage for CPU status register             */
    OS_CPU_SR  cpu_sr;
#endif


    OS_ENTER_CRITICAL();
    OSIntEnter();                                               /* Tell uC/OS-II that we are starting an ISR            */
    OS_EXIT_CRITICAL();

    OSTimeTick();                                               /* Call uC/OS-II's OSTimeTick()                         */

    OSIntExit();                                                /* Tell uC/OS-II that we are leaving the ISR            */
}


/*
*********************************************************************************************************
*                                         INITIALIZE SYS TICK
*
* Description: Initialize the SysTick using the CPU clock frequency.
*
* Arguments  : cpu_freq     CPU clock frequency.
*
* Note(s)    : 1) This function MUST be called after OSStart() & after processor initialization.
*
*              2) Either OS_CPU_SysTickInitFreq or OS_CPU_SysTickInit() can be called.
*********************************************************************************************************
*/

void  OS_CPU_SysTickInitFreq (INT32U  cpu_freq)
{
    INT32U  cnts;


    cnts = (cpu_freq / (INT32U)OS_TICKS_PER_SEC);               /* Determine nbr SysTick cnts between two OS tick intr. */

    OS_CPU_SysTickInit(cnts);
}


/*
*********************************************************************************************************
*                                         INITIALIZE SYS TICK
*
* Description: Initialize the SysTick using the number of counts between two ticks.
*
* Arguments  : cnts     Number of SysTick counts between two OS tick interrupts.
*
* Note(s)    : 1) This function MUST be called after OSStart() & after processor initialization.
*
*              2) Either OS_CPU_SysTickInitFreq or OS_CPU_SysTickInit() can be called.
*********************************************************************************************************
*/

void  OS_CPU_SysTickInit (INT32U  cnts)
{
    INT32U  prio;
    INT32U  basepri;


                                                                /* Set BASEPRI boundary from the configuration.         */
    basepri               = (INT32U)(CPU_CFG_KA_IPL_BOUNDARY << (8u - CPU_CFG_NVIC_PRIO_BITS));
    OS_CPU_CM_SYST_RVR    = cnts - 1u;                          /* Set Reload register.                                 */

                                                                /* Set SysTick handler prio.                            */
    prio                  =  OS_CPU_CM_SCB_SHPRI3;
    prio                 &=  0x00FFFFFFu;
    prio                 |= (basepri << 24u);
    OS_CPU_CM_SCB_SHPRI3  = prio;

                                                                /* Enable timer.                                        */
    OS_CPU_CM_SYST_CSR   |= OS_CPU_CM_SYST_CSR_CLKSOURCE |
                            OS_CPU_CM_SYST_CSR_ENABLE;
                                                                /* Enable timer interrupt.                              */
    OS_CPU_CM_SYST_CSR   |= OS_CPU_CM_SYST_CSR_TICKINT;
}
