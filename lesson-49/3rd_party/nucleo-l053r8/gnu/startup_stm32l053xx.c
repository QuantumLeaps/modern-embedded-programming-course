/* File: startup_stm32l053xx.c for GNU-ARM
 * Purpose: startup file for STM32L053xx Cortex-M0+ device.
 *          Should be used with GCC 'GNU Tools ARM Embedded'
 * Version: CMSIS 5.0.1
 * Date: 2017-09-13
 *
 * Modified by Quantum Leaps:
 * - Added relocating of the Vector Table to free up the 256B region at 0x0
 *   for NULL-pointer protection by the MPU.
 * - Modified all exception handlers to branch to assert_failed()
 *   instead of locking up the CPU inside an endless loop.
 *
 * Created from the CMSIS template for the specified device
 * Quantum Leaps, www.state-machine.com
 *
 * NOTE:
 * The function assert_failed defined at the end of this file defines
 * the error/assertion handling policy for the application and might
 * need to be customized for each project. This function is defined in
 * assembly to re-set the stack pointer, in case it is corrupted by the
 * time assert_failed is called.
 */

/* start and end of stack defined in the linker script ---------------------*/
/*extern int __stack_start__;*/
extern int __stack_end__;

/* Weak prototypes for error handlers --------------------------------------*/
/**
* \note
* The function assert_failed defined at the end of this file defines
* the error/assertion handling policy for the application and might
* need to be customized for each project. This function is defined in
* assembly to avoid accessing the stack, which might be corrupted by
* the time assert_failed is called.
*/
__attribute__ ((naked, noreturn))
void assert_failed(char const *module, int loc);

/* Function prototypes -----------------------------------------------------*/
void Default_Handler(void);  /* Default empty handler */
void Reset_Handler(void);    /* Reset Handler */
void SystemInit(void);       /* CMSIS system initialization */

/*----------------------------------------------------------------------------
* weak aliases for each Exception handler to the Default_Handler.
* Any function with the same name will override these definitions.
*/
/* Cortex-M Processor fault exceptions... */
void NMI_Handler           (void) __attribute__ ((weak));
void HardFault_Handler     (void) __attribute__ ((weak));
void MemManage_Handler     (void) __attribute__ ((weak));
void BusFault_Handler      (void) __attribute__ ((weak));
void UsageFault_Handler    (void) __attribute__ ((weak));

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));

/* external interrupts...   */
void WWDG_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PVD_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void FLASH_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void RCC_CRS_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_15_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void TSC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Channel2_3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Channel4_5_6_7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1_COMP_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM21_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM22_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void USART2_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void RNG_LPUART1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LCD_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void USB_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved14_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved16_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved18_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved19_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved21_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));

