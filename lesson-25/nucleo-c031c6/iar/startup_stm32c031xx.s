;/***************************************************************************/
; @file     startup_stm32c031xx.s for IAR ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for stm32c031xx
; @version  CMSIS 5.9.0
; @date     1 Feb 2023
;
; Modified by Quantum Leaps:
; - Added relocating of the Vector Table to free up the 256B region at 0x0
;   for NULL-pointer protection by the MPU.
; - Modified all exception handlers to branch to assert_failed()
;   instead of locking up the CPU inside an endless loop.
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
; The vector table
;
        DATA
__vector_table
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
        DCD     Reserved1_IRQHandler        ; [ 1] Reserved
        DCD     RTC_IRQHandler              ; [ 2] RTC through EXTI Line
        DCD     FLASH_IRQHandler            ; [ 3] FLASH
        DCD     RCC_IRQHandler              ; [ 4] RCC
        DCD     EXTI0_1_IRQHandler          ; [ 5] EXTI Line 0 and 1
        DCD     EXTI2_3_IRQHandler          ; [ 6] EXTI Line 2 and 3
        DCD     EXTI4_15_IRQHandler         ; [ 7] EXTI Line 4 to 15
        DCD     Reserved8_IRQHandler        ; [ 8] Reserved
        DCD     DMA1_Channel1_IRQHandler    ; [ 9] DMA1 Channel 1
        DCD     DMA1_Channel2_3_IRQHandler  ; [10] DMA1 Channel 2 and Channel 3
        DCD     DMAMUX1_IRQHandler          ; [11] DMAMUX
        DCD     ADC1_IRQHandler             ; [12] ADC1
        DCD     TIM1_BRK_UP_TRG_COM_IRQHandler ; [13] TIM1 Break, Update, Trigger and Commutation
        DCD     TIM1_CC_IRQHandler          ; [14] TIM1 Capture Compare
        DCD     Reserved15_IRQHandler       ; [15] Reserved
        DCD     TIM3_IRQHandler             ; [16] TIM3
        DCD     Reserved17_IRQHandler       ; [17] Reserved
        DCD     Reserved18_IRQHandler       ; [18] Reserved
        DCD     TIM14_IRQHandler            ; [19] TIM14
        DCD     Reserved20_IRQHandler       ; [20] Reserved
        DCD     TIM16_IRQHandler            ; [21] TIM16
        DCD     TIM17_IRQHandler            ; [22] TIM17
        DCD     I2C1_IRQHandler             ; [23] I2C1
        DCD     Reserved24_IRQHandler       ; [24] Reserved
        DCD     SPI1_IRQHandler             ; [25] SPI1
        DCD     Reserved26_IRQHandler       ; [26] Reserved
        DCD     USART1_IRQHandler           ; [27] USART1
        DCD     USART2_IRQHandler           ; [28] USART2
        DCD     Reserved29_IRQHandler       ; [29] Reserved
        DCD     Reserved30_IRQHandler       ; [30] Reserved
        DCD     Reserved31_IRQHandler       ; [31] Reserved

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        SECTION .text:CODE:REORDER:NOROOT(2)

;******************************************************************************
; This is the code that gets called when the CPU first starts execution
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        PUBWEAK  RTC_IRQHandler
        PUBWEAK  FLASH_IRQHandler
        PUBWEAK  RCC_IRQHandler
        PUBWEAK  EXTI0_1_IRQHandler
        PUBWEAK  EXTI2_3_IRQHandler
        PUBWEAK  EXTI4_15_IRQHandler
        PUBWEAK  DMA1_Channel1_IRQHandler
        PUBWEAK  DMA1_Channel2_3_IRQHandler
        PUBWEAK  DMAMUX1_IRQHandler
        PUBWEAK  ADC1_IRQHandler
        PUBWEAK  TIM1_BRK_UP_TRG_COM_IRQHandler
        PUBWEAK  TIM1_CC_IRQHandler
        PUBWEAK  TIM3_IRQHandler
        PUBWEAK  TIM14_IRQHandler
        PUBWEAK  TIM16_IRQHandler
        PUBWEAK  TIM17_IRQHandler
        PUBWEAK  I2C1_IRQHandler
        PUBWEAK  SPI1_IRQHandler
        PUBWEAK  USART1_IRQHandler
        PUBWEAK  USART2_IRQHandler
        PUBWEAK  Reserved1_IRQHandler
        PUBWEAK  Reserved8_IRQHandler
        PUBWEAK  Reserved15_IRQHandler
        PUBWEAK  Reserved17_IRQHandler
        PUBWEAK  Reserved18_IRQHandler
        PUBWEAK  Reserved20_IRQHandler
        PUBWEAK  Reserved24_IRQHandler
        PUBWEAK  Reserved26_IRQHandler
        PUBWEAK  Reserved29_IRQHandler
        PUBWEAK  Reserved30_IRQHandler
        PUBWEAK  Reserved31_IRQHandler

Default_Handler
WWDG_IRQHandler
RTC_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_1_IRQHandler
EXTI2_3_IRQHandler
EXTI4_15_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_3_IRQHandler
DMAMUX1_IRQHandler
ADC1_IRQHandler
TIM1_BRK_UP_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM3_IRQHandler
TIM14_IRQHandler
TIM16_IRQHandler
TIM17_IRQHandler
I2C1_IRQHandler
SPI1_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
Reserved1_IRQHandler
Reserved8_IRQHandler
Reserved15_IRQHandler
Reserved17_IRQHandler
Reserved18_IRQHandler
Reserved20_IRQHandler
Reserved24_IRQHandler
Reserved26_IRQHandler
Reserved29_IRQHandler
Reserved30_IRQHandler
Reserved31_IRQHandler
        CPSID   i                ; disable all interrupts
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGNROM 2

        END                      ; end of module

