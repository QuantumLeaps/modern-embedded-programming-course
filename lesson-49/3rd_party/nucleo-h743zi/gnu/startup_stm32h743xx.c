/* File: startup_stm32h743xx.c for GNU-ARM
 * Purpose: startup file for STM32H743xx Cortex-M7 device.
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
void WWDG_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void PVD_AVD_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ADC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FDCAN1_IT0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FDCAN2_IT0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FDCAN1_IT1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FDCAN2_IT1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void USART3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FMC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SDMMC1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void UART4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void UART5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ETH_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ETH_WKUP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FDCAN_CAL_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void USART6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_EP1_IN_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_WKUP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_HS_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DCMI_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void RNG_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void UART7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void UART8_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPI6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SAI1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LTDC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LTDC_ER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMA2D_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SAI2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void QUADSPI_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void CEC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C4_EV_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void I2C4_ER_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SPDIF_RX_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_EP1_OUT_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_EP1_IN_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMAMUX1_OVR_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_Master_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_TIMA_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_TIMB_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_TIMC_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_TIMD_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_TIME_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HRTIM1_FLT_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DFSDM1_FLT0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DFSDM1_FLT1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DFSDM1_FLT2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DFSDM1_FLT3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SAI3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SWPMI1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM15_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM16_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void TIM17_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void MDIOS_WKUP_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void MDIOS_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void JPEG_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void MDMA_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SDMMC2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void HSEM1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void ADC3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void DMAMUX2_OVR_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel0_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel6_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void BDMA_Channel7_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void COMP1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM2_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM3_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPTIM5_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void LPUART1_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void CRS_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void SAI4_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WAKEUP_PIN_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved42_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved64_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved65_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved66_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved67_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved79_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved124_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved123_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved126_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved143_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved145_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved147_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void Reserved148_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));

/*..........................................................................*/
__attribute__ ((section(".isr_vector")))
int const g_pfnVectors[] = {
    (int)&__stack_end__,            /* Top of Stack                        */
    (int)&Reset_Handler,            /* Reset Handler                       */
    (int)&NMI_Handler,              /* NMI Handler                         */
    (int)&HardFault_Handler,        /* Hard Fault Handler                  */
    (int)&MemManage_Handler,        /* The MPU fault handler               */
    (int)&BusFault_Handler,         /* The bus fault handler               */
    (int)&UsageFault_Handler,       /* The usage fault handler             */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&SVC_Handler,              /* SVCall handler                      */
    (int)&DebugMon_Handler,         /* Debug monitor handler               */
    (int)&Default_Handler,          /* Reserved                            */
    (int)&PendSV_Handler,           /* The PendSV handler                  */
    (int)&SysTick_Handler,          /* The SysTick handler                 */

    /* IRQ handlers... */
    (int)&WWDG_IRQHandler,          /* [ 0] Window WatchDog Interrupt ( wwdg1_it) */
    (int)&PVD_AVD_IRQHandler,       /* [ 1] PVD/AVD through EXTI Line detection */
    (int)&TAMP_STAMP_IRQHandler,    /* [ 2] Tamper and TimeStamps through EXTI line */
    (int)&RTC_WKUP_IRQHandler,      /* [ 3] RTC Wakeup through the EXTI line */
    (int)&FLASH_IRQHandler,         /* [ 4] FLASH */
    (int)&RCC_IRQHandler,           /* [ 5] RCC */
    (int)&EXTI0_IRQHandler,         /* [ 6] EXTI Line0 */
    (int)&EXTI1_IRQHandler,         /* [ 7] EXTI Line1 */
    (int)&EXTI2_IRQHandler,         /* [ 8] EXTI Line2 */
    (int)&EXTI3_IRQHandler,         /* [ 9] EXTI Line3 */
    (int)&EXTI4_IRQHandler,         /* [10] EXTI Line4 */
    (int)&DMA1_Stream0_IRQHandler,  /* [11] DMA1 Stream 0 */
    (int)&DMA1_Stream1_IRQHandler,  /* [12] DMA1 Stream 1 */
    (int)&DMA1_Stream2_IRQHandler,  /* [13] DMA1 Stream 2 */
    (int)&DMA1_Stream3_IRQHandler,  /* [14] DMA1 Stream 3 */
    (int)&DMA1_Stream4_IRQHandler,  /* [15] DMA1 Stream 4 */
    (int)&DMA1_Stream5_IRQHandler,  /* [16] DMA1 Stream 5 */
    (int)&DMA1_Stream6_IRQHandler,  /* [17] DMA1 Stream 6 */
    (int)&ADC_IRQHandler,           /* [18] ADC1, ADC2 */
    (int)&FDCAN1_IT0_IRQHandler,    /* [19] FDCAN1 interrupt line 0 */
    (int)&FDCAN2_IT0_IRQHandler,    /* [20] FDCAN2 interrupt line 0 */
    (int)&FDCAN1_IT1_IRQHandler,    /* [21] FDCAN1 interrupt line 1 */
    (int)&FDCAN2_IT1_IRQHandler,    /* [22] FDCAN2 interrupt line 1 */
    (int)&EXTI9_5_IRQHandler,       /* [23] External Line[9:5]s */
    (int)&TIM1_BRK_IRQHandler,      /* [24] TIM1 Break interrupt */
    (int)&TIM1_UP_IRQHandler,       /* [25] TIM1 Update */
    (int)&TIM1_TRG_COM_IRQHandler,  /* [26] TIM1 Trigger and Commutation Interrupt */
    (int)&TIM1_CC_IRQHandler,       /* [27] TIM1 Capture Compare */
    (int)&TIM2_IRQHandler,          /* [28] TIM2 */
    (int)&TIM3_IRQHandler,          /* [29] TIM3 */
    (int)&TIM4_IRQHandler,          /* [30] TIM4 */
    (int)&I2C1_EV_IRQHandler,       /* [31] I2C1 Event */
    (int)&I2C1_ER_IRQHandler,       /* [32] I2C1 Error */
    (int)&I2C2_EV_IRQHandler,       /* [33] I2C2 Event */
    (int)&I2C2_ER_IRQHandler,       /* [34] I2C2 Error */
    (int)&SPI1_IRQHandler,          /* [35] SPI1 */
    (int)&SPI2_IRQHandler,          /* [36] SPI2 */
    (int)&USART1_IRQHandler,        /* [37] USART1 */
    (int)&USART2_IRQHandler,        /* [38] USART2 */
    (int)&USART3_IRQHandler,        /* [39] USART3 */
    (int)&EXTI15_10_IRQHandler,     /* [40] External Line[15:10] */
    (int)&RTC_Alarm_IRQHandler,     /* [41] RTC Alarm (A and B) through EXTI Line */
    (int)&Reserved42_IRQHandler,    /* [42] Reserved */
    (int)&TIM8_BRK_TIM12_IRQHandler,/* [43] TIM8 Break Interrupt and TIM12 global interrupt */
    (int)&TIM8_UP_TIM13_IRQHandler, /* [44] TIM8 Update Interrupt and TIM13 global interrupt */
    (int)&TIM8_TRG_COM_TIM14_IRQHandler,/* [45] TIM8 Trigger and Commutation Interrupt and TIM14 glob */
    (int)&TIM8_CC_IRQHandler,       /* [46] TIM8 Capture Compare Interrupt */
    (int)&DMA1_Stream7_IRQHandler,  /* [47] DMA1 Stream7 */
    (int)&FMC_IRQHandler,           /* [48] FMC */
    (int)&SDMMC1_IRQHandler,        /* [49] SDMMC1 */
    (int)&TIM5_IRQHandler,          /* [50] TIM5 */
    (int)&SPI3_IRQHandler,          /* [51] SPI3 */
    (int)&UART4_IRQHandler,         /* [52] UART4 */
    (int)&UART5_IRQHandler,         /* [53] UART5 */
    (int)&TIM6_DAC_IRQHandler,      /* [54] TIM6 and DAC1&2 underrun errors */
    (int)&TIM7_IRQHandler,          /* [55] TIM7 */
    (int)&DMA2_Stream0_IRQHandler,  /* [56] DMA2 Stream 0 */
    (int)&DMA2_Stream1_IRQHandler,  /* [57] DMA2 Stream 1 */
    (int)&DMA2_Stream2_IRQHandler,  /* [58] DMA2 Stream 2 */
    (int)&DMA2_Stream3_IRQHandler,  /* [59] DMA2 Stream 3 */
    (int)&DMA2_Stream4_IRQHandler,  /* [60] DMA2 Stream 4 */
    (int)&ETH_IRQHandler,           /* [61] Ethernet */
    (int)&ETH_WKUP_IRQHandler,      /* [62] Ethernet Wakeup through EXTI line */
    (int)&FDCAN_CAL_IRQHandler,     /* [63] FDCAN calibration unit interrupt */
    (int)&Reserved64_IRQHandler,    /* [64] Reserved */
    (int)&Reserved65_IRQHandler,    /* [65] Reserved */
    (int)&Reserved66_IRQHandler,    /* [66] Reserved */
    (int)&Reserved67_IRQHandler,    /* [67] Reserved */
    (int)&DMA2_Stream5_IRQHandler,  /* [68] DMA2 Stream 5 */
    (int)&DMA2_Stream6_IRQHandler,  /* [69] DMA2 Stream 6 */
    (int)&DMA2_Stream7_IRQHandler,  /* [70] DMA2 Stream 7 */
    (int)&USART6_IRQHandler,        /* [71] USART6 */
    (int)&I2C3_EV_IRQHandler,       /* [72] I2C3 event */
    (int)&I2C3_ER_IRQHandler,       /* [73] I2C3 error */
    (int)&OTG_HS_EP1_OUT_IRQHandler,/* [74] USB OTG HS End Point 1 Out */
    (int)&OTG_HS_EP1_IN_IRQHandler, /* [75] USB OTG HS End Point 1 In */
    (int)&OTG_HS_WKUP_IRQHandler,   /* [76] USB OTG HS Wakeup through EXTI */
    (int)&OTG_HS_IRQHandler,        /* [77] USB OTG HS */
    (int)&DCMI_IRQHandler,          /* [78] DCMI */
    (int)&Reserved79_IRQHandler,    /* [79] Reserved */
    (int)&RNG_IRQHandler,           /* [80] Rng */
    (int)&FPU_IRQHandler,           /* [81] FPU */
    (int)&UART7_IRQHandler,         /* [82] UART7 */
    (int)&UART8_IRQHandler,         /* [83] UART8 */
    (int)&SPI4_IRQHandler,          /* [84] SPI4 */
    (int)&SPI5_IRQHandler,          /* [85] SPI5 */
    (int)&SPI6_IRQHandler,          /* [86] SPI6 */
    (int)&SAI1_IRQHandler,          /* [87] SAI1 */
    (int)&LTDC_IRQHandler,          /* [88] LTDC */
    (int)&LTDC_ER_IRQHandler,       /* [89] LTDC error */
    (int)&DMA2D_IRQHandler,         /* [90] DMA2D */
    (int)&SAI2_IRQHandler,          /* [91] SAI2 */
    (int)&QUADSPI_IRQHandler,       /* [92] QUADSPI */
    (int)&LPTIM1_IRQHandler,        /* [93] LPTIM1 */
    (int)&CEC_IRQHandler,           /* [94] HDMI_CEC */
    (int)&I2C4_EV_IRQHandler,       /* [95] I2C4 Event */
    (int)&I2C4_ER_IRQHandler,       /* [96] I2C4 Error */
    (int)&SPDIF_RX_IRQHandler,      /* [97] SPDIF_RX */
    (int)&OTG_FS_EP1_OUT_IRQHandler,/* [98] USB OTG FS End Point 1 Out */
    (int)&OTG_FS_EP1_IN_IRQHandler, /* [99] USB OTG FS End Point 1 In */
    (int)&OTG_FS_WKUP_IRQHandler,   /* [100] USB OTG FS Wakeup through EXTI */
    (int)&OTG_FS_IRQHandler,        /* [101] USB OTG FS */
    (int)&DMAMUX1_OVR_IRQHandler,   /* [102] DMAMUX1 Overrun interrupt */
    (int)&HRTIM1_Master_IRQHandler, /* [103] HRTIM Master Timer global Interrupts */
    (int)&HRTIM1_TIMA_IRQHandler,   /* [104] HRTIM Timer A global Interrupt */
    (int)&HRTIM1_TIMB_IRQHandler,   /* [105] HRTIM Timer B global Interrupt */
    (int)&HRTIM1_TIMC_IRQHandler,   /* [106] HRTIM Timer C global Interrupt */
    (int)&HRTIM1_TIMD_IRQHandler,   /* [107] HRTIM Timer D global Interrupt */
    (int)&HRTIM1_TIME_IRQHandler,   /* [108] HRTIM Timer E global Interrupt */
    (int)&HRTIM1_FLT_IRQHandler,    /* [109] HRTIM Fault global Interrupt */
    (int)&DFSDM1_FLT0_IRQHandler,   /* [110] DFSDM Filter0 Interrupt */
    (int)&DFSDM1_FLT1_IRQHandler,   /* [111] DFSDM Filter1 Interrupt */
    (int)&DFSDM1_FLT2_IRQHandler,   /* [112] DFSDM Filter2 Interrupt */
    (int)&DFSDM1_FLT3_IRQHandler,   /* [113] DFSDM Filter3 Interrupt */
    (int)&SAI3_IRQHandler,          /* [114] SAI3 global Interrupt */
    (int)&SWPMI1_IRQHandler,        /* [115] Serial Wire Interface 1 global interrupt */
    (int)&TIM15_IRQHandler,         /* [116] TIM15 global Interrupt */
    (int)&TIM16_IRQHandler,         /* [117] TIM16 global Interrupt */
    (int)&TIM17_IRQHandler,         /* [118] TIM17 global Interrupt */
    (int)&MDIOS_WKUP_IRQHandler,    /* [119] MDIOS Wakeup  Interrupt */
    (int)&MDIOS_IRQHandler,         /* [120] MDIOS global Interrupt */
    (int)&JPEG_IRQHandler,          /* [121] JPEG global Interrupt */
    (int)&MDMA_IRQHandler,          /* [122] MDMA global Interrupt */
    (int)&Reserved123_IRQHandler,   /* [123] Reserved */
    (int)&SDMMC2_IRQHandler,        /* [124] SDMMC2 global Interrupt */
    (int)&HSEM1_IRQHandler,         /* [125] HSEM1 global Interrupt */
    (int)&Reserved126_IRQHandler,   /* [126] Reserved */
    (int)&ADC3_IRQHandler,          /* [127] ADC3 global Interrupt */
    (int)&DMAMUX2_OVR_IRQHandler,   /* [128] DMAMUX Overrun interrupt */
    (int)&BDMA_Channel0_IRQHandler, /* [129] BDMA Channel 0 global Interrupt */
    (int)&BDMA_Channel1_IRQHandler, /* [130] BDMA Channel 1 global Interrupt */
    (int)&BDMA_Channel2_IRQHandler, /* [131] BDMA Channel 2 global Interrupt */
    (int)&BDMA_Channel3_IRQHandler, /* [132] BDMA Channel 3 global Interrupt */
    (int)&BDMA_Channel4_IRQHandler, /* [133] BDMA Channel 4 global Interrupt */
    (int)&BDMA_Channel5_IRQHandler, /* [134] BDMA Channel 5 global Interrupt */
    (int)&BDMA_Channel6_IRQHandler, /* [135] BDMA Channel 6 global Interrupt */
    (int)&BDMA_Channel7_IRQHandler, /* [136] BDMA Channel 7 global Interrupt */
    (int)&COMP1_IRQHandler,         /* [137] COMP1 global Interrupt */
    (int)&LPTIM2_IRQHandler,        /* [138] LP TIM2 global interrupt */
    (int)&LPTIM3_IRQHandler,        /* [139] LP TIM3 global interrupt */
    (int)&LPTIM4_IRQHandler,        /* [140] LP TIM4 global interrupt */
    (int)&LPTIM5_IRQHandler,        /* [141] LP TIM5 global interrupt */
    (int)&LPUART1_IRQHandler,       /* [142] LP UART1 interrupt */
    (int)&Reserved143_IRQHandler,   /* [143] Reserved */
    (int)&CRS_IRQHandler,           /* [144] Clock Recovery Global Interrupt */
    (int)&Reserved145_IRQHandler,   /* [145] Reserved */
    (int)&SAI4_IRQHandler,          /* [146] SAI4 global interrupt */
    (int)&Reserved147_IRQHandler,   /* [147] Reserved */
    (int)&Reserved148_IRQHandler,   /* [148] Reserved */
    (int)&WAKEUP_PIN_IRQHandler,    /* [149] Interrupt for all 6 wake-up pins */
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
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("MemManage_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("MemManage_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("BusFault_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("Default_Handler", 1U);
}
