/* startup code for TM4C MCU */
#include "tm4c_cmsis.h"

extern int CSTACK$$Limit;
void __iar_program_start(void);
void Unused_Handler(void);


int const __vector_table[] @ ".intvec" = {
    (int)&CSTACK$$Limit,
    (int)&__iar_program_start,
    (int)&NMI_Handler,
    (int)&HardFault_Handler,
    (int)&MemManage_Handler,
    (int)&BusFault_Handler,
    (int)&UsageFault_Handler,
    0,  /* Reserved */
    0,  /* Reserved */
    0,  /* Reserved */
    0,  /* Reserved */
    (int)&SVC_Handler,
    (int)&DebugMon_Handler,
    0,  /* Reserved */
    (int)&PendSV_Handler,
    (int)&SysTick_Handler,

    /* external interrupts (IRQs) ... */
    (int)&GPIOPortA_IRQHandler,    /* GPIO Port A                  */
    (int)&GPIOPortB_IRQHandler,    /* GPIO Port B                  */
    (int)&GPIOPortC_IRQHandler,    /* GPIO Port C                  */
    (int)&GPIOPortD_IRQHandler,    /* GPIO Port D                  */
    (int)&GPIOPortE_IRQHandler,    /* GPIO Port E                  */
    (int)&UART0_IRQHandler,        /* UART0 Rx and Tx              */
    (int)&UART1_IRQHandler,        /* UART1 Rx and Tx              */
    (int)&SSI0_IRQHandler,         /* SSI0 Rx and Tx               */
    (int)&I2C0_IRQHandler,         /* I2C0 Master and Slave        */
    (int)&PWMFault_IRQHandler,     /* PWM Fault                    */
    (int)&PWMGen0_IRQHandler,      /* PWM Generator 0              */
    (int)&PWMGen1_IRQHandler,      /* PWM Generator 1              */
    (int)&PWMGen2_IRQHandler,      /* PWM Generator 2              */
    (int)&QEI0_IRQHandler,         /* Quadrature Encoder 0         */
    (int)&ADCSeq0_IRQHandler,      /* ADC Sequence 0               */
    (int)&ADCSeq1_IRQHandler,      /* ADC Sequence 1               */
    (int)&ADCSeq2_IRQHandler,      /* ADC Sequence 2               */
    (int)&ADCSeq3_IRQHandler,      /* ADC Sequence 3               */
    (int)&Watchdog_IRQHandler,     /* Watchdog timer               */
    (int)&Timer0A_IRQHandler,      /* Timer 0 subtimer A           */
    (int)&Timer0B_IRQHandler,      /* Timer 0 subtimer B           */
    (int)&Timer1A_IRQHandler,      /* Timer 1 subtimer A           */
    (int)&Timer1B_IRQHandler,      /* Timer 1 subtimer B           */
    (int)&Timer2A_IRQHandler,      /* Timer 2 subtimer A           */
    (int)&Timer2B_IRQHandler,      /* Timer 2 subtimer B           */
    (int)&Comp0_IRQHandler,        /* Analog Comparator 0          */
    (int)&Comp1_IRQHandler,        /* Analog Comparator 1          */
    (int)&Comp2_IRQHandler,        /* Analog Comparator 2          */
    (int)&SysCtrl_IRQHandler,      /* System Control (PLL,OSC,BO)  */
    (int)&FlashCtrl_IRQHandler,    /* FLASH Control                */
    (int)&GPIOPortF_IRQHandler,    /* GPIO Port F                  */
    (int)&GPIOPortG_IRQHandler,    /* GPIO Port G                  */
    (int)&GPIOPortH_IRQHandler,    /* GPIO Port H                  */
    (int)&UART2_IRQHandler,        /* UART2 Rx and Tx              */
    (int)&SSI1_IRQHandler ,        /* SSI1 Rx and Tx               */
    (int)&Timer3A_IRQHandler,      /* Timer 3 subtimer A           */
    (int)&Timer3B_IRQHandler,      /* Timer 3 subtimer B           */
    (int)&I2C1_IRQHandler,         /* I2C1 Master and Slave        */
    (int)&QEI1_IRQHandler,         /* Quadrature Encoder 1         */
    (int)&CAN0_IRQHandler,         /* CAN0                         */
    (int)&CAN1_IRQHandler,         /* CAN1                         */
    (int)&CAN2_IRQHandler,         /* CAN2                         */
    0,                             /* Reserved                     */
    (int)&Hibernate_IRQHandler,    /* Hibernate                    */
    (int)&USB0_IRQHandler,         /* USB0                         */
    (int)&PWMGen3_IRQHandler,      /* PWM Generator 3              */
    (int)&uDMAST_IRQHandler,       /* uDMA Software Transfer       */
    (int)&uDMAError_IRQHandler,    /* uDMA Error                   */
    (int)&ADC1Seq0_IRQHandler,     /* ADC1 Sequence 0              */
    (int)&ADC1Seq1_IRQHandler,     /* ADC1 Sequence 1              */
    (int)&ADC1Seq2_IRQHandler,     /* ADC1 Sequence 2              */
    (int)&ADC1Seq3_IRQHandler,     /* ADC1 Sequence 3              */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    (int)&GPIOPortJ_IRQHandler,    /* GPIO Port J                  */
    (int)&GPIOPortK_IRQHandler,    /* GPIO Port K                  */
    (int)&GPIOPortL_IRQHandler,    /* GPIO Port L                  */
    (int)&SSI2_IRQHandler,         /* SSI2 Rx and Tx               */
    (int)&SSI3_IRQHandler,         /* SSI3 Rx and Tx               */
    (int)&UART3_IRQHandler,        /* UART3 Rx and Tx              */
    (int)&UART4_IRQHandler,        /* UART4 Rx and Tx              */
    (int)&UART5_IRQHandler,        /* UART5 Rx and Tx              */
    (int)&UART6_IRQHandler,        /* UART6 Rx and Tx              */
    (int)&UART7_IRQHandler,        /* UART7 Rx and Tx              */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    (int)&I2C2_IRQHandler,         /* I2C2 Master and Slave        */
    (int)&I2C3_IRQHandler,         /* I2C3 Master and Slave        */
    (int)&Timer4A_IRQHandler,      /* Timer 4 subtimer A           */
    (int)&Timer4B_IRQHandler,      /* Timer 4 subtimer B           */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    (int)&Timer5A_IRQHandler,      /* Timer 5 subtimer A           */
    (int)&Timer5B_IRQHandler,      /* Timer 5 subtimer B           */
    (int)&WideTimer0A_IRQHandler,  /* Wide Timer 0 subtimer A      */
    (int)&WideTimer0B_IRQHandler,  /* Wide Timer 0 subtimer B      */
    (int)&WideTimer1A_IRQHandler,  /* Wide Timer 1 subtimer A      */
    (int)&WideTimer1B_IRQHandler,  /* Wide Timer 1 subtimer B      */
    (int)&WideTimer2A_IRQHandler,  /* Wide Timer 2 subtimer A      */
    (int)&WideTimer2B_IRQHandler,  /* Wide Timer 2 subtimer B      */
    (int)&WideTimer3A_IRQHandler,  /* Wide Timer 3 subtimer A      */
    (int)&WideTimer3B_IRQHandler,  /* Wide Timer 3 subtimer B      */
    (int)&WideTimer4A_IRQHandler,  /* Wide Timer 4 subtimer A      */
    (int)&WideTimer4B_IRQHandler,  /* Wide Timer 4 subtimer B      */
    (int)&WideTimer5A_IRQHandler,  /* Wide Timer 5 subtimer A      */
    (int)&WideTimer5B_IRQHandler,  /* Wide Timer 5 subtimer B      */
    (int)&FPU_IRQHandler,          /* FPU                          */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    (int)&I2C4_IRQHandler,         /* I2C4 Master and Slave        */
    (int)&I2C5_IRQHandler,         /* I2C5 Master and Slave        */
    (int)&GPIOPortM_IRQHandler,    /* GPIO Port M                  */
    (int)&GPIOPortN_IRQHandler,    /* GPIO Port N                  */
    (int)&QEI2_IRQHandler,         /* Quadrature Encoder 2         */
    0,                             /* Reserved                     */
    0,                             /* Reserved                     */
    (int)&GPIOPortP0_IRQHandler,   /* GPIO Port P (Summary or P0)  */
    (int)&GPIOPortP1_IRQHandler,   /* GPIO Port P1                 */
    (int)&GPIOPortP2_IRQHandler,   /* GPIO Port P2                 */
    (int)&GPIOPortP3_IRQHandler,   /* GPIO Port P3                 */
    (int)&GPIOPortP4_IRQHandler,   /* GPIO Port P4                 */
    (int)&GPIOPortP5_IRQHandler,   /* GPIO Port P5                 */
    (int)&GPIOPortP6_IRQHandler,   /* GPIO Port P6                 */
    (int)&GPIOPortP7_IRQHandler,   /* GPIO Port P7                 */
    (int)&GPIOPortQ0_IRQHandler,   /* GPIO Port Q (Summary or Q0)  */
    (int)&GPIOPortQ1_IRQHandler,   /* GPIO Port Q1                 */
    (int)&GPIOPortQ2_IRQHandler,   /* GPIO Port Q2                 */
    (int)&GPIOPortQ3_IRQHandler,   /* GPIO Port Q3                 */
    (int)&GPIOPortQ4_IRQHandler,   /* GPIO Port Q4                 */
    (int)&GPIOPortQ5_IRQHandler,   /* GPIO Port Q5                 */
    (int)&GPIOPortQ6_IRQHandler,   /* GPIO Port Q6                 */
    (int)&GPIOPortQ7_IRQHandler,   /* GPIO Port Q7                 */
    (int)&GPIOPortR_IRQHandler,    /* GPIO Port R                  */
    (int)&GPIOPortS_IRQHandler,    /* GPIO Port S                  */
    (int)&PWM1Gen0_IRQHandler,     /* PWM 1 Generator 0            */
    (int)&PWM1Gen1_IRQHandler,     /* PWM 1 Generator 1            */
    (int)&PWM1Gen2_IRQHandler,     /* PWM 1 Generator 2            */
    (int)&PWM1Gen3_IRQHandler,     /* PWM 1 Generator 3            */
    (int)&PWM1Fault_IRQHandler     /* PWM 1 Fault                  */

};

