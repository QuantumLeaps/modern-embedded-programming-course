;********************************************************************************************************
;                                              uC/OS-II
;                                        The Real-Time Kernel
;
;                    Copyright 1992-2020 Silicon Laboratories Inc. www.silabs.com
;
;                                 SPDX-License-Identifier: APACHE-2.0
;
;               This software is subject to an open source license and is distributed by
;                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
;                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
;
;********************************************************************************************************
; Modified from the original to interoperate with CMIS as follows:
; - renamed OS_CPU_PendSVHandler to CMSIS-compatible name PendSV_Handler
;
; Quantum Leaps, LLC. www.state-machine.com
; 2020-06-01
;********************************************************************************************************

;********************************************************************************************************
;
;                                             ARMv7-M Port
;
; Filename  : os_cpu_a.asm
; Version   : V2.93.00
;********************************************************************************************************
; For       : ARMv7-M Cortex-M
; Mode      : Thumb-2 ISA
; Toolchain : IAR EWARM
;********************************************************************************************************
; Note(s)   : (1) This port supports the ARM Cortex-M3, Cortex-M4 and Cortex-M7 architectures.
;             (2) It has been tested with the following Hardware Floating Point Unit.
;                 (a) Single-precision: FPv4-SP-D16-M and FPv5-SP-D16-M
;                 (b) Double-precision: FPv5-D16-M
;********************************************************************************************************

;********************************************************************************************************
;                                          PUBLIC FUNCTIONS
;********************************************************************************************************

    EXTERN  OSRunning                                           ; External references
    EXTERN  OSPrioCur
    EXTERN  OSPrioHighRdy
    EXTERN  OSTCBCur
    EXTERN  OSTCBHighRdy
    EXTERN  OSIntExit
    EXTERN  OSTaskSwHook
    EXTERN  OS_CPU_ExceptStkBase
    EXTERN  OS_KA_BASEPRI_Boundary


    PUBLIC  OSStartHighRdy                                      ; Functions declared in this file
    PUBLIC  OS_CPU_SR_Save
    PUBLIC  OS_CPU_SR_Restore
    PUBLIC  OSCtxSw
    PUBLIC  OSIntCtxSw
    PUBLIC  PendSV_Handler  ; QL was: OS_CPU_PendSVHandler

#ifdef __ARMVFP__
    PUBLIC  OS_CPU_FP_Reg_Push
    PUBLIC  OS_CPU_FP_Reg_Pop
#endif


;********************************************************************************************************
;                                               EQUATES
;********************************************************************************************************

NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI14   EQU     0xE000ED22                              ; System priority register (priority 14).
NVIC_PENDSV_PRI EQU           0xFF                              ; PendSV priority value (lowest).
NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.


;********************************************************************************************************
;                                     CODE GENERATION DIRECTIVES
;********************************************************************************************************

    RSEG CODE:CODE:NOROOT(2)
    THUMB


;********************************************************************************************************
;                                   FLOATING POINT REGISTERS PUSH
;                             void  OS_CPU_FP_Reg_Push (OS_STK  *stkPtr)
;
; Note(s) : 1) This function saves S16-S31 registers of the Floating Point Unit.
;
;           2) Pseudo-code is:
;              a) Push remaining FPU regs S16-S31 on process stack;
;              b) Update OSTCBCur->OSTCBStkPtr;
;********************************************************************************************************

#ifdef __ARMVFP__
OS_CPU_FP_Reg_Push
    MRS     R1, PSP                                             ; PSP is process stack pointer
    CBZ     R1, OS_CPU_FP_nosave                                ; Skip FP register save the first time

    VSTMDB  R0!, {S16-S31}
    LDR     R1, =OSTCBCur
    LDR     R2, [R1]
    STR     R0, [R2]
OS_CPU_FP_nosave
    BX      LR
#endif


;********************************************************************************************************
;                                   FLOATING POINT REGISTERS POP
;                             void  OS_CPU_FP_Reg_Pop (OS_STK  *stkPtr)
;
; Note(s) : 1) This function restores S16-S31 of the Floating Point Unit.
;
;           2) Pseudo-code is:
;              a) Restore regs S16-S31 of new process stack;
;              b) Update OSTCBHighRdy->OSTCBStkPtr pointer of new proces stack;
;********************************************************************************************************

#ifdef __ARMVFP__
OS_CPU_FP_Reg_Pop
    VLDMIA  R0!, {S16-S31}
    LDR     R1, =OSTCBHighRdy
    LDR     R2, [R1]
    STR     R0, [R2]
    BX      LR
#endif


