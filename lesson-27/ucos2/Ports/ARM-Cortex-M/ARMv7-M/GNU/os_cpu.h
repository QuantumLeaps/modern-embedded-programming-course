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
* - renamed OS_CPU_PendSVHandler to CMSIS-compatible name PendSV_Handler
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
* Filename  : os_cpu.h
* Version   : V2.93.00
*********************************************************************************************************
* For       : ARMv7-M Cortex-M
* Mode      : Thumb-2 ISA
* Toolchain : GNU C Compiler
*********************************************************************************************************
* Note(s)   : (1) This port supports the ARM Cortex-M3, Cortex-M4 and Cortex-M7 architectures.
*             (2) It has been tested with the following Hardware Floating Point Unit.
*                 (a) Single-precision: FPv4-SP-D16-M and FPv5-SP-D16-M
*                 (b) Double-precision: FPv5-D16-M
*********************************************************************************************************
*/

#ifndef  OS_CPU_H
#define  OS_CPU_H

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                    /* See Note #1.                                       */
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*
* Note(s) : (1) If using uC/CPU then 'cpu_cfg.h' has already this define in its template. The user will
*               only need to include 'cpu_cfg.h' in 'app_cfg.h'; otherwise, define CPU_CFG_KA_IPL_BOUNDARY
*               and CPU_CFG_NVIC_PRIO_BITS in 'app_cfg.h'
*
*           (2) Since the port is using BASEPRI to separate kernel vs non-kernel aware ISR, please make sure
*               your external interrupt priorities are set accordingly. For example, if CPU_CFG_KA_IPL_BOUNDARY
*               is set to 4 then external interrupt priorities 4-15 will be kernel aware while priorities
*               0-3 will be use as non-kernel aware.
*
*           (3) Determines the interrupt programmable priority levels. This is normally specified in the
*               Microcontroller reference manual. 4-bits gives us 16 programmable priority levels.
*
*                     NVIC_IPRx
*                 7                0
*                +------------------+
*                |       PRIO       |
*                +------------------+
*
*                Bits[7:4] Priority mask bits
*                Bits[3:0] Reserved
*
*                In this example our CPU_CFG_NVIC_PRIO_BITS define should be set to 4 due to the processor
*                implementing only bits[7:4].
*********************************************************************************************************
*/

#ifndef  OS_CPU_EXCEPT_STK_SIZE
#define  OS_CPU_EXCEPT_STK_SIZE      256u        /* Default exception stack size is 256 OS_STK entries */
#endif

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
#define  OS_CPU_ARM_FP_EN              1u
#else
#define  OS_CPU_ARM_FP_EN              0u
#endif

#ifndef CPU_CFG_KA_IPL_BOUNDARY
#error  "CPU_CFG_KA_IPL_BOUNDARY        not #define'd in 'app_cfg.h'    "   /* See Note # 1 & 2        */
#else
#if (CPU_CFG_KA_IPL_BOUNDARY == 0u)
#error  "CPU_CFG_KA_IPL_BOUNDARY        should be > 0 "
#endif
#endif

#ifndef CPU_CFG_NVIC_PRIO_BITS
#error  "CPU_CFG_NVIC_PRIO_BITS         not #define'd in 'app_cfg.h'    "   /* See Note # 3            */
#else
#if (CPU_CFG_KA_IPL_BOUNDARY >= (1u << CPU_CFG_NVIC_PRIO_BITS))
#error  "CPU_CFG_KA_IPL_BOUNDARY        should not be set to higher than max programable priority level "
#endif
#endif


/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */


/*
*********************************************************************************************************
*                                              Cortex-M
*                                      Critical Section Management
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*             NOT IMPLEMENTED
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*             NOT IMPLEMENTED
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#define  OS_CRITICAL_METHOD   3u

#if OS_CRITICAL_METHOD == 3u
                                                 /* Save current BASEPRI priority lvl for exception... */
                                                 /* .. and set BASEPRI to CPU_CFG_KA_IPL_BOUNDARY      */
#define  OS_ENTER_CRITICAL()  do { cpu_sr = OS_CPU_SR_Save(CPU_CFG_KA_IPL_BOUNDARY << (8u - CPU_CFG_NVIC_PRIO_BITS));} while (0)
                                                 /* Restore CPU BASEPRI priority level.                */
#define  OS_EXIT_CRITICAL()   do { OS_CPU_SR_Restore(cpu_sr);} while (0)
#endif


/*
*********************************************************************************************************
*                                        Cortex-M Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1u                 /* Stack grows from HIGH to LOW memory on ARM         */

#define  OS_TASK_SW()         OSCtxSw()


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_CPU_EXT  OS_STK   OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE];
OS_CPU_EXT  OS_STK  *OS_CPU_ExceptStkBase;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3u                     /* See OS_CPU_A.ASM                                   */
OS_CPU_SR  OS_CPU_SR_Save         (OS_CPU_SR  new_basepri);
void       OS_CPU_SR_Restore      (OS_CPU_SR  cpu_sr);
#endif

void       OSCtxSw                (void);
void       OSIntCtxSw             (void);
void       OSStartHighRdy         (void);

                                                 /* See OS_CPU_C.C                                     */
void       OS_CPU_SysTickInit     (INT32U     cnts);
void       OS_CPU_SysTickInitFreq (INT32U     cpu_freq);

void       SysTick_Handler        (void);  /* QL was: OS_CPU_SysTickHandler  (void); */
void       PendSV_Handler         (void);  /* QL was: OS_CPU_PendSVHandler   (void); */

#if (OS_CPU_ARM_FP_EN > 0u)
void       OS_CPU_FP_Reg_Push     (OS_STK    *stkPtr);
void       OS_CPU_FP_Reg_Pop      (OS_STK    *stkPtr);
#endif

/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                                /* End of 'extern'al C lang linkage.                  */
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
