;/***************************************************************************/
; @file     startup_TM4C123GH6PM.s for IAR ARM assembler
; @brief    CMSIS Cortex-M4F Core Device Startup File for TM4C123GH6PM
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
        ALIGNROM 8   ; Extend the initial Vector Table to the 2^8==256B

    ; Relocated Vector Table beyond the 256B region around address 0.
    ; That region is used for NULL-pointer protection by the MPU.
__relocated_vector_table
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
        DCD     GPIOPortA_IRQHandler        ; GPIO Port A
        DCD     GPIOPortB_IRQHandler        ; GPIO Port B
        DCD     GPIOPortC_IRQHandler        ; GPIO Port C
        DCD     GPIOPortD_IRQHandler        ; GPIO Port D
        DCD     GPIOPortE_IRQHandler        ; GPIO Port E
        DCD     UART0_IRQHandler            ; UART0 Rx and Tx
        DCD     UART1_IRQHandler            ; UART1 Rx and Tx
        DCD     SSI0_IRQHandler             ; SSI0 Rx and Tx
        DCD     I2C0_IRQHandler             ; I2C0 Master and Slave
        DCD     PWMFault_IRQHandler         ; PWM Fault
        DCD     PWMGen0_IRQHandler          ; PWM Generator 0
        DCD     PWMGen1_IRQHandler          ; PWM Generator 1
        DCD     PWMGen2_IRQHandler          ; PWM Generator 2
        DCD     QEI0_IRQHandler             ; Quadrature Encoder 0
        DCD     ADCSeq0_IRQHandler          ; ADC Sequence 0
        DCD     ADCSeq1_IRQHandler          ; ADC Sequence 1
        DCD     ADCSeq2_IRQHandler          ; ADC Sequence 2
        DCD     ADCSeq3_IRQHandler          ; ADC Sequence 3
        DCD     Watchdog_IRQHandler         ; Watchdog timer
        DCD     Timer0A_IRQHandler          ; Timer 0 subtimer A
        DCD     Timer0B_IRQHandler          ; Timer 0 subtimer B
        DCD     Timer1A_IRQHandler          ; Timer 1 subtimer A
        DCD     Timer1B_IRQHandler          ; Timer 1 subtimer B
        DCD     Timer2A_IRQHandler          ; Timer 2 subtimer A
        DCD     Timer2B_IRQHandler          ; Timer 2 subtimer B
        DCD     Comp0_IRQHandler            ; Analog Comparator 0
        DCD     Comp1_IRQHandler            ; Analog Comparator 1
        DCD     Comp2_IRQHandler            ; Analog Comparator 2
        DCD     SysCtrl_IRQHandler          ; System Control (PLL, OSC, BO)
        DCD     FlashCtrl_IRQHandler        ; FLASH Control
        DCD     GPIOPortF_IRQHandler        ; GPIO Port F
        DCD     GPIOPortG_IRQHandler        ; GPIO Port G
        DCD     GPIOPortH_IRQHandler        ; GPIO Port H
        DCD     UART2_IRQHandler            ; UART2 Rx and Tx
        DCD     SSI1_IRQHandler             ; SSI1 Rx and Tx
        DCD     Timer3A_IRQHandler          ; Timer 3 subtimer A
        DCD     Timer3B_IRQHandler          ; Timer 3 subtimer B
        DCD     I2C1_IRQHandler             ; I2C1 Master and Slave
        DCD     QEI1_IRQHandler             ; Quadrature Encoder 1
        DCD     CAN0_IRQHandler             ; CAN0
        DCD     CAN1_IRQHandler             ; CAN1
        DCD     CAN2_IRQHandler             ; CAN2
        DCD     Default_Handler             ; Reserved
        DCD     Hibernate_IRQHandler        ; Hibernate
        DCD     USB0_IRQHandler             ; USB0
        DCD     PWMGen3_IRQHandler          ; PWM Generator 3
        DCD     uDMAST_IRQHandler           ; uDMA Software Transfer
        DCD     uDMAError_IRQHandler        ; uDMA Error
        DCD     ADC1Seq0_IRQHandler         ; ADC1 Sequence 0
        DCD     ADC1Seq1_IRQHandler         ; ADC1 Sequence 1
        DCD     ADC1Seq2_IRQHandler         ; ADC1 Sequence 2
        DCD     ADC1Seq3_IRQHandler         ; ADC1 Sequence 3
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     GPIOPortJ_IRQHandler        ; GPIO Port J
        DCD     GPIOPortK_IRQHandler        ; GPIO Port K
        DCD     GPIOPortL_IRQHandler        ; GPIO Port L
        DCD     SSI2_IRQHandler             ; SSI2 Rx and Tx
        DCD     SSI3_IRQHandler             ; SSI3 Rx and Tx
        DCD     UART3_IRQHandler            ; UART3 Rx and Tx
        DCD     UART4_IRQHandler            ; UART4 Rx and Tx
        DCD     UART5_IRQHandler            ; UART5 Rx and Tx
        DCD     UART6_IRQHandler            ; UART6 Rx and Tx
        DCD     UART7_IRQHandler            ; UART7 Rx and Tx
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     I2C2_IRQHandler             ; I2C2 Master and Slave
        DCD     I2C3_IRQHandler             ; I2C3 Master and Slave
        DCD     Timer4A_IRQHandler          ; Timer 4 subtimer A
        DCD     Timer4B_IRQHandler          ; Timer 4 subtimer B
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Timer5A_IRQHandler          ; Timer 5 subtimer A
        DCD     Timer5B_IRQHandler          ; Timer 5 subtimer B
        DCD     WideTimer0A_IRQHandler      ; Wide Timer 0 subtimer A
        DCD     WideTimer0B_IRQHandler      ; Wide Timer 0 subtimer B
        DCD     WideTimer1A_IRQHandler      ; Wide Timer 1 subtimer A
        DCD     WideTimer1B_IRQHandler      ; Wide Timer 1 subtimer B
        DCD     WideTimer2A_IRQHandler      ; Wide Timer 2 subtimer A
        DCD     WideTimer2B_IRQHandler      ; Wide Timer 2 subtimer B
        DCD     WideTimer3A_IRQHandler      ; Wide Timer 3 subtimer A
        DCD     WideTimer3B_IRQHandler      ; Wide Timer 3 subtimer B
        DCD     WideTimer4A_IRQHandler      ; Wide Timer 4 subtimer A
        DCD     WideTimer4B_IRQHandler      ; Wide Timer 4 subtimer B
        DCD     WideTimer5A_IRQHandler      ; Wide Timer 5 subtimer A
        DCD     WideTimer5B_IRQHandler      ; Wide Timer 5 subtimer B
        DCD     FPU_IRQHandler              ; FPU
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     I2C4_IRQHandler             ; I2C4 Master and Slave
        DCD     I2C5_IRQHandler             ; I2C5 Master and Slave
        DCD     GPIOPortM_IRQHandler        ; GPIO Port M
        DCD     GPIOPortN_IRQHandler        ; GPIO Port N
        DCD     QEI2_IRQHandler             ; Quadrature Encoder 2
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     GPIOPortP0_IRQHandler       ; GPIO Port P (Summary or P0)
        DCD     GPIOPortP1_IRQHandler       ; GPIO Port P1
        DCD     GPIOPortP2_IRQHandler       ; GPIO Port P2
        DCD     GPIOPortP3_IRQHandler       ; GPIO Port P3
        DCD     GPIOPortP4_IRQHandler       ; GPIO Port P4
        DCD     GPIOPortP5_IRQHandler       ; GPIO Port P5
        DCD     GPIOPortP6_IRQHandler       ; GPIO Port P6
        DCD     GPIOPortP7_IRQHandler       ; GPIO Port P7
        DCD     GPIOPortQ0_IRQHandler       ; GPIO Port Q (Summary or Q0)
        DCD     GPIOPortQ1_IRQHandler       ; GPIO Port Q1
        DCD     GPIOPortQ2_IRQHandler       ; GPIO Port Q2
        DCD     GPIOPortQ3_IRQHandler       ; GPIO Port Q3
        DCD     GPIOPortQ4_IRQHandler       ; GPIO Port Q4
        DCD     GPIOPortQ5_IRQHandler       ; GPIO Port Q5
        DCD     GPIOPortQ6_IRQHandler       ; GPIO Port Q6
        DCD     GPIOPortQ7_IRQHandler       ; GPIO Port Q7
        DCD     GPIOPortR_IRQHandler        ; GPIO Port R
        DCD     GPIOPortS_IRQHandler        ; GPIO Port S
        DCD     PWM1Gen0_IRQHandler         ; PWM 1 Generator 0
        DCD     PWM1Gen1_IRQHandler         ; PWM 1 Generator 1
        DCD     PWM1Gen2_IRQHandler         ; PWM 1 Generator 2
        DCD     PWM1Gen3_IRQHandler         ; PWM 1 Generator 3
        DCD     PWM1Fault_IRQHandler        ; PWM 1 Fault

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
        ; relocate the Vector Table
        LDR     r0,=0xE000ED08  ; System Control Block/Vector Table Offset Reg
        LDR     r1,=__relocated_vector_table
        STR     r1,[r0]         ; SCB->VTOR := __relocated_vector_table

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
        PUBWEAK MemManage_Handler
