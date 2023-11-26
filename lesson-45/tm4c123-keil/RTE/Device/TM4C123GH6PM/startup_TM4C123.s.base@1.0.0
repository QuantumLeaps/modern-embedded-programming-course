;/**************************************************************************//**
; * @file     startup_TM4C123.s
; * @brief    CMSIS Cortex-M4 Core Device Startup File for
; *           TI Tiva TM4C123 Blizzard Class Device
; * @version  V1.00
; * @date     15. May 2013
; *
; * @note
; * Copyright (C) 2011 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts

                DCD     GPIOA_Handler             ;   0: GPIO Port A
                DCD     GPIOB_Handler             ;   1: GPIO Port B
                DCD     GPIOC_Handler             ;   2: GPIO Port C
                DCD     GPIOD_Handler             ;   3: GPIO Port D
                DCD     GPIOE_Handler             ;   4: GPIO Port E
                DCD     UART0_Handler             ;   5: UART0 Rx and Tx
                DCD     UART1_Handler             ;   6: UART1 Rx and Tx
                DCD     SSI0_Handler              ;   7: SSI0 Rx and Tx
                DCD     I2C0_Handler              ;   8: I2C0 Master and Slave
                DCD     PMW0_FAULT_Handler        ;   9: PWM Fault
                DCD     PWM0_0_Handler            ;  10: PWM Generator 0
                DCD     PWM0_1_Handler            ;  11: PWM Generator 1
                DCD     PWM0_2_Handler            ;  12: PWM Generator 2
                DCD     QEI0_Handler              ;  13: Quadrature Encoder 0
                DCD     ADC0SS0_Handler           ;  14: ADC Sequence 0
                DCD     ADC0SS1_Handler           ;  15: ADC Sequence 1
                DCD     ADC0SS2_Handler           ;  16: ADC Sequence 2
                DCD     ADC0SS3_Handler           ;  17: ADC Sequence 3
                DCD     WDT0_Handler              ;  18: Watchdog timer
                DCD     TIMER0A_Handler           ;  19: Timer 0 subtimer A
                DCD     TIMER0B_Handler           ;  20: Timer 0 subtimer B
                DCD     TIMER1A_Handler           ;  21: Timer 1 subtimer A
                DCD     TIMER1B_Handler           ;  22: Timer 1 subtimer B
                DCD     TIMER2A_Handler           ;  23: Timer 2 subtimer A
                DCD     TIMER2B_Handler           ;  24: Timer 2 subtimer B
                DCD     COMP0_Handler             ;  25: Analog Comparator 0
                DCD     COMP1_Handler             ;  26: Analog Comparator 1
                DCD     COMP2_Handler             ;  27: Analog Comparator 2
                DCD     SYSCTL_Handler            ;  28: System Control (PLL, OSC, BO)
                DCD     FLASH_Handler             ;  29: FLASH Control
                DCD     GPIOF_Handler             ;  30: GPIO Port F
                DCD     GPIOG_Handler             ;  31: GPIO Port G
                DCD     GPIOH_Handler             ;  32: GPIO Port H
                DCD     UART2_Handler             ;  33: UART2 Rx and Tx
                DCD     SSI1_Handler              ;  34: SSI1 Rx and Tx
                DCD     TIMER3A_Handler           ;  35: Timer 3 subtimer A
                DCD     TIMER3B_Handler           ;  36: Timer 3 subtimer B
                DCD     I2C1_Handler              ;  37: I2C1 Master and Slave
                DCD     QEI1_Handler              ;  38: Quadrature Encoder 1
                DCD     CAN0_Handler              ;  39: CAN0
                DCD     CAN1_Handler              ;  40: CAN1
                DCD     CAN2_Handler              ;  41: CAN2
                DCD     0                         ;  42: Reserved
                DCD     HIB_Handler               ;  43: Hibernate
                DCD     USB0_Handler              ;  44: USB0
                DCD     PWM0_3_Handler            ;  45: PWM Generator 3
                DCD     UDMA_Handler              ;  46: uDMA Software Transfer
                DCD     UDMAERR_Handler           ;  47: uDMA Error
                DCD     ADC1SS0_Handler           ;  48: ADC1 Sequence 0
                DCD     ADC1SS1_Handler           ;  49: ADC1 Sequence 1
                DCD     ADC1SS2_Handler           ;  50: ADC1 Sequence 2
                DCD     ADC1SS3_Handler           ;  51: ADC1 Sequence 3
                DCD     0                         ;  52: Reserved
                DCD     0                         ;  53: Reserved
                DCD     GPIOJ_Handler             ;  54: GPIO Port J
                DCD     GPIOK_Handler             ;  55: GPIO Port K
                DCD     GPIOL_Handler             ;  56: GPIO Port L
                DCD     SSI2_Handler              ;  57: SSI2 Rx and Tx
                DCD     SSI3_Handler              ;  58: SSI3 Rx and Tx
                DCD     UART3_Handler             ;  59: UART3 Rx and Tx
                DCD     UART4_Handler             ;  60: UART4 Rx and Tx
                DCD     UART5_Handler             ;  61: UART5 Rx and Tx
                DCD     UART6_Handler             ;  62: UART6 Rx and Tx
                DCD     UART7_Handler             ;  63: UART7 Rx and Tx
                DCD     0                         ;  64: Reserved
                DCD     0                         ;  65: Reserved
                DCD     0                         ;  66: Reserved
                DCD     0                         ;  67: Reserved
                DCD     I2C2_Handler              ;  68: I2C2 Master and Slave
                DCD     I2C3_Handler              ;  69: I2C3 Master and Slave
                DCD     TIMER4A_Handler           ;  70: Timer 4 subtimer A
                DCD     TIMER4B_Handler           ;  71: Timer 4 subtimer B
                DCD     0                         ;  72: Reserved
                DCD     0                         ;  73: Reserved
                DCD     0                         ;  74: Reserved
                DCD     0                         ;  75: Reserved
                DCD     0                         ;  76: Reserved
                DCD     0                         ;  77: Reserved
                DCD     0                         ;  78: Reserved
                DCD     0                         ;  79: Reserved
                DCD     0                         ;  80: Reserved
                DCD     0                         ;  81: Reserved
                DCD     0                         ;  82: Reserved
                DCD     0                         ;  83: Reserved
                DCD     0                         ;  84: Reserved
                DCD     0                         ;  85: Reserved
                DCD     0                         ;  86: Reserved
                DCD     0                         ;  87: Reserved
                DCD     0                         ;  88: Reserved
                DCD     0                         ;  89: Reserved
                DCD     0                         ;  90: Reserved
                DCD     0                         ;  91: Reserved
                DCD     TIMER5A_Handler           ;  92: Timer 5 subtimer A
                DCD     TIMER5B_Handler           ;  93: Timer 5 subtimer B
                DCD     WTIMER0A_Handler          ;  94: Wide Timer 0 subtimer A
                DCD     WTIMER0B_Handler          ;  95: Wide Timer 0 subtimer B
                DCD     WTIMER1A_Handler          ;  96: Wide Timer 1 subtimer A
                DCD     WTIMER1B_Handler          ;  97: Wide Timer 1 subtimer B
                DCD     WTIMER2A_Handler          ;  98: Wide Timer 2 subtimer A
                DCD     WTIMER2B_Handler          ;  99: Wide Timer 2 subtimer B
                DCD     WTIMER3A_Handler          ; 100: Wide Timer 3 subtimer A
                DCD     WTIMER3B_Handler          ; 101: Wide Timer 3 subtimer B
                DCD     WTIMER4A_Handler          ; 102: Wide Timer 4 subtimer A
                DCD     WTIMER4B_Handler          ; 103: Wide Timer 4 subtimer B
                DCD     WTIMER5A_Handler          ; 104: Wide Timer 5 subtimer A
                DCD     WTIMER5B_Handler          ; 105: Wide Timer 5 subtimer B
                DCD     FPU_Handler               ; 106: FPU
                DCD     0                         ; 107: Reserved
                DCD     0                         ; 108: Reserved
                DCD     I2C4_Handler              ; 109: I2C4 Master and Slave
                DCD     I2C5_Handler              ; 110: I2C5 Master and Slave
                DCD     GPIOM_Handler             ; 111: GPIO Port M
                DCD     GPION_Handler             ; 112: GPIO Port N
                DCD     QEI2_Handler              ; 113: Quadrature Encoder 2
                DCD     0                         ; 114: Reserved
                DCD     0                         ; 115: Reserved
                DCD     GPIOP0_Handler            ; 116: GPIO Port P (Summary or P0)
                DCD     GPIOP1_Handler            ; 117: GPIO Port P1
                DCD     GPIOP2_Handler            ; 118: GPIO Port P2
                DCD     GPIOP3_Handler            ; 119: GPIO Port P3
                DCD     GPIOP4_Handler            ; 120: GPIO Port P4
                DCD     GPIOP5_Handler            ; 121: GPIO Port P5
                DCD     GPIOP6_Handler            ; 122: GPIO Port P6
                DCD     GPIOP7_Handler            ; 123: GPIO Port P7
                DCD     GPIOQ0_Handler            ; 124: GPIO Port Q (Summary or Q0)
                DCD     GPIOQ1_Handler            ; 125: GPIO Port Q1
                DCD     GPIOQ2_Handler            ; 126: GPIO Port Q2
                DCD     GPIOQ3_Handler            ; 127: GPIO Port Q3
                DCD     GPIOQ4_Handler            ; 128: GPIO Port Q4
                DCD     GPIOQ5_Handler            ; 129: GPIO Port Q5
                DCD     GPIOQ6_Handler            ; 130: GPIO Port Q6
                DCD     GPIOQ7_Handler            ; 131: GPIO Port Q7
                DCD     GPIOR_Handler             ; 132: GPIO Port R
                DCD     GPIOS_Handler             ; 133: GPIO Port S
                DCD     PMW1_0_Handler            ; 134: PWM 1 Generator 0
                DCD     PWM1_1_Handler            ; 135: PWM 1 Generator 1
                DCD     PWM1_2_Handler            ; 136: PWM 1 Generator 2
                DCD     PWM1_3_Handler            ; 137: PWM 1 Generator 3
                DCD     PWM1_FAULT_Handler        ; 138: PWM 1 Fault

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

GPIOA_Handler\
                PROC
                EXPORT  GPIOA_Handler [WEAK]
                B       .
                ENDP

GPIOB_Handler\
                PROC
                EXPORT  GPIOB_Handler [WEAK]
                B       .
                ENDP

GPIOC_Handler\
                PROC
                EXPORT  GPIOC_Handler [WEAK]
                B       .
                ENDP

GPIOD_Handler\
                PROC
                EXPORT  GPIOD_Handler [WEAK]
                B       .
                ENDP

GPIOE_Handler\
                PROC
                EXPORT  GPIOE_Handler [WEAK]
                B       .
                ENDP

UART0_Handler\
                PROC
                EXPORT  UART0_Handler [WEAK]
                B       .
                ENDP

UART1_Handler\
                PROC
                EXPORT  UART1_Handler [WEAK]
                B       .
                ENDP

SSI0_Handler\
                PROC
                EXPORT  SSI0_Handler [WEAK]
                B       .
                ENDP

I2C0_Handler\
                PROC
                EXPORT  I2C0_Handler [WEAK]
                B       .
                ENDP

PMW0_FAULT_Handler\
                PROC
                EXPORT  PMW0_FAULT_Handler [WEAK]
                B       .
                ENDP

PWM0_0_Handler\
                PROC
                EXPORT  PWM0_0_Handler [WEAK]
                B       .
                ENDP

PWM0_1_Handler\
                PROC
                EXPORT  PWM0_1_Handler [WEAK]
                B       .
                ENDP

PWM0_2_Handler\
                PROC
                EXPORT  PWM0_2_Handler [WEAK]
                B       .
                ENDP

QEI0_Handler\
                PROC
                EXPORT  QEI0_Handler [WEAK]
                B       .
                ENDP

ADC0SS0_Handler\
                PROC
                EXPORT  ADC0SS0_Handler [WEAK]
                B       .
                ENDP

ADC0SS1_Handler\
                PROC
                EXPORT  ADC0SS1_Handler [WEAK]
                B       .
                ENDP

ADC0SS2_Handler\
                PROC
                EXPORT  ADC0SS2_Handler [WEAK]
                B       .
                ENDP

ADC0SS3_Handler\
                PROC
                EXPORT  ADC0SS3_Handler [WEAK]
                B       .
                ENDP

WDT0_Handler\
                PROC
                EXPORT  WDT0_Handler [WEAK]
                B       .
                ENDP

TIMER0A_Handler\
                PROC
                EXPORT  TIMER0A_Handler [WEAK]
                B       .
                ENDP

TIMER0B_Handler\
                PROC
                EXPORT  TIMER0B_Handler [WEAK]
                B       .
                ENDP

TIMER1A_Handler\
                PROC
                EXPORT  TIMER1A_Handler [WEAK]
                B       .
                ENDP

TIMER1B_Handler\
                PROC
                EXPORT  TIMER1B_Handler [WEAK]
                B       .
                ENDP

TIMER2A_Handler\
                PROC
                EXPORT  TIMER2A_Handler [WEAK]
                B       .
                ENDP

TIMER2B_Handler\
                PROC
                EXPORT  TIMER2B_Handler [WEAK]
                B       .
                ENDP

COMP0_Handler\
                PROC
                EXPORT  COMP0_Handler [WEAK]
                B       .
                ENDP

COMP1_Handler\
                PROC
                EXPORT  COMP1_Handler [WEAK]
                B       .
                ENDP

COMP2_Handler\
                PROC
                EXPORT  COMP2_Handler [WEAK]
                B       .
                ENDP

SYSCTL_Handler\
                PROC
                EXPORT  SYSCTL_Handler [WEAK]
                B       .
                ENDP

FLASH_Handler\
                PROC
                EXPORT  FLASH_Handler [WEAK]
                B       .
                ENDP

GPIOF_Handler\
                PROC
                EXPORT  GPIOF_Handler [WEAK]
                B       .
                ENDP

GPIOG_Handler\
                PROC
                EXPORT  GPIOG_Handler [WEAK]
                B       .
                ENDP

GPIOH_Handler\
                PROC
                EXPORT  GPIOH_Handler [WEAK]
                B       .
                ENDP

UART2_Handler\
                PROC
                EXPORT  UART2_Handler [WEAK]
                B       .
                ENDP

SSI1_Handler\
                PROC
                EXPORT  SSI1_Handler [WEAK]
                B       .
                ENDP

TIMER3A_Handler\
                PROC
                EXPORT  TIMER3A_Handler [WEAK]
                B       .
                ENDP

TIMER3B_Handler\
                PROC
                EXPORT  TIMER3B_Handler [WEAK]
                B       .
                ENDP

I2C1_Handler\
                PROC
                EXPORT  I2C1_Handler [WEAK]
                B       .
                ENDP

QEI1_Handler\
                PROC
                EXPORT  QEI1_Handler [WEAK]
                B       .
                ENDP

CAN0_Handler\
                PROC
                EXPORT  CAN0_Handler [WEAK]
                B       .
                ENDP

CAN1_Handler\
                PROC
                EXPORT  CAN1_Handler [WEAK]
                B       .
                ENDP

CAN2_Handler\
                PROC
                EXPORT  CAN2_Handler [WEAK]
                B       .
                ENDP

HIB_Handler\
                PROC
                EXPORT  HIB_Handler [WEAK]
                B       .
                ENDP

USB0_Handler\
                PROC
                EXPORT  USB0_Handler [WEAK]
                B       .
                ENDP

PWM0_3_Handler\
                PROC
                EXPORT  PWM0_3_Handler [WEAK]
                B       .
                ENDP

UDMA_Handler\
                PROC
                EXPORT  UDMA_Handler [WEAK]
                B       .
                ENDP

UDMAERR_Handler\
                PROC
                EXPORT  UDMAERR_Handler [WEAK]
                B       .
                ENDP

ADC1SS0_Handler\
                PROC
                EXPORT  ADC1SS0_Handler [WEAK]
                B       .
                ENDP

ADC1SS1_Handler\
                PROC
                EXPORT  ADC1SS1_Handler [WEAK]
                B       .
                ENDP

ADC1SS2_Handler\
                PROC
                EXPORT  ADC1SS2_Handler [WEAK]
                B       .
                ENDP

ADC1SS3_Handler\
                PROC
                EXPORT  ADC1SS3_Handler [WEAK]
                B       .
                ENDP

GPIOJ_Handler\
                PROC
                EXPORT  GPIOJ_Handler [WEAK]
                B       .
                ENDP

GPIOK_Handler\
                PROC
                EXPORT  GPIOK_Handler [WEAK]
                B       .
                ENDP

GPIOL_Handler\
                PROC
                EXPORT  GPIOL_Handler [WEAK]
                B       .
                ENDP

SSI2_Handler\
                PROC
                EXPORT  SSI2_Handler [WEAK]
                B       .
                ENDP

SSI3_Handler\
                PROC
                EXPORT  SSI3_Handler [WEAK]
                B       .
                ENDP

UART3_Handler\
                PROC
                EXPORT  UART3_Handler [WEAK]
                B       .
                ENDP

UART4_Handler\
                PROC
                EXPORT  UART4_Handler [WEAK]
                B       .
                ENDP

UART5_Handler\
                PROC
                EXPORT  UART5_Handler [WEAK]
                B       .
                ENDP

UART6_Handler\
                PROC
                EXPORT  UART6_Handler [WEAK]
                B       .
                ENDP

UART7_Handler\
                PROC
                EXPORT  UART7_Handler [WEAK]
                B       .
                ENDP

I2C2_Handler\
                PROC
                EXPORT  I2C2_Handler [WEAK]
                B       .
                ENDP

I2C3_Handler\
                PROC
                EXPORT  I2C3_Handler [WEAK]
                B       .
                ENDP

TIMER4A_Handler\
                PROC
                EXPORT  TIMER4A_Handler [WEAK]
                B       .
                ENDP

TIMER4B_Handler\
                PROC
                EXPORT  TIMER4B_Handler [WEAK]
                B       .
                ENDP

TIMER5A_Handler\
                PROC
                EXPORT  TIMER5A_Handler [WEAK]
                B       .
                ENDP

TIMER5B_Handler\
                PROC
                EXPORT  TIMER5B_Handler [WEAK]
                B       .
                ENDP

WTIMER0A_Handler\
                PROC
                EXPORT  WTIMER0A_Handler [WEAK]
                B       .
                ENDP

WTIMER0B_Handler\
                PROC
                EXPORT  WTIMER0B_Handler [WEAK]
                B       .
                ENDP

WTIMER1A_Handler\
                PROC
                EXPORT  WTIMER1A_Handler [WEAK]
                B       .
                ENDP

WTIMER1B_Handler\
                PROC
                EXPORT  WTIMER1B_Handler [WEAK]
                B       .
                ENDP

WTIMER2A_Handler\
                PROC
                EXPORT  WTIMER2A_Handler [WEAK]
                B       .
                ENDP

WTIMER2B_Handler\
                PROC
                EXPORT  WTIMER2B_Handler [WEAK]
                B       .
                ENDP

WTIMER3A_Handler\
                PROC
                EXPORT  WTIMER3A_Handler [WEAK]
                B       .
                ENDP

WTIMER3B_Handler\
                PROC
                EXPORT  WTIMER3B_Handler [WEAK]
                B       .
                ENDP

WTIMER4A_Handler\
                PROC
                EXPORT  WTIMER4A_Handler [WEAK]
                B       .
                ENDP

WTIMER4B_Handler\
                PROC
                EXPORT  WTIMER4B_Handler [WEAK]
                B       .
                ENDP

WTIMER5A_Handler\
                PROC
                EXPORT  WTIMER5A_Handler [WEAK]
                B       .
                ENDP

WTIMER5B_Handler\
                PROC
                EXPORT  WTIMER5B_Handler [WEAK]
                B       .
                ENDP

FPU_Handler\
                PROC
                EXPORT  FPU_Handler [WEAK]
                B       .
                ENDP

I2C4_Handler\
                PROC
                EXPORT  I2C4_Handler [WEAK]
                B       .
                ENDP

I2C5_Handler\
                PROC
                EXPORT  I2C5_Handler [WEAK]
                B       .
                ENDP

GPIOM_Handler\
                PROC
                EXPORT  GPIOM_Handler [WEAK]
                B       .
                ENDP

GPION_Handler\
                PROC
                EXPORT  GPION_Handler [WEAK]
                B       .
                ENDP

QEI2_Handler\
                PROC
                EXPORT  QEI2_Handler [WEAK]
                B       .
                ENDP

GPIOP0_Handler\
                PROC
                EXPORT  GPIOP0_Handler [WEAK]
                B       .
                ENDP

GPIOP1_Handler\
                PROC
                EXPORT  GPIOP1_Handler [WEAK]
                B       .
                ENDP

GPIOP2_Handler\
                PROC
                EXPORT  GPIOP2_Handler [WEAK]
                B       .
                ENDP

GPIOP3_Handler\
                PROC
                EXPORT  GPIOP3_Handler [WEAK]
                B       .
                ENDP

GPIOP4_Handler\
                PROC
                EXPORT  GPIOP4_Handler [WEAK]
                B       .
                ENDP

GPIOP5_Handler\
                PROC
                EXPORT  GPIOP5_Handler [WEAK]
                B       .
                ENDP

GPIOP6_Handler\
                PROC
                EXPORT  GPIOP6_Handler [WEAK]
                B       .
                ENDP

GPIOP7_Handler\
                PROC
                EXPORT  GPIOP7_Handler [WEAK]
                B       .
                ENDP

GPIOQ0_Handler\
                PROC
                EXPORT  GPIOQ0_Handler [WEAK]
                B       .
                ENDP

GPIOQ1_Handler\
                PROC
                EXPORT  GPIOQ1_Handler [WEAK]
                B       .
                ENDP

GPIOQ2_Handler\
                PROC
                EXPORT  GPIOQ2_Handler [WEAK]
                B       .
                ENDP

GPIOQ3_Handler\
                PROC
                EXPORT  GPIOQ3_Handler [WEAK]
                B       .
                ENDP

GPIOQ4_Handler\
                PROC
                EXPORT  GPIOQ4_Handler [WEAK]
                B       .
                ENDP

GPIOQ5_Handler\
                PROC
                EXPORT  GPIOQ5_Handler [WEAK]
                B       .
                ENDP

GPIOQ6_Handler\
                PROC
                EXPORT  GPIOQ6_Handler [WEAK]
                B       .
                ENDP

GPIOQ7_Handler\
                PROC
                EXPORT  GPIOQ7_Handler [WEAK]
                B       .
                ENDP

GPIOR_Handler\
                PROC
                EXPORT  GPIOR_Handler [WEAK]
                B       .
                ENDP

GPIOS_Handler\
                PROC
                EXPORT  GPIOS_Handler [WEAK]
                B       .
                ENDP

PMW1_0_Handler\
                PROC
                EXPORT  PMW1_0_Handler [WEAK]
                B       .
                ENDP

PWM1_1_Handler\
                PROC
                EXPORT  PWM1_1_Handler [WEAK]
                B       .
                ENDP

PWM1_2_Handler\
                PROC
                EXPORT  PWM1_2_Handler [WEAK]
                B       .
                ENDP

PWM1_3_Handler\
                PROC
                EXPORT  PWM1_3_Handler [WEAK]
                B       .
                ENDP

PWM1_FAULT_Handler\
                PROC
                EXPORT  PWM1_FAULT_Handler [WEAK]
                B       .
                ENDP

                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
