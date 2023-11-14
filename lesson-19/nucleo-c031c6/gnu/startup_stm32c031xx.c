/* File: startup_stm32c031xx.c for GNU-ARM
 * Purpose: startup file for STM32C031xx Cortex-M0+ device.
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
void Reserved1_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void FLASH_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void RCC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_15_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved8_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Channel2_3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMAMUX1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved15_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved17_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved18_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM14_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved20_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM16_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM17_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved24_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved26_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void USART2_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved29_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved30_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved31_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));

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
    (int)&WWDG_IRQHandler,        /* [ 0] Window Watchdog           */
    (int)&Reserved1_IRQHandler,   /* [ 1] Reserved                  */
    (int)&RTC_IRQHandler,         /* [ 2] RTC through EXTI Line     */
    (int)&FLASH_IRQHandler,       /* [ 3] FLASH                     */
    (int)&RCC_IRQHandler,         /* [ 4] RCC                       */
    (int)&EXTI0_1_IRQHandler,     /* [ 5] EXTI Line 0 and 1         */
    (int)&EXTI2_3_IRQHandler,     /* [ 6] EXTI Line 2 and 3         */
    (int)&EXTI4_15_IRQHandler,    /* [ 7] EXTI Line 4 to 15         */
    (int)&Reserved8_IRQHandler,   /* [ 8] Reserved                  */
    (int)&DMA1_Channel1_IRQHandler,/* [ 9] DMA1 Channel 1           */
    (int)&DMA1_Channel2_3_IRQHandler,/* [10] DMA1 Channel 2 and Channel 3 */
    (int)&DMAMUX1_IRQHandler,     /* [11] DMAMUX                    */
    (int)&ADC1_IRQHandler,        /* [12] ADC1                      */
    (int)&TIM1_BRK_UP_TRG_COM_IRQHandler,/* [13] TIM1 Break,Update,Trigger & Comm */
    (int)&TIM1_CC_IRQHandler,     /* [14] TIM1 Capture Compare      */
    (int)&Reserved15_IRQHandler,  /* [15] Reserved                  */
    (int)&TIM3_IRQHandler,        /* [16] TIM3                      */
    (int)&Reserved17_IRQHandler,  /* [17] Reserved                  */
    (int)&Reserved18_IRQHandler,  /* [18] Reserved                  */
    (int)&TIM14_IRQHandler,       /* [19] TIM14                     */
    (int)&Reserved20_IRQHandler,  /* [20] Reserved                  */
    (int)&TIM16_IRQHandler,       /* [21] TIM16                     */
    (int)&TIM17_IRQHandler,       /* [22] TIM17                     */
    (int)&I2C1_IRQHandler,        /* [23] I2C1                      */
    (int)&Reserved24_IRQHandler,  /* [24] Reserved                  */
    (int)&SPI1_IRQHandler,        /* [25] SPI1                      */
    (int)&Reserved26_IRQHandler,  /* [26] Reserved                  */
    (int)&USART1_IRQHandler,      /* [27] USART1                    */
    (int)&USART2_IRQHandler,      /* [28] USART2                    */
    (int)&Reserved29_IRQHandler,  /* [29] Reserved                  */
    (int)&Reserved30_IRQHandler,  /* [30] Reserved                  */
    (int)&Reserved31_IRQHandler,  /* [31] Reserved                  */
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
        //???__libc_init_array();
        (void)main(); /* application's entry point; should never return! */
    }

    /* the previous code should not return, but assert just in case... */
    __asm volatile ("  CPSID i");
    assert_failed("Reset_Handler", 1U);
}

/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("NMI", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("HardFault", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    /* disable interrupts and reset SP in case of stack overflow */
    __asm volatile ("  CPSID i\n  MOV   sp,%0" : : "r" (&__stack_end__));
    assert_failed("Default", 1U);
}
