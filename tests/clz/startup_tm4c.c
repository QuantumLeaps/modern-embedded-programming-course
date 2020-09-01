/*****************************************************************************
// This TM4C startup file has been adapted from startup_ewarm.c
// Quantum Leaps on 28-Jul-2014
// www.state-machine.com
*****************************************************************************/
//*****************************************************************************
//
// startup_ewarm.c - Startup code for use with IAR's Embedded Workbench,
//                   version 5.
//
// Copyright (c) 2012-2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 2.0.1.11577 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************
#include "tm4c_cmsis.h"

/* Enable the IAR extensions for this source file */
#pragma language=extended

/* Function prototypes -----------------------------------------------------*/
void __iar_program_start(void);   /* IAR startup code */
void Spurious_Handler(void);      /* spurious exception/interrupt handler */


/* CSTACK section limits (created by the IAR linker) -----------------------*/
extern int CSTACK$$Base;  /* symbol created by the IAR linker */
extern int CSTACK$$Limit; /* symbol created by the IAR linker */

/* exception and interrupt vector table ------------------------------------*/
typedef void (*handler)(void); /* pointer to handler function type */

/*..........................................................................*/
__root handler const  __vector_table[] @ ".intvec" = {
    (handler)&CSTACK$$Limit,  /* The initial stack pointer    */
    &__iar_program_start,     /* Reset Handler                */
    &NMI_Handler,             /* NMI Handler                  */
    &HardFault_Handler,       /* Hard Fault Handler           */
    &MemManage_Handler,       /* MPU Fault Handler            */
    &BusFault_Handler,        /* Bus Fault Handler            */
    &UsageFault_Handler,      /* Usage Fault Handler          */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &SVC_Handler,             /* SVCall Handler               */
    &DebugMon_Handler,        /* Debug Monitor Handler        */
    &Spurious_Handler,        /* Reserved                     */
    &PendSV_Handler,          /* PendSV Handler               */
    &SysTick_Handler,         /* SysTick Handler              */

    /* external interrupts (IRQs) ... */
    &GPIOPortA_IRQHandler,    /* GPIO Port A                  */
    &GPIOPortB_IRQHandler,    /* GPIO Port B                  */
    &GPIOPortC_IRQHandler,    /* GPIO Port C                  */
    &GPIOPortD_IRQHandler,    /* GPIO Port D                  */
    &GPIOPortE_IRQHandler,    /* GPIO Port E                  */
    &UART0_IRQHandler,        /* UART0 Rx and Tx              */
    &UART1_IRQHandler,        /* UART1 Rx and Tx              */
    &SSI0_IRQHandler,         /* SSI0 Rx and Tx               */
    &I2C0_IRQHandler,         /* I2C0 Master and Slave        */
    &PWMFault_IRQHandler,     /* PWM Fault                    */
    &PWMGen0_IRQHandler,      /* PWM Generator 0              */
    &PWMGen1_IRQHandler,      /* PWM Generator 1              */
    &PWMGen2_IRQHandler,      /* PWM Generator 2              */
    &QEI0_IRQHandler,         /* Quadrature Encoder 0         */
    &ADCSeq0_IRQHandler,      /* ADC Sequence 0               */
    &ADCSeq1_IRQHandler,      /* ADC Sequence 1               */
    &ADCSeq2_IRQHandler,      /* ADC Sequence 2               */
    &ADCSeq3_IRQHandler,      /* ADC Sequence 3               */
    &Watchdog_IRQHandler,     /* Watchdog timer               */
    &Timer0A_IRQHandler,      /* Timer 0 subtimer A           */
    &Timer0B_IRQHandler,      /* Timer 0 subtimer B           */
    &Timer1A_IRQHandler,      /* Timer 1 subtimer A           */
    &Timer1B_IRQHandler,      /* Timer 1 subtimer B           */
    &Timer2A_IRQHandler,      /* Timer 2 subtimer A           */
    &Timer2B_IRQHandler,      /* Timer 2 subtimer B           */
    &Comp0_IRQHandler,        /* Analog Comparator 0          */
    &Comp1_IRQHandler,        /* Analog Comparator 1          */
    &Comp2_IRQHandler,        /* Analog Comparator 2          */
    &SysCtrl_IRQHandler,      /* System Control (PLL,OSC,BO)  */
    &FlashCtrl_IRQHandler,    /* FLASH Control                */
    &GPIOPortF_IRQHandler,    /* GPIO Port F                  */
    &GPIOPortG_IRQHandler,    /* GPIO Port G                  */
    &GPIOPortH_IRQHandler,    /* GPIO Port H                  */
    &UART2_IRQHandler,        /* UART2 Rx and Tx              */
    &SSI1_IRQHandler ,        /* SSI1 Rx and Tx               */
    &Timer3A_IRQHandler,      /* Timer 3 subtimer A           */
    &Timer3B_IRQHandler,      /* Timer 3 subtimer B           */
    &I2C1_IRQHandler,         /* I2C1 Master and Slave        */
    &QEI1_IRQHandler,         /* Quadrature Encoder 1         */
    &CAN0_IRQHandler,         /* CAN0                         */
    &CAN1_IRQHandler,         /* CAN1                         */
    &CAN2_IRQHandler,         /* CAN2                         */
    &Spurious_Handler,        /* Reserved                     */
    &Hibernate_IRQHandler,    /* Hibernate                    */
    &USB0_IRQHandler,         /* USB0                         */
    &PWMGen3_IRQHandler,      /* PWM Generator 3              */
    &uDMAST_IRQHandler,       /* uDMA Software Transfer       */
    &uDMAError_IRQHandler,    /* uDMA Error                   */
    &ADC1Seq0_IRQHandler,     /* ADC1 Sequence 0              */
    &ADC1Seq1_IRQHandler,     /* ADC1 Sequence 1              */
    &ADC1Seq2_IRQHandler,     /* ADC1 Sequence 2              */
    &ADC1Seq3_IRQHandler,     /* ADC1 Sequence 3              */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &GPIOPortJ_IRQHandler,    /* GPIO Port J                  */
    &GPIOPortK_IRQHandler,    /* GPIO Port K                  */
    &GPIOPortL_IRQHandler,    /* GPIO Port L                  */
    &SSI2_IRQHandler,         /* SSI2 Rx and Tx               */
    &SSI3_IRQHandler,         /* SSI3 Rx and Tx               */
    &UART3_IRQHandler,        /* UART3 Rx and Tx              */
    &UART4_IRQHandler,        /* UART4 Rx and Tx              */
    &UART5_IRQHandler,        /* UART5 Rx and Tx              */
    &UART6_IRQHandler,        /* UART6 Rx and Tx              */
    &UART7_IRQHandler,        /* UART7 Rx and Tx              */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &I2C2_IRQHandler,         /* I2C2 Master and Slave        */
    &I2C3_IRQHandler,         /* I2C3 Master and Slave        */
    &Timer4A_IRQHandler,      /* Timer 4 subtimer A           */
    &Timer4B_IRQHandler,      /* Timer 4 subtimer B           */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &Timer5A_IRQHandler,      /* Timer 5 subtimer A           */
    &Timer5B_IRQHandler,      /* Timer 5 subtimer B           */
    &WideTimer0A_IRQHandler,  /* Wide Timer 0 subtimer A      */
    &WideTimer0B_IRQHandler,  /* Wide Timer 0 subtimer B      */
    &WideTimer1A_IRQHandler,  /* Wide Timer 1 subtimer A      */
    &WideTimer1B_IRQHandler,  /* Wide Timer 1 subtimer B      */
    &WideTimer2A_IRQHandler,  /* Wide Timer 2 subtimer A      */
    &WideTimer2B_IRQHandler,  /* Wide Timer 2 subtimer B      */
    &WideTimer3A_IRQHandler,  /* Wide Timer 3 subtimer A      */
    &WideTimer3B_IRQHandler,  /* Wide Timer 3 subtimer B      */
    &WideTimer4A_IRQHandler,  /* Wide Timer 4 subtimer A      */
    &WideTimer4B_IRQHandler,  /* Wide Timer 4 subtimer B      */
    &WideTimer5A_IRQHandler,  /* Wide Timer 5 subtimer A      */
    &WideTimer5B_IRQHandler,  /* Wide Timer 5 subtimer B      */
    &FPU_IRQHandler,          /* FPU                          */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &I2C4_IRQHandler,         /* I2C4 Master and Slave        */
    &I2C5_IRQHandler,         /* I2C5 Master and Slave        */
    &GPIOPortM_IRQHandler,    /* GPIO Port M                  */
    &GPIOPortN_IRQHandler,    /* GPIO Port N                  */
    &QEI2_IRQHandler,         /* Quadrature Encoder 2         */
    &Spurious_Handler,        /* Reserved                     */
    &Spurious_Handler,        /* Reserved                     */
    &GPIOPortP0_IRQHandler,   /* GPIO Port P (Summary or P0)  */
    &GPIOPortP1_IRQHandler,   /* GPIO Port P1                 */
    &GPIOPortP2_IRQHandler,   /* GPIO Port P2                 */
    &GPIOPortP3_IRQHandler,   /* GPIO Port P3                 */
    &GPIOPortP4_IRQHandler,   /* GPIO Port P4                 */
    &GPIOPortP5_IRQHandler,   /* GPIO Port P5                 */
    &GPIOPortP6_IRQHandler,   /* GPIO Port P6                 */
    &GPIOPortP7_IRQHandler,   /* GPIO Port P7                 */
    &GPIOPortQ0_IRQHandler,   /* GPIO Port Q (Summary or Q0)  */
    &GPIOPortQ1_IRQHandler,   /* GPIO Port Q1                 */
    &GPIOPortQ2_IRQHandler,   /* GPIO Port Q2                 */
    &GPIOPortQ3_IRQHandler,   /* GPIO Port Q3                 */
    &GPIOPortQ4_IRQHandler,   /* GPIO Port Q4                 */
    &GPIOPortQ5_IRQHandler,   /* GPIO Port Q5                 */
    &GPIOPortQ6_IRQHandler,   /* GPIO Port Q6                 */
    &GPIOPortQ7_IRQHandler,   /* GPIO Port Q7                 */
    &GPIOPortR_IRQHandler,    /* GPIO Port R                  */
    &GPIOPortS_IRQHandler,    /* GPIO Port S                  */
    &PWM1Gen0_IRQHandler,     /* PWM 1 Generator 0            */
    &PWM1Gen1_IRQHandler,     /* PWM 1 Generator 1            */
    &PWM1Gen2_IRQHandler,     /* PWM 1 Generator 2            */
    &PWM1Gen3_IRQHandler,     /* PWM 1 Generator 3            */
    &PWM1Fault_IRQHandler     /* PWM 1 Fault                  */
};

