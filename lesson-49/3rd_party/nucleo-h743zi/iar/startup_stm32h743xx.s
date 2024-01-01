;/***************************************************************************/
; * @file     startup_stm32h743xx.s for IAR ARM assembler
; * @brief    CMSIS Cortex-M7 Core Device Startup File for STM32H743xx
; * @version  CMSIS 5.9.0
; * @date     1 Feb 2023
; *
; * Modified by Quantum Leaps:
; * - Added relocating of the Vector Table to free up the 256B region at 0x0
; *   for NULL-pointer protection by the MPU.
; * - Modified all exception handlers to branch to assert_failed()
; *   instead of locking up the CPU inside an endless loop.
; *
; * @description
; * Created from the CMSIS template for the specified device
; * Quantum Leaps, www.state-machine.com
; *

        MODULE  ?cstartup

        ; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(8)

        PUBLIC  __vector_table
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

;******************************************************************************
; The vector table.
;
        DATA
__vector_table
    ; Initial Vector Table before relocation
        DCD     sfe(CSTACK)
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU fault handler
        DCD     BusFault_Handler            ; Bus fault handler
        DCD     UsageFault_Handler          ; Usage fault handler
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug Monitor handler
        DCD     Default_Handler             ; Reserved
        DCD     PendSV_Handler              ; PendSV handler
        DCD     SysTick_Handler             ; SysTick handler

        ; IRQ handlers...
        DCD     WWDG_IRQHandler             ; [ 0] Window WatchDog Interrupt ( wwdg1_it)
        DCD     PVD_AVD_IRQHandler          ; [ 1] PVD/AVD through EXTI Line detection
        DCD     TAMP_STAMP_IRQHandler       ; [ 2] Tamper and TimeStamps through EXTI line
        DCD     RTC_WKUP_IRQHandler         ; [ 3] RTC Wakeup through the EXTI line
        DCD     FLASH_IRQHandler            ; [ 4] FLASH
        DCD     RCC_IRQHandler              ; [ 5] RCC
        DCD     EXTI0_IRQHandler            ; [ 6] EXTI Line0
        DCD     EXTI1_IRQHandler            ; [ 7] EXTI Line1
        DCD     EXTI2_IRQHandler            ; [ 8] EXTI Line2
        DCD     EXTI3_IRQHandler            ; [ 9] EXTI Line3
        DCD     EXTI4_IRQHandler            ; [10] EXTI Line4
        DCD     DMA1_Stream0_IRQHandler     ; [11] DMA1 Stream 0
        DCD     DMA1_Stream1_IRQHandler     ; [12] DMA1 Stream 1
        DCD     DMA1_Stream2_IRQHandler     ; [13] DMA1 Stream 2
        DCD     DMA1_Stream3_IRQHandler     ; [14] DMA1 Stream 3
        DCD     DMA1_Stream4_IRQHandler     ; [15] DMA1 Stream 4
        DCD     DMA1_Stream5_IRQHandler     ; [16] DMA1 Stream 5
        DCD     DMA1_Stream6_IRQHandler     ; [17] DMA1 Stream 6
        DCD     ADC_IRQHandler              ; [18] ADC1, ADC2
        DCD     FDCAN1_IT0_IRQHandler       ; [19] FDCAN1 interrupt line 0
        DCD     FDCAN2_IT0_IRQHandler       ; [20] FDCAN2 interrupt line 0
        DCD     FDCAN1_IT1_IRQHandler       ; [21] FDCAN1 interrupt line 1
        DCD     FDCAN2_IT1_IRQHandler       ; [22] FDCAN2 interrupt line 1
        DCD     EXTI9_5_IRQHandler          ; [23] External Line[9:5]s
        DCD     TIM1_BRK_IRQHandler         ; [24] TIM1 Break interrupt
        DCD     TIM1_UP_IRQHandler          ; [25] TIM1 Update
        DCD     TIM1_TRG_COM_IRQHandler     ; [26] TIM1 Trigger and Commutation Interrupt
        DCD     TIM1_CC_IRQHandler          ; [27] TIM1 Capture Compare
        DCD     TIM2_IRQHandler             ; [28] TIM2
        DCD     TIM3_IRQHandler             ; [29] TIM3
        DCD     TIM4_IRQHandler             ; [30] TIM4
        DCD     I2C1_EV_IRQHandler          ; [31] I2C1 Event
        DCD     I2C1_ER_IRQHandler          ; [32] I2C1 Error
        DCD     I2C2_EV_IRQHandler          ; [33] I2C2 Event
        DCD     I2C2_ER_IRQHandler          ; [34] I2C2 Error
        DCD     SPI1_IRQHandler             ; [35] SPI1
        DCD     SPI2_IRQHandler             ; [36] SPI2
        DCD     USART1_IRQHandler           ; [37] USART1
        DCD     USART2_IRQHandler           ; [38] USART2
        DCD     USART3_IRQHandler           ; [39] USART3
        DCD     EXTI15_10_IRQHandler        ; [40] External Line[15:10]
        DCD     RTC_Alarm_IRQHandler        ; [41] RTC Alarm (A and B) through EXTI Line
        DCD     Reserved42_IRQHandler       ; [42] Reserved
        DCD     TIM8_BRK_TIM12_IRQHandler   ; [43] TIM8 Break Interrupt and TIM12 global interrupt
        DCD     TIM8_UP_TIM13_IRQHandler    ; [44] TIM8 Update Interrupt and TIM13 global interrupt
        DCD     TIM8_TRG_COM_TIM14_IRQHandler;[45] TIM8 Trigger and Commutation Interrupt and TIM14 glob
        DCD     TIM8_CC_IRQHandler          ; [46] TIM8 Capture Compare Interrupt
        DCD     DMA1_Stream7_IRQHandler     ; [47] DMA1 Stream7
        DCD     FMC_IRQHandler              ; [48] FMC
        DCD     SDMMC1_IRQHandler           ; [49] SDMMC1
        DCD     TIM5_IRQHandler             ; [50] TIM5
        DCD     SPI3_IRQHandler             ; [51] SPI3
        DCD     UART4_IRQHandler            ; [52] UART4
        DCD     UART5_IRQHandler            ; [53] UART5
        DCD     TIM6_DAC_IRQHandler         ; [54] TIM6 and DAC1&2 underrun errors
        DCD     TIM7_IRQHandler             ; [55] TIM7
        DCD     DMA2_Stream0_IRQHandler     ; [56] DMA2 Stream 0
        DCD     DMA2_Stream1_IRQHandler     ; [57] DMA2 Stream 1
        DCD     DMA2_Stream2_IRQHandler     ; [58] DMA2 Stream 2
        DCD     DMA2_Stream3_IRQHandler     ; [59] DMA2 Stream 3
        DCD     DMA2_Stream4_IRQHandler     ; [60] DMA2 Stream 4
        DCD     ETH_IRQHandler              ; [61] Ethernet
        DCD     ETH_WKUP_IRQHandler         ; [62] Ethernet Wakeup through EXTI line
        DCD     FDCAN_CAL_IRQHandler        ; [63] FDCAN calibration unit interrupt
        DCD     Reserved64_IRQHandler       ; [64] Reserved
        DCD     Reserved65_IRQHandler       ; [65] Reserved
        DCD     Reserved66_IRQHandler       ; [66] Reserved
        DCD     Reserved67_IRQHandler       ; [67] Reserved
        DCD     DMA2_Stream5_IRQHandler     ; [68] DMA2 Stream 5
        DCD     DMA2_Stream6_IRQHandler     ; [69] DMA2 Stream 6
        DCD     DMA2_Stream7_IRQHandler     ; [70] DMA2 Stream 7
        DCD     USART6_IRQHandler           ; [71] USART6
        DCD     I2C3_EV_IRQHandler          ; [72] I2C3 event
        DCD     I2C3_ER_IRQHandler          ; [73] I2C3 error
        DCD     OTG_HS_EP1_OUT_IRQHandler   ; [74] USB OTG HS End Point 1 Out
        DCD     OTG_HS_EP1_IN_IRQHandler    ; [75] USB OTG HS End Point 1 In
        DCD     OTG_HS_WKUP_IRQHandler      ; [76] USB OTG HS Wakeup through EXTI
        DCD     OTG_HS_IRQHandler           ; [77] USB OTG HS
        DCD     DCMI_IRQHandler             ; [78] DCMI
        DCD     Reserved79_IRQHandler       ; [79] Reserved
        DCD     RNG_IRQHandler              ; [80] Rng
        DCD     FPU_IRQHandler              ; [81] FPU
        DCD     UART7_IRQHandler            ; [82] UART7
        DCD     UART8_IRQHandler            ; [83] UART8
        DCD     SPI4_IRQHandler             ; [84] SPI4
        DCD     SPI5_IRQHandler             ; [85] SPI5
        DCD     SPI6_IRQHandler             ; [86] SPI6
        DCD     SAI1_IRQHandler             ; [87] SAI1
        DCD     LTDC_IRQHandler             ; [88] LTDC
        DCD     LTDC_ER_IRQHandler          ; [89] LTDC error
        DCD     DMA2D_IRQHandler            ; [90] DMA2D
        DCD     SAI2_IRQHandler             ; [91] SAI2
        DCD     QUADSPI_IRQHandler          ; [92] QUADSPI
        DCD     LPTIM1_IRQHandler           ; [93] LPTIM1
        DCD     CEC_IRQHandler              ; [94] HDMI_CEC
        DCD     I2C4_EV_IRQHandler          ; [95] I2C4 Event
        DCD     I2C4_ER_IRQHandler          ; [96] I2C4 Error
        DCD     SPDIF_RX_IRQHandler         ; [97] SPDIF_RX
        DCD     OTG_FS_EP1_OUT_IRQHandler   ; [98] USB OTG FS End Point 1 Out
        DCD     OTG_FS_EP1_IN_IRQHandler    ; [99] USB OTG FS End Point 1 In
        DCD     OTG_FS_WKUP_IRQHandler      ; [100] USB OTG FS Wakeup through EXTI
        DCD     OTG_FS_IRQHandler           ; [101] USB OTG FS
        DCD     DMAMUX1_OVR_IRQHandler      ; [102] DMAMUX1 Overrun interrupt
        DCD     HRTIM1_Master_IRQHandler    ; [103] HRTIM Master Timer global Interrupts
        DCD     HRTIM1_TIMA_IRQHandler      ; [104] HRTIM Timer A global Interrupt
        DCD     HRTIM1_TIMB_IRQHandler      ; [105] HRTIM Timer B global Interrupt
        DCD     HRTIM1_TIMC_IRQHandler      ; [106] HRTIM Timer C global Interrupt
        DCD     HRTIM1_TIMD_IRQHandler      ; [107] HRTIM Timer D global Interrupt
        DCD     HRTIM1_TIME_IRQHandler      ; [108] HRTIM Timer E global Interrupt
        DCD     HRTIM1_FLT_IRQHandler       ; [109] HRTIM Fault global Interrupt
        DCD     DFSDM1_FLT0_IRQHandler      ; [110] DFSDM Filter0 Interrupt
        DCD     DFSDM1_FLT1_IRQHandler      ; [111] DFSDM Filter1 Interrupt
        DCD     DFSDM1_FLT2_IRQHandler      ; [112] DFSDM Filter2 Interrupt
        DCD     DFSDM1_FLT3_IRQHandler      ; [113] DFSDM Filter3 Interrupt
        DCD     SAI3_IRQHandler             ; [114] SAI3 global Interrupt
        DCD     SWPMI1_IRQHandler           ; [115] Serial Wire Interface 1 global interrupt
        DCD     TIM15_IRQHandler            ; [116] TIM15 global Interrupt
        DCD     TIM16_IRQHandler            ; [117] TIM16 global Interrupt
        DCD     TIM17_IRQHandler            ; [118] TIM17 global Interrupt
        DCD     MDIOS_WKUP_IRQHandler       ; [119] MDIOS Wakeup  Interrupt
        DCD     MDIOS_IRQHandler            ; [120] MDIOS global Interrupt
        DCD     JPEG_IRQHandler             ; [121] JPEG global Interrupt
        DCD     MDMA_IRQHandler             ; [122] MDMA global Interrupt
        DCD     Reserved123_IRQHandler      ; [123] Reserved
        DCD     SDMMC2_IRQHandler           ; [124] SDMMC2 global Interrupt
        DCD     HSEM1_IRQHandler            ; [125] HSEM1 global Interrupt
        DCD     Reserved126_IRQHandler      ; [126] Reserved
        DCD     ADC3_IRQHandler             ; [127] ADC3 global Interrupt
        DCD     DMAMUX2_OVR_IRQHandler      ; [128] DMAMUX Overrun interrupt
        DCD     BDMA_Channel0_IRQHandler    ; [129] BDMA Channel 0 global Interrupt
        DCD     BDMA_Channel1_IRQHandler    ; [130] BDMA Channel 1 global Interrupt
        DCD     BDMA_Channel2_IRQHandler    ; [131] BDMA Channel 2 global Interrupt
        DCD     BDMA_Channel3_IRQHandler    ; [132] BDMA Channel 3 global Interrupt
        DCD     BDMA_Channel4_IRQHandler    ; [133] BDMA Channel 4 global Interrupt
        DCD     BDMA_Channel5_IRQHandler    ; [134] BDMA Channel 5 global Interrupt
        DCD     BDMA_Channel6_IRQHandler    ; [135] BDMA Channel 6 global Interrupt
        DCD     BDMA_Channel7_IRQHandler    ; [136] BDMA Channel 7 global Interrupt
        DCD     COMP1_IRQHandler            ; [137] COMP1 global Interrupt
        DCD     LPTIM2_IRQHandler           ; [138] LP TIM2 global interrupt
        DCD     LPTIM3_IRQHandler           ; [139] LP TIM3 global interrupt
        DCD     LPTIM4_IRQHandler           ; [140] LP TIM4 global interrupt
        DCD     LPTIM5_IRQHandler           ; [141] LP TIM5 global interrupt
        DCD     LPUART1_IRQHandler          ; [142] LP UART1 interrupt
        DCD     Reserved143_IRQHandler      ; [143] Reserved
        DCD     CRS_IRQHandler              ; [144] Clock Recovery Global Interrupt
        DCD     Reserved145_IRQHandler      ; [145] Reserved
        DCD     SAI4_IRQHandler             ; [146] SAI4 global interrupt
        DCD     Reserved147_IRQHandler      ; [147] Reserved
        DCD     Reserved148_IRQHandler      ; [148] Reserved
        DCD     WAKEUP_PIN_IRQHandler       ; [149] Interrupt for all 6 wake-up pins

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        SECTION .text:CODE:REORDER:NOROOT(2)

