;/***************************************************************************/
; * @file     startup_stm32h743xx.s for ARM-KEIL ARM assembler
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
; * @note
; * The symbols Stack_Size and Heap_Size should be provided on the command-
; * line options to the assembler, for example as:
; *     --pd "Stack_Size SETA 1024" --pd "Heap_Size SETA 0"


;******************************************************************************
; Allocate space for the stack.
;
        AREA    STACK, NOINIT, READWRITE, ALIGN=3
__stack_base
StackMem
        SPACE   Stack_Size    ; provided in command-line option, for example:
                              ; --pd "Stack_Size SETA 512"
__stack_limit
__initial_sp

;******************************************************************************
; Allocate space for the heap.
;
        AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
HeapMem
        SPACE   Heap_Size     ; provided in command-line option, for example:
                              ; --pd "Heap_Size SETA 0"
__heap_limit

; Indicate that the code in this file preserves 8-byte alignment of the stack.
        PRESERVE8

;******************************************************************************
; The vector table.
;
; Place code into the reset code section.
        AREA   RESET, DATA, READONLY, ALIGN=8
        EXPORT  __Vectors
        EXPORT  __Vectors_End
        EXPORT  __Vectors_Size

__Vectors
    ; Initial Vector Table before relocation
        DCD     __initial_sp                ; Top of Stack
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

__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        AREA    |.text|, CODE, READONLY

;******************************************************************************
; This is the code that gets called when the processor first starts execution
; following a reset event.
;
Reset_Handler   PROC
        EXPORT  Reset_Handler  [WEAK]
        IMPORT  SystemInit
        IMPORT  __main
        IMPORT  assert_failed

        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; Call the C library enty point that handles startup. This will copy
        ; the .data section initializers from flash to SRAM and zero fill the
        ; .bss section.
        ; NOTE: The __main function clears the C stack as well
        LDR     r0,=__main
        BX      r0

        ; __main calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGN
        ENDP

;******************************************************************************
NMI_Handler     PROC
        EXPORT  NMI_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGN
        ENDP