MemManage_Handler
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        CPSID   i                ; disable all interrupts
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
        PUBWEAK  GPIOPortA_IRQHandler
        PUBWEAK  GPIOPortB_IRQHandler
        PUBWEAK  GPIOPortC_IRQHandler
        PUBWEAK  GPIOPortD_IRQHandler
        PUBWEAK  GPIOPortE_IRQHandler
        PUBWEAK  UART0_IRQHandler
        PUBWEAK  UART1_IRQHandler
        PUBWEAK  SSI0_IRQHandler
        PUBWEAK  I2C0_IRQHandler
        PUBWEAK  PWMFault_IRQHandler
        PUBWEAK  PWMGen0_IRQHandler
        PUBWEAK  PWMGen1_IRQHandler
        PUBWEAK  PWMGen2_IRQHandler
        PUBWEAK  QEI0_IRQHandler
        PUBWEAK  ADCSeq0_IRQHandler
        PUBWEAK  ADCSeq1_IRQHandler
        PUBWEAK  ADCSeq2_IRQHandler
        PUBWEAK  ADCSeq3_IRQHandler
        PUBWEAK  Watchdog_IRQHandler
        PUBWEAK  Timer0A_IRQHandler
        PUBWEAK  Timer0B_IRQHandler
        PUBWEAK  Timer1A_IRQHandler
        PUBWEAK  Timer1B_IRQHandler
        PUBWEAK  Timer2A_IRQHandler
        PUBWEAK  Timer2B_IRQHandler
        PUBWEAK  Comp0_IRQHandler
        PUBWEAK  Comp1_IRQHandler
        PUBWEAK  Comp2_IRQHandler
        PUBWEAK  SysCtrl_IRQHandler
        PUBWEAK  FlashCtrl_IRQHandler
        PUBWEAK  GPIOPortF_IRQHandler
        PUBWEAK  GPIOPortG_IRQHandler
        PUBWEAK  GPIOPortH_IRQHandler
        PUBWEAK  UART2_IRQHandler
        PUBWEAK  SSI1_IRQHandler
        PUBWEAK  Timer3A_IRQHandler
        PUBWEAK  Timer3B_IRQHandler
        PUBWEAK  I2C1_IRQHandler
        PUBWEAK  QEI1_IRQHandler
        PUBWEAK  CAN0_IRQHandler
        PUBWEAK  CAN1_IRQHandler
        PUBWEAK  CAN2_IRQHandler
        PUBWEAK  Hibernate_IRQHandler
        PUBWEAK  USB0_IRQHandler
        PUBWEAK  PWMGen3_IRQHandler
        PUBWEAK  uDMAST_IRQHandler
        PUBWEAK  uDMAError_IRQHandler
        PUBWEAK  ADC1Seq0_IRQHandler
        PUBWEAK  ADC1Seq1_IRQHandler
        PUBWEAK  ADC1Seq2_IRQHandler
        PUBWEAK  ADC1Seq3_IRQHandler
        PUBWEAK  GPIOPortJ_IRQHandler
        PUBWEAK  GPIOPortK_IRQHandler
        PUBWEAK  GPIOPortL_IRQHandler
        PUBWEAK  SSI2_IRQHandler
        PUBWEAK  SSI3_IRQHandler
        PUBWEAK  UART3_IRQHandler
        PUBWEAK  UART4_IRQHandler
        PUBWEAK  UART5_IRQHandler
        PUBWEAK  UART6_IRQHandler
        PUBWEAK  UART7_IRQHandler
        PUBWEAK  I2C2_IRQHandler
        PUBWEAK  I2C3_IRQHandler
        PUBWEAK  Timer4A_IRQHandler
        PUBWEAK  Timer4B_IRQHandler
        PUBWEAK  Timer5A_IRQHandler
        PUBWEAK  Timer5B_IRQHandler
        PUBWEAK  WideTimer0A_IRQHandler
        PUBWEAK  WideTimer0B_IRQHandler
        PUBWEAK  WideTimer1A_IRQHandler
        PUBWEAK  WideTimer1B_IRQHandler
        PUBWEAK  WideTimer2A_IRQHandler
        PUBWEAK  WideTimer2B_IRQHandler
        PUBWEAK  WideTimer3A_IRQHandler
        PUBWEAK  WideTimer3B_IRQHandler
        PUBWEAK  WideTimer4A_IRQHandler
        PUBWEAK  WideTimer4B_IRQHandler
        PUBWEAK  WideTimer5A_IRQHandler
        PUBWEAK  WideTimer5B_IRQHandler
        PUBWEAK  FPU_IRQHandler
        PUBWEAK  I2C4_IRQHandler
        PUBWEAK  I2C5_IRQHandler
        PUBWEAK  GPIOPortM_IRQHandler
        PUBWEAK  GPIOPortN_IRQHandler
        PUBWEAK  QEI2_IRQHandler
        PUBWEAK  GPIOPortP0_IRQHandler
        PUBWEAK  GPIOPortP1_IRQHandler
        PUBWEAK  GPIOPortP2_IRQHandler
        PUBWEAK  GPIOPortP3_IRQHandler
        PUBWEAK  GPIOPortP4_IRQHandler
        PUBWEAK  GPIOPortP5_IRQHandler
        PUBWEAK  GPIOPortP6_IRQHandler
        PUBWEAK  GPIOPortP7_IRQHandler
        PUBWEAK  GPIOPortQ0_IRQHandler
        PUBWEAK  GPIOPortQ1_IRQHandler
        PUBWEAK  GPIOPortQ2_IRQHandler
        PUBWEAK  GPIOPortQ3_IRQHandler
        PUBWEAK  GPIOPortQ4_IRQHandler
        PUBWEAK  GPIOPortQ5_IRQHandler
        PUBWEAK  GPIOPortQ6_IRQHandler
        PUBWEAK  GPIOPortQ7_IRQHandler
        PUBWEAK  GPIOPortR_IRQHandler
        PUBWEAK  GPIOPortS_IRQHandler
        PUBWEAK  PWM1Gen0_IRQHandler
        PUBWEAK  PWM1Gen1_IRQHandler
        PUBWEAK  PWM1Gen2_IRQHandler
        PUBWEAK  PWM1Gen3_IRQHandler
        PUBWEAK  PWM1Fault_IRQHandler