;******************************************************************************
; This is the code that gets called when theessor first starts execution
; following a reset event.
;
        PUBWEAK Reset_Handler
        EXTERN  SystemInit
        EXTERN  __iar_program_start
        EXTERN  assert_failed

Reset_Handler
        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; pre-fill the CSTACK with 0xDEADBEEF...................
        LDR     r0,=0xDEADBEEF
        MOV     r1,r0
        LDR     r2,=sfb(CSTACK)
        LDR     r3,=sfe(CSTACK)
Reset_stackInit_fill:
        STMIA   r2!,{r0,r1}
        CMP     r2,r3
        BLT.N   Reset_stackInit_fill

        LDR     r0,=__iar_program_start ; IAR startup code
        BLX     r0

        ; __iar_program_start calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK NMI_Handler
NMI_Handler
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK HardFault_Handler
HardFault_Handler
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK MemManage_Handler
MemManage_Handler
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK BusFault_Handler
BusFault_Handler
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK UsageFault_Handler
UsageFault_Handler
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGNROM 2

;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
        PUBWEAK SVC_Handler
SVC_Handler
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK DebugMon_Handler
DebugMon_Handler
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK PendSV_Handler
PendSV_Handler
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK SysTick_Handler
SysTick_Handler
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGNROM 2

