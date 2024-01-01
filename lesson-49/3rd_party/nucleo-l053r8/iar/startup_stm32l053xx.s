;/***************************************************************************/
; * @file     startup_startup_stm32l053xx.s for IAR ARM assembler
; * @brief    CMSIS Cortex-M0+ Core Device Startup File for STM32L053xx
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
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
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
        DCD     WWDG_IRQHandler             ; [ 0] Window Watchdog
        DCD     PVD_IRQHandler              ; [ 1] PVD through EXTI Line detect
        DCD     RTC_IRQHandler              ; [ 2] RTC through EXTI Line
        DCD     FLASH_IRQHandler            ; [ 3] FLASH
        DCD     RCC_CRS_IRQHandler          ; [ 4] RCC and CRS
        DCD     EXTI0_1_IRQHandler          ; [ 5] EXTI Line 0 and 1
        DCD     EXTI2_3_IRQHandler          ; [ 6] EXTI Line 2 and 3
        DCD     EXTI4_15_IRQHandler         ; [ 7] EXTI Line 4 to 15
        DCD     TSC_IRQHandler              ; [ 8] TSC
        DCD     DMA1_Channel1_IRQHandler    ; [ 9] DMA1 Channel 1
        DCD     DMA1_Channel2_3_IRQHandler  ; [10] DMA1 Channel 2 and Channel 3
        DCD     DMA1_Channel4_5_6_7_IRQHandler ; [11] DMA1 Channel 4, 5, 6 and 7
        DCD     ADC1_COMP_IRQHandler        ; [12] ADC1, COMP1 and COMP2
        DCD     LPTIM1_IRQHandler           ; [13] LPTIM1
        DCD     Reserved14_IRQHandler       ; [14] Reserved
        DCD     TIM2_IRQHandler             ; [15] TIM2
        DCD     Reserved16_IRQHandler       ; [16] Reserved
        DCD     TIM6_DAC_IRQHandler         ; [17] TIM6 and DAC
        DCD     Reserved18_IRQHandler       ; [18] Reserved
        DCD     Reserved19_IRQHandler       ; [19] Reserved
        DCD     TIM21_IRQHandler            ; [20] TIM21
        DCD     Reserved21_IRQHandler       ; [21] Reserved
        DCD     TIM22_IRQHandler            ; [22] TIM22
        DCD     I2C1_IRQHandler             ; [23] I2C1
        DCD     I2C2_IRQHandler             ; [24] I2C2
        DCD     SPI1_IRQHandler             ; [25] SPI1
        DCD     SPI2_IRQHandler             ; [26] SPI2
        DCD     USART1_IRQHandler           ; [27] USART1
        DCD     USART2_IRQHandler           ; [28] USART2
        DCD     RNG_LPUART1_IRQHandler      ; [29] RNG and LPUART1
        DCD     LCD_IRQHandler              ; [30] LCD
        DCD     USB_IRQHandler              ; [31] USB

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
        PUBWEAK  Default_Handler
        PUBWEAK  WWDG_IRQHandler
        PUBWEAK  PVD_IRQHandler
        PUBWEAK  RTC_IRQHandler
        PUBWEAK  FLASH_IRQHandler
        PUBWEAK  RCC_CRS_IRQHandler
        PUBWEAK  EXTI0_1_IRQHandler
        PUBWEAK  EXTI2_3_IRQHandler
        PUBWEAK  EXTI4_15_IRQHandler
        PUBWEAK  TSC_IRQHandler
        PUBWEAK  DMA1_Channel1_IRQHandler
        PUBWEAK  DMA1_Channel2_3_IRQHandler
        PUBWEAK  DMA1_Channel4_5_6_7_IRQHandler
        PUBWEAK  ADC1_COMP_IRQHandler
        PUBWEAK  LPTIM1_IRQHandler
        PUBWEAK  TIM2_IRQHandler
        PUBWEAK  TIM6_DAC_IRQHandler
        PUBWEAK  TIM21_IRQHandler
        PUBWEAK  TIM22_IRQHandler
        PUBWEAK  I2C1_IRQHandler
        PUBWEAK  I2C2_IRQHandler
        PUBWEAK  SPI1_IRQHandler
        PUBWEAK  SPI2_IRQHandler
        PUBWEAK  USART1_IRQHandler
        PUBWEAK  USART2_IRQHandler
        PUBWEAK  RNG_LPUART1_IRQHandler
        PUBWEAK  LCD_IRQHandler
        PUBWEAK  USB_IRQHandler
        PUBWEAK  Reserved14_IRQHandler
        PUBWEAK  Reserved16_IRQHandler
        PUBWEAK  Reserved18_IRQHandler
        PUBWEAK  Reserved19_IRQHandler
        PUBWEAK  Reserved21_IRQHandler

Default_Handler
WWDG_IRQHandler
PVD_IRQHandler
RTC_IRQHandler
FLASH_IRQHandler
RCC_CRS_IRQHandler
EXTI0_1_IRQHandler
EXTI2_3_IRQHandler
EXTI4_15_IRQHandler
TSC_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_3_IRQHandler
DMA1_Channel4_5_6_7_IRQHandler
ADC1_COMP_IRQHandler
LPTIM1_IRQHandler
TIM2_IRQHandler
TIM6_DAC_IRQHandler
TIM21_IRQHandler
TIM22_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
RNG_LPUART1_IRQHandler
LCD_IRQHandler
USB_IRQHandler
Reserved14_IRQHandler
Reserved16_IRQHandler
Reserved18_IRQHandler
Reserved19_IRQHandler
Reserved21_IRQHandler
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