/* exception handlers ------------------------------------------------------*/
/*..........................................................................*/
__stackless void NMI_Handler(void) {
    assert_failed("NMI", __LINE__); /* should never return! */
}
/*..........................................................................*/
__stackless void MemManage_Handler(void) {
    assert_failed("MemManage", __LINE__); /* should never return! */
}
/*..........................................................................*/
__stackless void HardFault_Handler(void) {
    unsigned old_sp = __get_SP();

    if (old_sp < (unsigned)&CSTACK$$Base) { /* stack overflow? */
        __set_SP((unsigned)&CSTACK$$Limit); /* initial stack pointer */
        assert_failed("StackOverflow", old_sp); /* should never return! */
    }
    else {
        assert_failed("HardFault", __LINE__); /* should never return! */
    }
}
/*..........................................................................*/
__stackless void BusFault_Handler(void) {
    assert_failed("BusFault", __LINE__); /* should never return! */
}
/*..........................................................................*/
__stackless void UsageFault_Handler(void) {
    assert_failed("UsageFault", __LINE__); /* should never return! */
}
/*..........................................................................*/
__stackless void Spurious_Handler(void) {
    assert_failed("Spurious", __LINE__); /* should never return! */
}

/*----------------------------------------------------------------------------
* Weak aliases for each Exception handler to the Spurious_Handler.
*/
#pragma weak SVC_Handler            = Spurious_Handler
#pragma weak DebugMon_Handler       = Spurious_Handler
#pragma weak PendSV_Handler         = Spurious_Handler
#pragma weak SysTick_Handler        = Spurious_Handler
#pragma weak GPIOPortA_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortB_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortC_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortD_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortE_IRQHandler   = Spurious_Handler
#pragma weak UART0_IRQHandler       = Spurious_Handler
#pragma weak UART1_IRQHandler       = Spurious_Handler
#pragma weak SSI0_IRQHandler        = Spurious_Handler
#pragma weak I2C0_IRQHandler        = Spurious_Handler
#pragma weak PWMFault_IRQHandler    = Spurious_Handler
#pragma weak PWMGen0_IRQHandler     = Spurious_Handler
#pragma weak PWMGen1_IRQHandler     = Spurious_Handler
#pragma weak PWMGen2_IRQHandler     = Spurious_Handler
#pragma weak QEI0_IRQHandler        = Spurious_Handler
#pragma weak ADCSeq0_IRQHandler     = Spurious_Handler
#pragma weak ADCSeq1_IRQHandler     = Spurious_Handler
#pragma weak ADCSeq2_IRQHandler     = Spurious_Handler
#pragma weak ADCSeq3_IRQHandler     = Spurious_Handler
#pragma weak Watchdog_IRQHandler    = Spurious_Handler
#pragma weak Timer0A_IRQHandler     = Spurious_Handler
#pragma weak Timer0B_IRQHandler     = Spurious_Handler
#pragma weak Timer1A_IRQHandler     = Spurious_Handler
#pragma weak Timer1B_IRQHandler     = Spurious_Handler
#pragma weak Timer2A_IRQHandler     = Spurious_Handler
#pragma weak Timer2B_IRQHandler     = Spurious_Handler
#pragma weak Comp0_IRQHandler       = Spurious_Handler
#pragma weak Comp1_IRQHandler       = Spurious_Handler
#pragma weak Comp2_IRQHandler       = Spurious_Handler
#pragma weak SysCtrl_IRQHandler     = Spurious_Handler
#pragma weak FlashCtrl_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortF_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortG_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortH_IRQHandler   = Spurious_Handler
#pragma weak UART2_IRQHandler       = Spurious_Handler
#pragma weak SSI1_IRQHandler        = Spurious_Handler
#pragma weak Timer3A_IRQHandler     = Spurious_Handler
#pragma weak Timer3B_IRQHandler     = Spurious_Handler
#pragma weak I2C1_IRQHandler        = Spurious_Handler
#pragma weak QEI1_IRQHandler        = Spurious_Handler
#pragma weak CAN0_IRQHandler        = Spurious_Handler
#pragma weak CAN1_IRQHandler        = Spurious_Handler
#pragma weak CAN2_IRQHandler        = Spurious_Handler
#pragma weak Hibernate_IRQHandler   = Spurious_Handler
#pragma weak USB0_IRQHandler        = Spurious_Handler
#pragma weak PWMGen3_IRQHandler     = Spurious_Handler
#pragma weak uDMAST_IRQHandler      = Spurious_Handler
#pragma weak uDMAError_IRQHandler   = Spurious_Handler
#pragma weak ADC1Seq0_IRQHandler    = Spurious_Handler
#pragma weak ADC1Seq1_IRQHandler    = Spurious_Handler
#pragma weak ADC1Seq2_IRQHandler    = Spurious_Handler
#pragma weak ADC1Seq3_IRQHandler    = Spurious_Handler
#pragma weak I2S0_IRQHandler        = Spurious_Handler
#pragma weak EBI0_IRQHandler        = Spurious_Handler
#pragma weak GPIOPortJ_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortK_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortL_IRQHandler   = Spurious_Handler
#pragma weak SSI2_IRQHandler        = Spurious_Handler
#pragma weak SSI3_IRQHandler        = Spurious_Handler
#pragma weak UART3_IRQHandler       = Spurious_Handler
#pragma weak UART4_IRQHandler       = Spurious_Handler
#pragma weak UART5_IRQHandler       = Spurious_Handler
#pragma weak UART6_IRQHandler       = Spurious_Handler
#pragma weak UART7_IRQHandler       = Spurious_Handler
#pragma weak I2C2_IRQHandler        = Spurious_Handler
#pragma weak I2C3_IRQHandler        = Spurious_Handler
#pragma weak Timer4A_IRQHandler     = Spurious_Handler
#pragma weak Timer4B_IRQHandler     = Spurious_Handler
#pragma weak Timer5A_IRQHandler     = Spurious_Handler
#pragma weak Timer5B_IRQHandler     = Spurious_Handler
#pragma weak WideTimer0A_IRQHandler = Spurious_Handler
#pragma weak WideTimer0B_IRQHandler = Spurious_Handler
#pragma weak WideTimer1A_IRQHandler = Spurious_Handler
#pragma weak WideTimer1B_IRQHandler = Spurious_Handler
#pragma weak WideTimer2A_IRQHandler = Spurious_Handler
#pragma weak WideTimer2B_IRQHandler = Spurious_Handler
#pragma weak WideTimer3A_IRQHandler = Spurious_Handler
#pragma weak WideTimer3B_IRQHandler = Spurious_Handler
#pragma weak WideTimer4A_IRQHandler = Spurious_Handler
#pragma weak WideTimer4B_IRQHandler = Spurious_Handler
#pragma weak WideTimer5A_IRQHandler = Spurious_Handler
#pragma weak WideTimer5B_IRQHandler = Spurious_Handler
#pragma weak FPU_IRQHandler         = Spurious_Handler
#pragma weak PECI0_IRQHandler       = Spurious_Handler
#pragma weak LPC0_IRQHandler        = Spurious_Handler
#pragma weak I2C4_IRQHandler        = Spurious_Handler
#pragma weak I2C5_IRQHandler        = Spurious_Handler
#pragma weak GPIOPortM_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortN_IRQHandler   = Spurious_Handler
#pragma weak QEI2_IRQHandler        = Spurious_Handler
#pragma weak Fan0_IRQHandler        = Spurious_Handler
#pragma weak GPIOPortP0_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP1_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP2_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP3_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP4_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP5_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP6_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortP7_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ0_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ1_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ2_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ3_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ4_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ5_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ6_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortQ7_IRQHandler  = Spurious_Handler
#pragma weak GPIOPortR_IRQHandler   = Spurious_Handler
#pragma weak GPIOPortS_IRQHandler   = Spurious_Handler
#pragma weak PWM1Gen0_IRQHandler    = Spurious_Handler
#pragma weak PWM1Gen1_IRQHandler    = Spurious_Handler
#pragma weak PWM1Gen2_IRQHandler    = Spurious_Handler
#pragma weak PWM1Gen3_IRQHandler    = Spurious_Handler
#pragma weak PWM1Fault_IRQHandler   = Spurious_Handler