;********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description : Disable/Enable Kernel aware interrupts by preserving the state of BASEPRI.  Generally speaking,
;               the state of the BASEPRI interrupt exception processing is stored in the local variable
;               'cpu_sr' & Kernel Aware interrupts are then disabled ('cpu_sr' is allocated in all functions
;               that need to disable Kernel aware interrupts). The previous BASEPRI interrupt state is restored
;               by copying 'cpu_sr' into the BASEPRI register.
;
; Prototypes  : OS_CPU_SR  OS_CPU_SR_Save   (OS_CPU_SR  new_basepri);
;               void       OS_CPU_SR_Restore(OS_CPU_SR  cpu_sr);
;
;
; Note(s)     : 1) These functions are used in general like this:
;
;                  void Task (void *p_arg)
;                  {
;                  #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register  */
;                      OS_CPU_SR  cpu_sr;
;                  #endif
;
;                          :
;                          :
;                      OS_ENTER_CRITICAL();             /* cpu_sr = OS_CPU_SR_Save(new_basepri);     */
;                          :
;                          :
;                      OS_EXIT_CRITICAL();              /* OS_CPU_RestoreSR(cpu_sr);                 */
;                          :
;                          :
;                  }
;
;               2) Increasing priority using a write to BASEPRI does not take effect immediately.
;                  (a) IMPLICATION  This erratum means that the instruction after an MSR to boost BASEPRI
;                      might incorrectly be preempted by an insufficient high priority exception.
;
;                  (b) WORKAROUND  The MSR to boost BASEPRI can be replaced by the following code sequence:
;
;                      CPSID i
;                      MSR to BASEPRI
;                      DSB
;                      ISB
;                      CPSIE i
;********************************************************************************************************

OS_CPU_SR_Save
    CPSID   I                                   ; Cortex-M7 errata notice. See Note #2
    PUSH   {R1}
    MRS     R1, BASEPRI
    MSR     BASEPRI, R0
    DSB
    ISB
    MOV     R0, R1
    POP    {R1}
    CPSIE   I
    BX      LR

OS_CPU_SR_Restore
    CPSID   I                                   ; Cortex-M7 errata notice. See Note #2
    MSR     BASEPRI, R0
    DSB
    ISB
    CPSIE   I
    BX      LR


;********************************************************************************************************
;                                         START MULTITASKING
;                                      void OSStartHighRdy(void)
;
; Note(s) : 1) This function triggers a PendSV exception (essentially, causes a context switch) to cause
;              the first task to start.
;
;           2) During task execution, PSP is used as the stack pointer.
;              When an exception occurs, the core will switch to MSP until the exception return.
;
;           3) OSStartHighRdy() MUST:
;              a) Setup PendSV exception priority to lowest;
;              b) Set initial PSP to 0, to tell context switcher this is first run;
;              c) Set the main stack to OS_CPU_ExceptStkBase
;              d) Set OSRunning to TRUE;
;              e) Get current high priority, OSPrioCur = OSPrioHighRdy;
;              f) Get current ready thread TCB, OSTCBCur = OSTCBHighRdy;
;              g) Get new process SP from TCB, SP = OSTCBHighRdy->OSTCBStkPtr;
;              h) Restore R0-R11 and R14 from new process stack;
;              i) Enable interrupts (tasks will run with interrupts enabled).
;********************************************************************************************************

OSStartHighRdy
    CPSID   I                                                   ; Prevent interruption during context switch
    LDR     R0, =NVIC_SYSPRI14                                  ; Set the PendSV exception priority
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0]

    MOVS    R0, #0                                              ; Set the PSP to 0 for initial context switch call
    MSR     PSP, R0

    LDR     R0, =OS_CPU_ExceptStkBase                           ; Initialize the MSP to the OS_CPU_ExceptStkBase
    LDR     R1, [R0]
    MSR     MSP, R1

    BL      OSTaskSwHook                                        ; Call OSTaskSwHook() for FPU Push & Pop

    LDR     R0, =OSRunning                                      ; OSRunning = TRUE
    MOVS    R1, #1
    STRB    R1, [R0]

    LDR     R0, =OSPrioCur                                      ; OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, =OSTCBCur                                       ; OSTCBCur  = OSTCBHighRdy;
    LDR     R1, =OSTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]                                            ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
    MSR     PSP, R0                                             ; Load PSP with new process SP

    MRS     R0, CONTROL
    ORR     R0, R0, #2
    MSR     CONTROL, R0
    ISB                                                         ; Sync instruction stream

    LDMFD    SP!, {R4-R11, LR}                                  ; Restore r4-11, lr from new process stack
    LDMFD    SP!, {R0-R3}                                       ; Restore r0, r3
    LDMFD    SP!, {R12, LR}                                     ; Load R12 and LR
    LDMFD    SP!, {R1, R2}                                      ; Load PC and discard xPSR
    CPSIE    I
    BX       R1