;******************************************************************************
; Weak IRQ handlers...
;

        PUBWEAK Default_Handler
        PUBWEAK WWDG_IRQHandler
        PUBWEAK PVD_AVD_IRQHandler
        PUBWEAK TAMP_STAMP_IRQHandler
        PUBWEAK RTC_WKUP_IRQHandler
        PUBWEAK FLASH_IRQHandler
        PUBWEAK RCC_IRQHandler
        PUBWEAK EXTI0_IRQHandler
        PUBWEAK EXTI1_IRQHandler
        PUBWEAK EXTI2_IRQHandler
        PUBWEAK EXTI3_IRQHandler
        PUBWEAK EXTI4_IRQHandler
        PUBWEAK DMA1_Stream0_IRQHandler
        PUBWEAK DMA1_Stream1_IRQHandler
        PUBWEAK DMA1_Stream2_IRQHandler
        PUBWEAK DMA1_Stream3_IRQHandler
        PUBWEAK DMA1_Stream4_IRQHandler
        PUBWEAK DMA1_Stream5_IRQHandler
        PUBWEAK DMA1_Stream6_IRQHandler
        PUBWEAK ADC_IRQHandler
        PUBWEAK FDCAN1_IT0_IRQHandler
        PUBWEAK FDCAN2_IT0_IRQHandler
        PUBWEAK FDCAN1_IT1_IRQHandler
        PUBWEAK FDCAN2_IT1_IRQHandler
        PUBWEAK EXTI9_5_IRQHandler
        PUBWEAK TIM1_BRK_IRQHandler
        PUBWEAK TIM1_UP_IRQHandler
        PUBWEAK TIM1_TRG_COM_IRQHandler
        PUBWEAK TIM1_CC_IRQHandler
        PUBWEAK TIM2_IRQHandler
        PUBWEAK TIM3_IRQHandler
        PUBWEAK TIM4_IRQHandler
        PUBWEAK I2C1_EV_IRQHandler
        PUBWEAK I2C1_ER_IRQHandler
        PUBWEAK I2C2_EV_IRQHandler
        PUBWEAK I2C2_ER_IRQHandler
        PUBWEAK SPI1_IRQHandler
        PUBWEAK SPI2_IRQHandler
        PUBWEAK USART1_IRQHandler
        PUBWEAK USART2_IRQHandler
        PUBWEAK USART3_IRQHandler
        PUBWEAK EXTI15_10_IRQHandler
        PUBWEAK RTC_Alarm_IRQHandler
        PUBWEAK TIM8_BRK_TIM12_IRQHandler
        PUBWEAK TIM8_UP_TIM13_IRQHandler
        PUBWEAK TIM8_TRG_COM_TIM14_IRQHandler
        PUBWEAK TIM8_CC_IRQHandler
        PUBWEAK DMA1_Stream7_IRQHandler
        PUBWEAK FMC_IRQHandler
        PUBWEAK SDMMC1_IRQHandler
        PUBWEAK TIM5_IRQHandler
        PUBWEAK SPI3_IRQHandler
        PUBWEAK UART4_IRQHandler
        PUBWEAK UART5_IRQHandler
        PUBWEAK TIM6_DAC_IRQHandler
        PUBWEAK TIM7_IRQHandler
        PUBWEAK DMA2_Stream0_IRQHandler
        PUBWEAK DMA2_Stream1_IRQHandler
        PUBWEAK DMA2_Stream2_IRQHandler
        PUBWEAK DMA2_Stream3_IRQHandler
        PUBWEAK DMA2_Stream4_IRQHandler
        PUBWEAK ETH_IRQHandler
        PUBWEAK ETH_WKUP_IRQHandler
        PUBWEAK FDCAN_CAL_IRQHandler
        PUBWEAK DMA2_Stream5_IRQHandler
        PUBWEAK DMA2_Stream6_IRQHandler
        PUBWEAK DMA2_Stream7_IRQHandler
        PUBWEAK USART6_IRQHandler
        PUBWEAK I2C3_EV_IRQHandler
        PUBWEAK I2C3_ER_IRQHandler
        PUBWEAK OTG_HS_EP1_OUT_IRQHandler
        PUBWEAK OTG_HS_EP1_IN_IRQHandler
        PUBWEAK OTG_HS_WKUP_IRQHandler
        PUBWEAK OTG_HS_IRQHandler
        PUBWEAK DCMI_IRQHandler
        PUBWEAK RNG_IRQHandler
        PUBWEAK FPU_IRQHandler
        PUBWEAK UART7_IRQHandler
        PUBWEAK UART8_IRQHandler
        PUBWEAK SPI4_IRQHandler
        PUBWEAK SPI5_IRQHandler
        PUBWEAK SPI6_IRQHandler
        PUBWEAK SAI1_IRQHandler
        PUBWEAK LTDC_IRQHandler
        PUBWEAK LTDC_ER_IRQHandler
        PUBWEAK DMA2D_IRQHandler
        PUBWEAK SAI2_IRQHandler
        PUBWEAK QUADSPI_IRQHandler
        PUBWEAK LPTIM1_IRQHandler
        PUBWEAK CEC_IRQHandler
        PUBWEAK I2C4_EV_IRQHandler
        PUBWEAK I2C4_ER_IRQHandler
        PUBWEAK SPDIF_RX_IRQHandler
        PUBWEAK OTG_FS_EP1_OUT_IRQHandler
        PUBWEAK OTG_FS_EP1_IN_IRQHandler
        PUBWEAK OTG_FS_WKUP_IRQHandler
        PUBWEAK OTG_FS_IRQHandler
        PUBWEAK DMAMUX1_OVR_IRQHandler
        PUBWEAK HRTIM1_Master_IRQHandler
        PUBWEAK HRTIM1_TIMA_IRQHandler
        PUBWEAK HRTIM1_TIMB_IRQHandler
        PUBWEAK HRTIM1_TIMC_IRQHandler
        PUBWEAK HRTIM1_TIMD_IRQHandler
        PUBWEAK HRTIM1_TIME_IRQHandler
        PUBWEAK HRTIM1_FLT_IRQHandler
        PUBWEAK DFSDM1_FLT0_IRQHandler
        PUBWEAK DFSDM1_FLT1_IRQHandler
        PUBWEAK DFSDM1_FLT2_IRQHandler
        PUBWEAK DFSDM1_FLT3_IRQHandler
        PUBWEAK SAI3_IRQHandler
        PUBWEAK SWPMI1_IRQHandler
        PUBWEAK TIM15_IRQHandler
        PUBWEAK TIM16_IRQHandler
        PUBWEAK TIM17_IRQHandler
        PUBWEAK MDIOS_WKUP_IRQHandler
        PUBWEAK MDIOS_IRQHandler
        PUBWEAK JPEG_IRQHandler
        PUBWEAK MDMA_IRQHandler
        PUBWEAK SDMMC2_IRQHandler
        PUBWEAK HSEM1_IRQHandler
        PUBWEAK ADC3_IRQHandler
        PUBWEAK DMAMUX2_OVR_IRQHandler
        PUBWEAK BDMA_Channel0_IRQHandler
        PUBWEAK BDMA_Channel1_IRQHandler
        PUBWEAK BDMA_Channel2_IRQHandler
        PUBWEAK BDMA_Channel3_IRQHandler
        PUBWEAK BDMA_Channel4_IRQHandler
        PUBWEAK BDMA_Channel5_IRQHandler
        PUBWEAK BDMA_Channel6_IRQHandler
        PUBWEAK BDMA_Channel7_IRQHandler
        PUBWEAK COMP1_IRQHandler
        PUBWEAK LPTIM2_IRQHandler
        PUBWEAK LPTIM3_IRQHandler
        PUBWEAK LPTIM4_IRQHandler
        PUBWEAK LPTIM5_IRQHandler
        PUBWEAK LPUART1_IRQHandler
        PUBWEAK CRS_IRQHandler
        PUBWEAK SAI4_IRQHandler
        PUBWEAK WAKEUP_PIN_IRQHandler
        PUBWEAK Reserved42_IRQHandler
        PUBWEAK Reserved64_IRQHandler
        PUBWEAK Reserved65_IRQHandler
        PUBWEAK Reserved66_IRQHandler
        PUBWEAK Reserved67_IRQHandler
        PUBWEAK Reserved79_IRQHandler
        PUBWEAK Reserved124_IRQHandler
        PUBWEAK Reserved123_IRQHandler
        PUBWEAK Reserved126_IRQHandler
        PUBWEAK Reserved143_IRQHandler
        PUBWEAK Reserved145_IRQHandler
        PUBWEAK Reserved147_IRQHandler
        PUBWEAK Reserved148_IRQHandler