;******************************************************************************
HardFault_Handler PROC
        EXPORT  HardFault_Handler [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGN
        ENDP

;******************************************************************************
MemManage_Handler PROC
        EXPORT  MemManage_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGN
        ENDP

;******************************************************************************
BusFault_Handler PROC
        EXPORT  BusFault_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGN
        ENDP

;******************************************************************************
UsageFault_Handler PROC
        EXPORT  UsageFault_Handler   [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGN
        ENDP


;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
SVC_Handler PROC
        EXPORT  SVC_Handler   [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGN
        ENDP

;******************************************************************************
DebugMon_Handler PROC
        EXPORT  DebugMon_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGN
        ENDP

;******************************************************************************
PendSV_Handler PROC
        EXPORT  PendSV_Handler       [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGN
        ENDP

;******************************************************************************
SysTick_Handler PROC
        EXPORT  SysTick_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGN
        ENDP

;******************************************************************************
Default_Handler PROC
        EXPORT  WWDG_IRQHandler               [WEAK]
        EXPORT  PVD_AVD_IRQHandler            [WEAK]
        EXPORT  TAMP_STAMP_IRQHandler         [WEAK]
        EXPORT  RTC_WKUP_IRQHandler           [WEAK]
        EXPORT  FLASH_IRQHandler              [WEAK]
        EXPORT  RCC_IRQHandler                [WEAK]
        EXPORT  EXTI0_IRQHandler              [WEAK]
        EXPORT  EXTI1_IRQHandler              [WEAK]
        EXPORT  EXTI2_IRQHandler              [WEAK]
        EXPORT  EXTI3_IRQHandler              [WEAK]
        EXPORT  EXTI4_IRQHandler              [WEAK]
        EXPORT  DMA1_Stream0_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream1_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream2_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream3_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream4_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream5_IRQHandler       [WEAK]
        EXPORT  DMA1_Stream6_IRQHandler       [WEAK]
        EXPORT  ADC_IRQHandler                [WEAK]
        EXPORT  FDCAN1_IT0_IRQHandler         [WEAK]
        EXPORT  FDCAN2_IT0_IRQHandler         [WEAK]
        EXPORT  FDCAN1_IT1_IRQHandler         [WEAK]
        EXPORT  FDCAN2_IT1_IRQHandler         [WEAK]
        EXPORT  EXTI9_5_IRQHandler            [WEAK]
        EXPORT  TIM1_BRK_IRQHandler           [WEAK]
        EXPORT  TIM1_UP_IRQHandler            [WEAK]
        EXPORT  TIM1_TRG_COM_IRQHandler       [WEAK]
        EXPORT  TIM1_CC_IRQHandler            [WEAK]
        EXPORT  TIM2_IRQHandler               [WEAK]
        EXPORT  TIM3_IRQHandler               [WEAK]
        EXPORT  TIM4_IRQHandler               [WEAK]
        EXPORT  I2C1_EV_IRQHandler            [WEAK]
        EXPORT  I2C1_ER_IRQHandler            [WEAK]
        EXPORT  I2C2_EV_IRQHandler            [WEAK]
        EXPORT  I2C2_ER_IRQHandler            [WEAK]
        EXPORT  SPI1_IRQHandler               [WEAK]
        EXPORT  SPI2_IRQHandler               [WEAK]
        EXPORT  USART1_IRQHandler             [WEAK]
        EXPORT  USART2_IRQHandler             [WEAK]
        EXPORT  USART3_IRQHandler             [WEAK]
        EXPORT  EXTI15_10_IRQHandler          [WEAK]
        EXPORT  RTC_Alarm_IRQHandler          [WEAK]
        EXPORT  TIM8_BRK_TIM12_IRQHandler     [WEAK]
        EXPORT  TIM8_UP_TIM13_IRQHandler      [WEAK]
        EXPORT  TIM8_TRG_COM_TIM14_IRQHandler [WEAK]
        EXPORT  TIM8_CC_IRQHandler            [WEAK]
        EXPORT  DMA1_Stream7_IRQHandler       [WEAK]
        EXPORT  FMC_IRQHandler                [WEAK]
        EXPORT  SDMMC1_IRQHandler             [WEAK]
        EXPORT  TIM5_IRQHandler               [WEAK]
        EXPORT  SPI3_IRQHandler               [WEAK]
        EXPORT  UART4_IRQHandler              [WEAK]
        EXPORT  UART5_IRQHandler              [WEAK]
        EXPORT  TIM6_DAC_IRQHandler           [WEAK]
        EXPORT  TIM7_IRQHandler               [WEAK]
        EXPORT  DMA2_Stream0_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream1_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream2_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream3_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream4_IRQHandler       [WEAK]
        EXPORT  ETH_IRQHandler                [WEAK]
        EXPORT  ETH_WKUP_IRQHandler           [WEAK]
        EXPORT  FDCAN_CAL_IRQHandler          [WEAK]
        EXPORT  DMA2_Stream5_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream6_IRQHandler       [WEAK]
        EXPORT  DMA2_Stream7_IRQHandler       [WEAK]
        EXPORT  USART6_IRQHandler             [WEAK]
        EXPORT  I2C3_EV_IRQHandler            [WEAK]
        EXPORT  I2C3_ER_IRQHandler            [WEAK]
        EXPORT  OTG_HS_EP1_OUT_IRQHandler     [WEAK]
        EXPORT  OTG_HS_EP1_IN_IRQHandler      [WEAK]
        EXPORT  OTG_HS_WKUP_IRQHandler        [WEAK]
        EXPORT  OTG_HS_IRQHandler             [WEAK]
        EXPORT  DCMI_IRQHandler               [WEAK]
        EXPORT  RNG_IRQHandler                [WEAK]
        EXPORT  FPU_IRQHandler                [WEAK]
        EXPORT  UART7_IRQHandler              [WEAK]
        EXPORT  UART8_IRQHandler              [WEAK]
        EXPORT  SPI4_IRQHandler               [WEAK]
        EXPORT  SPI5_IRQHandler               [WEAK]
        EXPORT  SPI6_IRQHandler               [WEAK]
        EXPORT  SAI1_IRQHandler               [WEAK]
        EXPORT  LTDC_IRQHandler               [WEAK]
        EXPORT  LTDC_ER_IRQHandler            [WEAK]
        EXPORT  DMA2D_IRQHandler              [WEAK]
        EXPORT  SAI2_IRQHandler               [WEAK]
        EXPORT  QUADSPI_IRQHandler            [WEAK]
        EXPORT  LPTIM1_IRQHandler             [WEAK]
        EXPORT  CEC_IRQHandler                [WEAK]
        EXPORT  I2C4_EV_IRQHandler            [WEAK]
        EXPORT  I2C4_ER_IRQHandler            [WEAK]
        EXPORT  SPDIF_RX_IRQHandler           [WEAK]
        EXPORT  OTG_FS_EP1_OUT_IRQHandler     [WEAK]
        EXPORT  OTG_FS_EP1_IN_IRQHandler      [WEAK]
        EXPORT  OTG_FS_WKUP_IRQHandler        [WEAK]
        EXPORT  OTG_FS_IRQHandler             [WEAK]
        EXPORT  DMAMUX1_OVR_IRQHandler        [WEAK]
        EXPORT  HRTIM1_Master_IRQHandler      [WEAK]
        EXPORT  HRTIM1_TIMA_IRQHandler        [WEAK]
        EXPORT  HRTIM1_TIMB_IRQHandler        [WEAK]
        EXPORT  HRTIM1_TIMC_IRQHandler        [WEAK]
        EXPORT  HRTIM1_TIMD_IRQHandler        [WEAK]
        EXPORT  HRTIM1_TIME_IRQHandler        [WEAK]
        EXPORT  HRTIM1_FLT_IRQHandler         [WEAK]
        EXPORT  DFSDM1_FLT0_IRQHandler        [WEAK]
        EXPORT  DFSDM1_FLT1_IRQHandler        [WEAK]
        EXPORT  DFSDM1_FLT2_IRQHandler        [WEAK]
        EXPORT  DFSDM1_FLT3_IRQHandler        [WEAK]
        EXPORT  SAI3_IRQHandler               [WEAK]
        EXPORT  SWPMI1_IRQHandler             [WEAK]
        EXPORT  TIM15_IRQHandler              [WEAK]
        EXPORT  TIM16_IRQHandler              [WEAK]
        EXPORT  TIM17_IRQHandler              [WEAK]
        EXPORT  MDIOS_WKUP_IRQHandler         [WEAK]
        EXPORT  MDIOS_IRQHandler              [WEAK]
        EXPORT  JPEG_IRQHandler               [WEAK]
        EXPORT  MDMA_IRQHandler               [WEAK]
        EXPORT  SDMMC2_IRQHandler             [WEAK]
        EXPORT  HSEM1_IRQHandler              [WEAK]
        EXPORT  ADC3_IRQHandler               [WEAK]
        EXPORT  DMAMUX2_OVR_IRQHandler        [WEAK]
        EXPORT  BDMA_Channel0_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel1_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel2_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel3_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel4_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel5_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel6_IRQHandler      [WEAK]
        EXPORT  BDMA_Channel7_IRQHandler      [WEAK]
        EXPORT  COMP1_IRQHandler              [WEAK]
        EXPORT  LPTIM2_IRQHandler             [WEAK]
        EXPORT  LPTIM3_IRQHandler             [WEAK]
        EXPORT  LPTIM4_IRQHandler             [WEAK]
        EXPORT  LPTIM5_IRQHandler             [WEAK]
        EXPORT  LPUART1_IRQHandler            [WEAK]
        EXPORT  CRS_IRQHandler                [WEAK]
        EXPORT  SAI4_IRQHandler               [WEAK]
        EXPORT  WAKEUP_PIN_IRQHandler         [WEAK]
        EXPORT  Reserved42_IRQHandler         [WEAK]
        EXPORT  Reserved64_IRQHandler         [WEAK]
        EXPORT  Reserved65_IRQHandler         [WEAK]
        EXPORT  Reserved66_IRQHandler         [WEAK]
        EXPORT  Reserved67_IRQHandler         [WEAK]
        EXPORT  Reserved79_IRQHandler         [WEAK]
        EXPORT  Reserved124_IRQHandler        [WEAK]
        EXPORT  Reserved123_IRQHandler        [WEAK]
        EXPORT  Reserved126_IRQHandler        [WEAK]
        EXPORT  Reserved143_IRQHandler        [WEAK]
        EXPORT  Reserved145_IRQHandler        [WEAK]
        EXPORT  Reserved147_IRQHandler        [WEAK]
        EXPORT  Reserved148_IRQHandler        [WEAK]

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
        IMPORT  assert_failed
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGN
        ENDP

        ALIGN               ; make sure the end of this section is aligned

;******************************************************************************
; The function expected of the C library startup code for defining the stack
; and heap memory locations.  For the C library version of the startup code,
; provide this function so that the C library initialization code can find out
; the location of the stack and heap.
;
    IF :DEF: __MICROLIB
        EXPORT  __initial_sp
        EXPORT  __stack_limit
        EXPORT  __heap_base
        EXPORT  __heap_limit
    ELSE
        IMPORT  __use_two_region_memory
        EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
        LDR     R0, =__heap_base
        LDR     R1, =__stack_limit
        LDR     R2, =__heap_limit
        LDR     R3, =__stack_base
        BX      LR
        ENDP
    ENDIF
        ALIGN               ; make sure the end of this section is aligned

    END                     ; end of module