;********************************************************************************************************
;                       PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
;                   PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
;
; Note(s) : 1) OSCtxSw() is called when OS wants to perform a task context switch.  This function
;              triggers the PendSV exception which is where the real work is done.
;
;           2) OSIntCtxSw() is called by OSIntExit() when it determines a context switch is needed as
;              the result of an interrupt.  This function simply triggers a PendSV exception which will
;              be handled when there are no more interrupts active and interrupts are enabled.
;********************************************************************************************************

OSCtxSw
OSIntCtxSw
    LDR     R0, =NVIC_INT_CTRL                                  ; Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR


;********************************************************************************************************
;                                       HANDLE PendSV EXCEPTION
;                                   void OS_CPU_PendSVHandler(void)
;
; Note(s) : 1) PendSV is used to cause a context switch.  This is a recommended method for performing
;              context switches with Cortex-M.  This is because the Cortex-M auto-saves half of the
;              processor context on any exception, and restores same on return from exception.  So only
;              saving of R4-R11 & R14 is required and fixing up the stack pointers. Using the PendSV exception
;              this way means that context saving and restoring is identical whether it is initiated from
;              a thread or occurs due to an interrupt or exception.
;
;           2) Pseudo-code is:
;              a) Get the process SP
;              b) Save remaining regs r4-r11 & r14 on process stack;
;              c) Save the process SP in its TCB, OSTCBCur->OSTCBStkPtr = SP;
;              d) Call OSTaskSwHook();
;              e) Get current high priority, OSPrioCur = OSPrioHighRdy;
;              f) Get current ready thread TCB, OSTCBCur = OSTCBHighRdy;
;              g) Get new process SP from TCB, SP = OSTCBHighRdy->OSTCBStkPtr;
;              h) Restore R4-R11 and R14 from new process stack;
;              i) Perform exception return which will restore remaining context.
;
;           3) On entry into PendSV handler:
;              a) The following have been saved on the process stack (by processor):
;                 xPSR, PC, LR, R12, R0-R3
;              b) Processor mode is switched to Handler mode (from Thread mode)
;              c) Stack is Main stack (switched from Process stack)
;              d) OSTCBCur      points to the OS_TCB of the task to suspend
;                 OSTCBHighRdy  points to the OS_TCB of the task to resume
;
;           4) Since PendSV is set to lowest priority in the system (by OSStartHighRdy() above), we
;              know that it will only be run when no other exception or interrupt is active, and
;              therefore safe to assume that context being switched out was using the process stack (PSP).
;
;           5) Increasing priority using a write to BASEPRI does not take effect immediately.
;              (a) IMPLICATION  This erratum means that the instruction after an MSR to boost BASEPRI
;                  might incorrectly be preempted by an insufficient high priority exception.
;
;              (b) WORKAROUND  The MSR to boost BASEPRI can be replaced by the following code sequence:
;
;                  CPSID i
;                  MSR to BASEPRI
;                  DSB
;                  ISB
;                  CPSIE i
;********************************************************************************************************

PendSV_Handler  ; QL was: OS_CPU_PendSVHandler
    CPSID   I                                                   ; Cortex-M7 errata notice. See Note #5
    MOV32   R2, OS_KA_BASEPRI_Boundary                          ; Set BASEPRI priority level required for exception preemption
    LDR     R1, [R2]
    MSR     BASEPRI, R1
    DSB
    ISB
    CPSIE   I

    MRS     R0, PSP                                             ; PSP is process stack pointer
    STMFD   R0!, {R4-R11, R14}                                  ; Save remaining regs r4-11, R14 on process stack

    LDR     R5, =OSTCBCur                                       ; OSTCBCur->OSTCBStkPtr = SP;
    LDR     R1, [R5]
    STR     R0, [R1]                                            ; R0 is SP of process being switched out

                                                                ; At this point, entire context of process has been saved
    MOV     R4, LR                                              ; Save LR exc_return value
    BL      OSTaskSwHook                                        ; Call OSTaskSwHook() for FPU Push & Pop

    LDR     R0, =OSPrioCur                                      ; OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R1, =OSTCBHighRdy                                   ; OSTCBCur  = OSTCBHighRdy;
    LDR     R2, [R1]
    STR     R2, [R5]

    ORR     LR,  R4, #0x04                                      ; Ensure exception return uses process stack
    LDR     R0,  [R2]                                           ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
    LDMFD   R0!, {R4-R11, R14}                                  ; Restore r4-11, R14 from new process stack
    MSR     PSP, R0                                             ; Load PSP with new process SP

    MOV32   R2, #0                                              ; Restore BASEPRI priority level to 0
    MSR     BASEPRI, R2
    BX      LR                                                  ; Exception return will restore remaining context

    END