/*..........................................................................*/
__attribute__ ((section(".isr_vector")))
int const g_pfnVectors[] = {
    (int)&__stack_end__,          /* Top of Stack                   */
    (int)&Reset_Handler,          /* Reset Handler                  */
    (int)&NMI_Handler,            /* NMI Handler                    */
    (int)&HardFault_Handler,      /* Hard Fault Handler             */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&SVC_Handler,            /* SVCall handler                 */
    (int)&DebugMon_Handler,       /* Debug monitor handler          */
    (int)&Default_Handler,        /* Reserved                       */
    (int)&PendSV_Handler,         /* The PendSV handler             */
    (int)&SysTick_Handler,        /* The SysTick handler            */

    /*IRQ handlers... */
    (int)&WWDG_IRQHandler,        /* [ 0] Window Watchdog                */
    (int)&PVD_IRQHandler,         /* [ 1] PVD through EXTI Line detect   */
    (int)&RTC_IRQHandler,         /* [ 2] RTC through EXTI Line          */
    (int)&FLASH_IRQHandler,       /* [ 3] FLASH                          */
    (int)&RCC_CRS_IRQHandler,     /* [ 4] RCC and CRS                    */
    (int)&EXTI0_1_IRQHandler,     /* [ 5] EXTI Line 0 and 1              */
    (int)&EXTI2_3_IRQHandler,     /* [ 6] EXTI Line 2 and 3              */
    (int)&EXTI4_15_IRQHandler,    /* [ 7] EXTI Line 4 to 15              */
    (int)&TSC_IRQHandler,         /* [ 8] TSC                            */
    (int)&DMA1_Channel1_IRQHandler,  /* [ 9] DMA1 Channel 1              */
    (int)&DMA1_Channel2_3_IRQHandler,/* [10] DMA1 Channel 2 and Channel 3 */
    (int)&DMA1_Channel4_5_6_7_IRQHandler, /* [11] DMA1 Channel 4, 5, 6 and 7   */
    (int)&ADC1_COMP_IRQHandler,   /* [12] ADC1, COMP1 and COMP2          */
    (int)&LPTIM1_IRQHandler,      /* [13] LPTIM1                         */
    (int)&Reserved14_IRQHandler,  /* [14] Reserved                       */
    (int)&TIM2_IRQHandler,        /* [15] TIM2                           */
    (int)&Reserved16_IRQHandler,  /* [16] Reserved                       */
    (int)&TIM6_DAC_IRQHandler,    /* [17] TIM6 and DAC                   */
    (int)&Reserved18_IRQHandler,  /* [18] Reserved                       */
    (int)&Reserved19_IRQHandler,  /* [19] Reserved                       */
    (int)&TIM21_IRQHandler,       /* [20] TIM21                          */
    (int)&Reserved21_IRQHandler,  /* [21] Reserved                       */
    (int)&TIM22_IRQHandler,       /* [22] TIM22                          */
    (int)&I2C1_IRQHandler,        /* [23] I2C1                           */
    (int)&I2C2_IRQHandler,        /* [24] I2C2                           */
    (int)&SPI1_IRQHandler,        /* [25] SPI1                           */
    (int)&SPI2_IRQHandler,        /* [26] SPI2                           */
    (int)&USART1_IRQHandler,      /* [27] USART1                         */
    (int)&USART2_IRQHandler,      /* [28] USART2                         */
    (int)&RNG_LPUART1_IRQHandler, /* [29] RNG and LPUART1                */
    (int)&LCD_IRQHandler,         /* [30] LCD                            */
    (int)&USB_IRQHandler,         /* [31] USB                            */
};


/* reset handler -----------------------------------------------------------*/
__attribute__((naked)) void Reset_Handler(void);
void Reset_Handler(void) {
    extern int main(void);
    extern int __libc_init_array(void);
    extern unsigned __data_start;  /* start of .data in the linker script */
    extern unsigned __data_end__;  /* end of .data in the linker script */
    extern unsigned const __data_load; /* initialization values for .data  */
    extern unsigned __bss_start__; /* start of .bss in the linker script */
    extern unsigned __bss_end__;   /* end of .bss in the linker script */
    extern void software_init_hook(void) __attribute__((weak));

    SystemInit(); /* CMSIS system initialization */

    /* copy the data segment initializers from flash to RAM... */
    unsigned const *src = &__data_load;
    unsigned *dst;
    for (dst = &__data_start; dst < &__data_end__; ++dst, ++src) {
        *dst = *src;
    }

    /* zero fill the .bss segment in RAM... */
    for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
        *dst = 0;
    }

    /* init hook provided? */
    if (&software_init_hook != (void (*)(void))(0)) {
        /* give control to the RTOS */
        software_init_hook(); /* this will also call __libc_init_array */
    }
    else {
        /* call all static constructors in C++ (harmless in C programs) */
        __libc_init_array();
        (void)main(); /* application's entry point; should never return! */
    }

    /* the previous code should not return, but assert just in case... */
    assert_failed("Reset_Handler", 1U);
}


/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("NMI_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("HardFault_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("Default_Handler", 1U);
}