__stackless void HardFault_Handler(void) {
    assert_failed("HardFault", __LINE__);
}

__stackless void NMI_Handler(void) {
    assert_failed("NMI", __LINE__);
}

__stackless void MemManage_Handler(void) {
    assert_failed("MemManage", __LINE__);
}

__stackless void BusFault_Handler(void) {
    assert_failed("BusFault", __LINE__);
}

__stackless void UsageFault_Handler(void) {
    assert_failed("UsageFault", __LINE__);
}

__stackless void Unused_Handler(void) {
    assert_failed("Unused", __LINE__);
}

#pragma weak SVC_Handler       = Unused_Handler
#pragma weak DebugMon_Handler  = Unused_Handler
#pragma weak PendSV_Handler    = Unused_Handler
#pragma weak SysTick_Handler   = Unused_Handler

#pragma weak GPIOPortA_IRQHandler   = Unused_Handler
#pragma weak GPIOPortB_IRQHandler   = Unused_Handler
#pragma weak GPIOPortC_IRQHandler   = Unused_Handler
#pragma weak GPIOPortD_IRQHandler   = Unused_Handler
#pragma weak GPIOPortE_IRQHandler   = Unused_Handler
#pragma weak UART0_IRQHandler       = Unused_Handler
#pragma weak UART1_IRQHandler       = Unused_Handler
#pragma weak SSI0_IRQHandler        = Unused_Handler
#pragma weak I2C0_IRQHandler        = Unused_Handler
#pragma weak PWMFault_IRQHandler    = Unused_Handler
#pragma weak PWMGen0_IRQHandler     = Unused_Handler
#pragma weak PWMGen1_IRQHandler     = Unused_Handler
#pragma weak PWMGen2_IRQHandler     = Unused_Handler
#pragma weak QEI0_IRQHandler        = Unused_Handler
#pragma weak ADCSeq0_IRQHandler     = Unused_Handler
#pragma weak ADCSeq1_IRQHandler     = Unused_Handler
#pragma weak ADCSeq2_IRQHandler     = Unused_Handler
#pragma weak ADCSeq3_IRQHandler     = Unused_Handler
#pragma weak Watchdog_IRQHandler    = Unused_Handler
#pragma weak Timer0A_IRQHandler     = Unused_Handler
#pragma weak Timer0B_IRQHandler     = Unused_Handler
#pragma weak Timer1A_IRQHandler     = Unused_Handler
#pragma weak Timer1B_IRQHandler     = Unused_Handler
#pragma weak Timer2A_IRQHandler     = Unused_Handler
#pragma weak Timer2B_IRQHandler     = Unused_Handler
#pragma weak Comp0_IRQHandler       = Unused_Handler
#pragma weak Comp1_IRQHandler       = Unused_Handler
#pragma weak Comp2_IRQHandler       = Unused_Handler
#pragma weak SysCtrl_IRQHandler     = Unused_Handler
#pragma weak FlashCtrl_IRQHandler   = Unused_Handler
#pragma weak GPIOPortF_IRQHandler   = Unused_Handler
#pragma weak GPIOPortG_IRQHandler   = Unused_Handler
#pragma weak GPIOPortH_IRQHandler   = Unused_Handler
#pragma weak UART2_IRQHandler       = Unused_Handler
#pragma weak SSI1_IRQHandler        = Unused_Handler
#pragma weak Timer3A_IRQHandler     = Unused_Handler
#pragma weak Timer3B_IRQHandler     = Unused_Handler
#pragma weak I2C1_IRQHandler        = Unused_Handler
#pragma weak QEI1_IRQHandler        = Unused_Handler
#pragma weak CAN0_IRQHandler        = Unused_Handler
#pragma weak CAN1_IRQHandler        = Unused_Handler
#pragma weak CAN2_IRQHandler        = Unused_Handler
#pragma weak Hibernate_IRQHandler   = Unused_Handler
#pragma weak USB0_IRQHandler        = Unused_Handler
#pragma weak PWMGen3_IRQHandler     = Unused_Handler
#pragma weak uDMAST_IRQHandler      = Unused_Handler
#pragma weak uDMAError_IRQHandler   = Unused_Handler
#pragma weak ADC1Seq0_IRQHandler    = Unused_Handler
#pragma weak ADC1Seq1_IRQHandler    = Unused_Handler
#pragma weak ADC1Seq2_IRQHandler    = Unused_Handler
#pragma weak ADC1Seq3_IRQHandler    = Unused_Handler
#pragma weak I2S0_IRQHandler        = Unused_Handler
#pragma weak EBI0_IRQHandler        = Unused_Handler
#pragma weak GPIOPortJ_IRQHandler   = Unused_Handler
#pragma weak GPIOPortK_IRQHandler   = Unused_Handler
#pragma weak GPIOPortL_IRQHandler   = Unused_Handler
#pragma weak SSI2_IRQHandler        = Unused_Handler
#pragma weak SSI3_IRQHandler        = Unused_Handler
#pragma weak UART3_IRQHandler       = Unused_Handler
#pragma weak UART4_IRQHandler       = Unused_Handler
#pragma weak UART5_IRQHandler       = Unused_Handler
#pragma weak UART6_IRQHandler       = Unused_Handler
#pragma weak UART7_IRQHandler       = Unused_Handler
#pragma weak I2C2_IRQHandler        = Unused_Handler
#pragma weak I2C3_IRQHandler        = Unused_Handler
#pragma weak Timer4A_IRQHandler     = Unused_Handler
#pragma weak Timer4B_IRQHandler     = Unused_Handler
#pragma weak Timer5A_IRQHandler     = Unused_Handler
#pragma weak Timer5B_IRQHandler     = Unused_Handler
#pragma weak WideTimer0A_IRQHandler = Unused_Handler
#pragma weak WideTimer0B_IRQHandler = Unused_Handler
#pragma weak WideTimer1A_IRQHandler = Unused_Handler
#pragma weak WideTimer1B_IRQHandler = Unused_Handler
#pragma weak WideTimer2A_IRQHandler = Unused_Handler
#pragma weak WideTimer2B_IRQHandler = Unused_Handler
#pragma weak WideTimer3A_IRQHandler = Unused_Handler
#pragma weak WideTimer3B_IRQHandler = Unused_Handler
#pragma weak WideTimer4A_IRQHandler = Unused_Handler
#pragma weak WideTimer4B_IRQHandler = Unused_Handler
#pragma weak WideTimer5A_IRQHandler = Unused_Handler
#pragma weak WideTimer5B_IRQHandler = Unused_Handler
#pragma weak FPU_IRQHandler         = Unused_Handler
#pragma weak PECI0_IRQHandler       = Unused_Handler
#pragma weak LPC0_IRQHandler        = Unused_Handler
#pragma weak I2C4_IRQHandler        = Unused_Handler
#pragma weak I2C5_IRQHandler        = Unused_Handler
#pragma weak GPIOPortM_IRQHandler   = Unused_Handler
#pragma weak GPIOPortN_IRQHandler   = Unused_Handler
#pragma weak QEI2_IRQHandler        = Unused_Handler
#pragma weak Fan0_IRQHandler        = Unused_Handler
#pragma weak GPIOPortP0_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP1_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP2_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP3_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP4_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP5_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP6_IRQHandler  = Unused_Handler
#pragma weak GPIOPortP7_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ0_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ1_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ2_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ3_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ4_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ5_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ6_IRQHandler  = Unused_Handler
#pragma weak GPIOPortQ7_IRQHandler  = Unused_Handler
#pragma weak GPIOPortR_IRQHandler   = Unused_Handler
#pragma weak GPIOPortS_IRQHandler   = Unused_Handler
#pragma weak PWM1Gen0_IRQHandler    = Unused_Handler
#pragma weak PWM1Gen1_IRQHandler    = Unused_Handler
#pragma weak PWM1Gen2_IRQHandler    = Unused_Handler
#pragma weak PWM1Gen3_IRQHandler    = Unused_Handler
#pragma weak PWM1Fault_IRQHandler   = Unused_Handler