Default_Handler
WWDG_IRQHandler
PVD_AVD_IRQHandler
TAMP_STAMP_IRQHandler
RTC_WKUP_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA1_Stream0_IRQHandler
DMA1_Stream1_IRQHandler
DMA1_Stream2_IRQHandler
DMA1_Stream3_IRQHandler
DMA1_Stream4_IRQHandler
DMA1_Stream5_IRQHandler
DMA1_Stream6_IRQHandler
ADC_IRQHandler
FDCAN1_IT0_IRQHandler
FDCAN2_IT0_IRQHandler
FDCAN1_IT1_IRQHandler
FDCAN2_IT1_IRQHandler
EXTI9_5_IRQHandler
TIM1_BRK_IRQHandler
TIM1_UP_IRQHandler
TIM1_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
I2C2_EV_IRQHandler
I2C2_ER_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
USART3_IRQHandler
EXTI15_10_IRQHandler
RTC_Alarm_IRQHandler
TIM8_BRK_TIM12_IRQHandler
TIM8_UP_TIM13_IRQHandler
TIM8_TRG_COM_TIM14_IRQHandler
TIM8_CC_IRQHandler
DMA1_Stream7_IRQHandler
FMC_IRQHandler
SDMMC1_IRQHandler
TIM5_IRQHandler
SPI3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
TIM6_DAC_IRQHandler
TIM7_IRQHandler
DMA2_Stream0_IRQHandler
DMA2_Stream1_IRQHandler
DMA2_Stream2_IRQHandler
DMA2_Stream3_IRQHandler
DMA2_Stream4_IRQHandler
ETH_IRQHandler
ETH_WKUP_IRQHandler
FDCAN_CAL_IRQHandler
DMA2_Stream5_IRQHandler
DMA2_Stream6_IRQHandler
DMA2_Stream7_IRQHandler
USART6_IRQHandler
I2C3_EV_IRQHandler
I2C3_ER_IRQHandler
OTG_HS_EP1_OUT_IRQHandler
OTG_HS_EP1_IN_IRQHandler
OTG_HS_WKUP_IRQHandler
OTG_HS_IRQHandler
DCMI_IRQHandler
RNG_IRQHandler
FPU_IRQHandler
UART7_IRQHandler
UART8_IRQHandler
SPI4_IRQHandler
SPI5_IRQHandler
SPI6_IRQHandler
SAI1_IRQHandler
LTDC_IRQHandler
LTDC_ER_IRQHandler
DMA2D_IRQHandler
SAI2_IRQHandler
QUADSPI_IRQHandler
LPTIM1_IRQHandler
CEC_IRQHandler
I2C4_EV_IRQHandler
I2C4_ER_IRQHandler
SPDIF_RX_IRQHandler
OTG_FS_EP1_OUT_IRQHandler
OTG_FS_EP1_IN_IRQHandler
OTG_FS_WKUP_IRQHandler
OTG_FS_IRQHandler
DMAMUX1_OVR_IRQHandler
HRTIM1_Master_IRQHandler
HRTIM1_TIMA_IRQHandler
HRTIM1_TIMB_IRQHandler
HRTIM1_TIMC_IRQHandler
HRTIM1_TIMD_IRQHandler
HRTIM1_TIME_IRQHandler
HRTIM1_FLT_IRQHandler
DFSDM1_FLT0_IRQHandler
DFSDM1_FLT1_IRQHandler
DFSDM1_FLT2_IRQHandler
DFSDM1_FLT3_IRQHandler
SAI3_IRQHandler
SWPMI1_IRQHandler
TIM15_IRQHandler
TIM16_IRQHandler
TIM17_IRQHandler
MDIOS_WKUP_IRQHandler
MDIOS_IRQHandler
JPEG_IRQHandler
MDMA_IRQHandler
SDMMC2_IRQHandler
HSEM1_IRQHandler
ADC3_IRQHandler
DMAMUX2_OVR_IRQHandler
BDMA_Channel0_IRQHandler
BDMA_Channel1_IRQHandler
BDMA_Channel2_IRQHandler
BDMA_Channel3_IRQHandler
BDMA_Channel4_IRQHandler
BDMA_Channel5_IRQHandler
BDMA_Channel6_IRQHandler
BDMA_Channel7_IRQHandler
COMP1_IRQHandler
LPTIM2_IRQHandler
LPTIM3_IRQHandler
LPTIM4_IRQHandler
LPTIM5_IRQHandler
LPUART1_IRQHandler
CRS_IRQHandler
SAI4_IRQHandler
WAKEUP_PIN_IRQHandler
Reserved42_IRQHandler
Reserved64_IRQHandler
Reserved65_IRQHandler
Reserved66_IRQHandler
Reserved67_IRQHandler
Reserved79_IRQHandler
Reserved124_IRQHandler
Reserved123_IRQHandler
Reserved126_IRQHandler
Reserved143_IRQHandler
Reserved145_IRQHandler
Reserved147_IRQHandler
Reserved148_IRQHandler
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGNROM 2

        END                     ; end of module