Default_Handler
GPIOPortA_IRQHandler
GPIOPortB_IRQHandler
GPIOPortC_IRQHandler
GPIOPortD_IRQHandler
GPIOPortE_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SSI0_IRQHandler
I2C0_IRQHandler
PWMFault_IRQHandler
PWMGen0_IRQHandler
PWMGen1_IRQHandler
PWMGen2_IRQHandler
QEI0_IRQHandler
ADCSeq0_IRQHandler
ADCSeq1_IRQHandler
ADCSeq2_IRQHandler
ADCSeq3_IRQHandler
Watchdog_IRQHandler
Timer0A_IRQHandler
Timer0B_IRQHandler
Timer1A_IRQHandler
Timer1B_IRQHandler
Timer2A_IRQHandler
Timer2B_IRQHandler
Comp0_IRQHandler
Comp1_IRQHandler
Comp2_IRQHandler
SysCtrl_IRQHandler
FlashCtrl_IRQHandler
GPIOPortF_IRQHandler
GPIOPortG_IRQHandler
GPIOPortH_IRQHandler
UART2_IRQHandler
SSI1_IRQHandler
Timer3A_IRQHandler
Timer3B_IRQHandler
I2C1_IRQHandler
QEI1_IRQHandler
CAN0_IRQHandler
CAN1_IRQHandler
CAN2_IRQHandler
Hibernate_IRQHandler
USB0_IRQHandler
PWMGen3_IRQHandler
uDMAST_IRQHandler
uDMAError_IRQHandler
ADC1Seq0_IRQHandler
ADC1Seq1_IRQHandler
ADC1Seq2_IRQHandler
ADC1Seq3_IRQHandler
GPIOPortJ_IRQHandler
GPIOPortK_IRQHandler
GPIOPortL_IRQHandler
SSI2_IRQHandler
SSI3_IRQHandler
UART3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
UART6_IRQHandler
UART7_IRQHandler
I2C2_IRQHandler
I2C3_IRQHandler
Timer4A_IRQHandler
Timer4B_IRQHandler
Timer5A_IRQHandler
Timer5B_IRQHandler
WideTimer0A_IRQHandler
WideTimer0B_IRQHandler
WideTimer1A_IRQHandler
WideTimer1B_IRQHandler
WideTimer2A_IRQHandler
WideTimer2B_IRQHandler
WideTimer3A_IRQHandler
WideTimer3B_IRQHandler
WideTimer4A_IRQHandler
WideTimer4B_IRQHandler
WideTimer5A_IRQHandler
WideTimer5B_IRQHandler
FPU_IRQHandler
I2C4_IRQHandler
I2C5_IRQHandler
GPIOPortM_IRQHandler
GPIOPortN_IRQHandler
QEI2_IRQHandler
GPIOPortP0_IRQHandler
GPIOPortP1_IRQHandler
GPIOPortP2_IRQHandler
GPIOPortP3_IRQHandler
GPIOPortP4_IRQHandler
GPIOPortP5_IRQHandler
GPIOPortP6_IRQHandler
GPIOPortP7_IRQHandler
GPIOPortQ0_IRQHandler
GPIOPortQ1_IRQHandler
GPIOPortQ2_IRQHandler
GPIOPortQ3_IRQHandler
GPIOPortQ4_IRQHandler
GPIOPortQ5_IRQHandler
GPIOPortQ6_IRQHandler
GPIOPortQ7_IRQHandler
GPIOPortR_IRQHandler
GPIOPortS_IRQHandler
PWM1Gen0_IRQHandler
PWM1Gen1_IRQHandler
PWM1Gen2_IRQHandler
PWM1Gen3_IRQHandler
PWM1Fault_IRQHandler
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

