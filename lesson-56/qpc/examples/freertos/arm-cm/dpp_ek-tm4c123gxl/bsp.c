//============================================================================
// Product: DPP example, EK-TM4C123GXL board, FreeRTOS kernel
// Last updated for version 8.0.0
// Last updated on  2024-09-18
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"                 // QP/C real-time event framework
#include "dpp.h"                 // DPP Application interface
#include "bsp.h"                 // Board Support Package

#include "TM4C123GH6PM.h"        // the device specific header (TI)
#include "sysctl.h"              // system control driver (TI)
#include "gpio.h"                // GPIO driver (TI)
// add other drivers if necessary...

Q_DEFINE_THIS_FILE  // define the name of this file for assertions

// LEDs and Switches of the EK-TM4C123GXL board ............................
#define LED_RED     (1U << 1U)
#define LED_GREEN   (1U << 3U)
#define LED_BLUE    (1U << 2U)

#define BTN_SW1     (1U << 4U)
#define BTN_SW2     (1U << 0U)

// "RTOS-aware" interrupt priorities for FreeRTOS on ARM Cortex-M, NOTE1
#define RTOS_AWARE_ISR_CMSIS_PRI \
    (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8-__NVIC_PRIO_BITS))

// Local-scope objects -----------------------------------------------------
static uint32_t l_rndSeed;

#ifdef Q_SPY

    // QS identifiers for non-QP sources of events
    static uint8_t const l_TickHook = 0U;
    static uint8_t const l_GPIOPortA_IRQHandler = 0U;

    #define UART_BAUD_RATE      115200U
    #define UART_FR_TXFE        (1U << 7)
    #define UART_FR_RXFE        (1U << 4)
    #define UART_TXFIFO_DEPTH   16U

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QS_USER,
        PAUSED_STAT,
    };

#endif

//============================================================================
// Error handler

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

#ifndef NDEBUG
    // light up all LEDs
    GPIOF_AHB->DATA_Bits[LED_GREEN | LED_RED | LED_BLUE] = 0xFFU;
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#else
    NVIC_SystemReset();
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs used in the application ==============================================

// NOTE: this ISR is for testing of the various preemption scenarios
// by triggering the GPIOPortA interrupt from the debugger. You achieve
// this by writing 0 to the  SWTRIG register at 0xE000,EF00.
//
// Code Composer Studio: From the CCS debugger you need open the register
// window and select NVIC registers from the drop-down list. You scroll to
// the NVIC_SW_TRIG register, which denotes the Software Trigger Interrupt
// Register in the NVIC. To trigger the GPIOA interrupt you need to write
// 0x00 to the NVIC_SW_TRIG by clicking on this field, entering the value,
// and pressing the Enter key.
//
// IAR EWARM: From the C-Spy debugger you need to open Registers view and
// select the "Other Systems Register" group. From there, you need to write
// 0 to the STIR write-only register and press enter.
//
// NOTE: only the "FromISR" FreeRTOS API variants are allowed in the ISRs!
void GPIOPortA_IRQHandler(void); // prototype
void GPIOPortA_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // for testing...
    QACTIVE_POST_FROM_ISR(AO_Table,
        Q_NEW_FROM_ISR(QEvt, MAX_PUB_SIG),
        &xHigherPriorityTaskWoken,
        &l_GPIOPortA_IRQHandler);

    // the usual end of FreeRTOS ISR...
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
//............................................................................
#ifdef Q_SPY

// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "kernel-unaware" meaning that it does not interact with
// the FreeRTOS or QP and is not disabled. Such ISRs don't need to call
// portEND_SWITCHING_ISR(() at the end, but they also cannot call any
// FreeRTOS or QP APIs.
void UART0_IRQHandler(void); // prototype
void UART0_IRQHandler(void) {
    uint32_t status = UART0->RIS; // get the raw interrupt status
    UART0->ICR = status;          // clear the asserted interrupts

    while ((UART0->FR & UART_FR_RXFE) == 0) { // while RX FIFO NOT empty
        uint32_t b = UART0->DR;
        QS_RX_PUT(b);
    }
}
#endif

// Application hooks used in this project ====================================
// NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook

void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // process time events at rate 0
    QTIMEEVT_TICK_FROM_ISR(0U, &xHigherPriorityTaskWoken, &l_TickHook);

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };

    uint32_t current = ~GPIOF_AHB->DATA_Bits[BTN_SW1 | BTN_SW2]; // SW1&SW2
    uint32_t tmp = buttons.depressed; // save debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

    if ((tmp & BTN_SW1) != 0U) {  // debounced SW1 state changed?
        if ((current & BTN_SW1) != 0U) { // is SW1 depressed?
            static QEvt const pauseEvt = QEVT_INITIALIZER(PAUSE_SIG);
            QACTIVE_PUBLISH_FROM_ISR(&pauseEvt,
                                &xHigherPriorityTaskWoken,
                                &l_TickHook);
        }
        else { // the button is released
            static QEvt const serveEvt = QEVT_INITIALIZER(SERVE_SIG);
            QACTIVE_PUBLISH_FROM_ISR(&serveEvt,
                                &xHigherPriorityTaskWoken,
                                &l_TickHook);
        }
    }

    // notify FreeRTOS to perform context switch from ISR, if needed
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
//............................................................................
void vApplicationIdleHook(void) {
    // toggle the User LED on and then off, see NOTE01
    QF_INT_DISABLE();
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0xFFU;  // turn the Blue LED on
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;     // turn the Blue LED off
    QF_INT_ENABLE();

    // Some floating point code is to exercise the VFP...
    float volatile x = 1.73205F;
    x = x * 1.73205F;

#ifdef Q_SPY
    QS_rxParse();  // parse all the received bytes

    if ((UART0->FR & UART_FR_TXFE) != 0U) {  // TX done?
        uint16_t fifo = UART_TXFIFO_DEPTH;   // max bytes we can accept
        uint8_t const *block;

        QF_INT_DISABLE();
        block = QS_getBlock(&fifo); // try to get next block to transmit
        QF_INT_ENABLE();

        while (fifo-- != 0U) {  // any bytes in the block?
            UART0->DR = *block++;  // put into the FIFO
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    __WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Q_UNUSED_PAR(xTask);
    Q_UNUSED_PAR(pcTaskName);
    Q_ERROR();
}
//............................................................................
// configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
// provide an implementation of vApplicationGetIdleTaskMemory() to provide
// the memory that is used by the Idle task.
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    // If the buffers to be provided to the Idle task are declared inside
    // this function then they must be declared static - otherwise they will
    // be allocated on the stack and so not exists after this function exits.
    //
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    // Pass out a pointer to the StaticTask_t structure in which the
    // Idle task's state will be stored.
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    // Pass out the array that will be used as the Idle task's stack.
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    // Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    // Note that, as the array is necessarily of type StackType_t,
    // configMINIMAL_STACK_SIZE is specified in words, not bytes.
    //
    *pulIdleTaskStackSize = Q_DIM(uxIdleTaskStack);
}

// BSP functions =============================================================

//............................................................................
void BSP_init(void) {
    // Configure the MPU to prevent NULL-pointer dereferencing ...
    MPU->RBAR = 0x0U                          // base address (NULL)
                | MPU_RBAR_VALID_Msk          // valid region
                | (MPU_RBAR_REGION_Msk & 7U); // region #7
    MPU->RASR = (7U << MPU_RASR_SIZE_Pos)     // 2^(7+1) region
                | (0x0U << MPU_RASR_AP_Pos)   // no-access region
                | MPU_RASR_ENABLE_Msk;        // region enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk       // enable background region
                | MPU_CTRL_ENABLE_Msk;        // enable the MPU
    __ISB();
    __DSB();

    // enable the MemManage_Handler for MPU exception
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // NOTE: VFP (hardware Floating Point) unit is configured by FreeRTOS

    // enable clock for to the peripherals used by this application...
    SYSCTL->RCGCGPIO  |= (1U << 5U); // enable Run mode for GPIOF
    SYSCTL->GPIOHBCTL |= (1U << 5U); // enable AHB for GPIOF
    __ISB();
    __DSB();

    // configure LEDs (digital output)
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    // configure switches...

    // unlock access to the SW2 pin because it is PROTECTED
    GPIOF_AHB->LOCK = 0x4C4F434BU; // unlock GPIOCR register for SW2
    // commit the write (cast const away)
    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x01U;

    GPIOF_AHB->DIR &= ~(BTN_SW1 | BTN_SW2); // input
    GPIOF_AHB->DEN |= (BTN_SW1 | BTN_SW2); // digital enable
    GPIOF_AHB->PUR |= (BTN_SW1 | BTN_SW2); // pull-up resistor enable

    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x00U;
    GPIOF_AHB->LOCK = 0x0; // lock GPIOCR register for SW2

    BSP_randomSeed(1234U);

    // initialize the QS software tracing...
    if (!QS_INIT((void *)0)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_TickHook);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QS_GRP_ALL);   // all records
    QS_GLB_FILTER(-QS_QF_TICK);      // exclude the clock tick
}
//............................................................................
void BSP_start(void) {
    // initialize event pools
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO]; // small pool
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive_psInit(subscrSto, Q_DIM(subscrSto));

    // start the active objects/threads...
    static QEvtPtr philoQueueSto[N_PHILO][10];
    static StackType_t philoStack[N_PHILO][configMINIMAL_STACK_SIZE];
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        Philo_ctor(n); // instantiate all Philosopher active objects
        QActive_setAttr(AO_Philo[n], TASK_NAME_ATTR, "Philo");
        QActive_start(AO_Philo[n],  // AO to start
            n + 3U,                  // QP prio. of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            philoStack[n],           // stack storage
            sizeof(philoStack[n]),   // stack size [bytes]
            (QEvt *)0);              // initialization event (not used)
    }

    static QEvtPtr tableQueueSto[N_PHILO];
    static StackType_t tableStack[configMINIMAL_STACK_SIZE];
    Table_ctor(); // instantiate the Table active object
    QActive_setAttr(AO_Table, TASK_NAME_ATTR, "Table");
    QActive_start(AO_Table,         // AO to start
        N_PHILO + 7U,                // QP prio. of the AO
        tableQueueSto,               // event queue storage
        Q_DIM(tableQueueSto),        // queue length [events]
        tableStack,                  // stack storage
        sizeof(tableStack),          // stack size [bytes]
        (QEvt *)0);                  // initialization event (not used)
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    GPIOF_AHB->DATA_Bits[LED_GREEN] = ((stat[0] == 'e') ? LED_GREEN : 0U);

    // app-specific trace record...
    QS_BEGIN_ID(PHILO_STAT, AO_Table->prio)
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t const paused) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = ((paused != 0U) ? LED_BLUE : 0U);

    // application-specific trace record
    QS_BEGIN_ID(PAUSED_STAT, AO_Table->prio)
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
void BSP_randomSeed(uint32_t const seed) {
    l_rndSeed = seed;
}
//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // Some floating point code is to exercise the VFP...
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    vTaskSuspendAll(); // lock FreeRTOS scheduler
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rndSeed * (3U*7U*11U*13U*23U);
    l_rndSeed = rnd; // set for the next time
    xTaskResumeAll(); // unlock the FreeRTOS scheduler

    return (rnd >> 8);
}
//............................................................................
void BSP_ledOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0xFFU;
}
//............................................................................
void BSP_ledOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0x00U;
}
//............................................................................
void BSP_terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

//============================================================================

// QF callbacks --------------------------------------------------------------

void QF_onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    //SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); // done in FreeRTOS

    // assign all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(UART0_IRQn,     0U); // kernel unaware interrupt
    NVIC_SetPriority(GPIOA_IRQn,     RTOS_AWARE_ISR_CMSIS_PRI + 0U);
    NVIC_SetPriority(SysTick_IRQn,   RTOS_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(GPIOA_IRQn);

#ifdef Q_SPY
    NVIC_EnableIRQ(UART0_IRQn); // UART0 interrupt used for QS-RX
#endif
}
//............................................................................
void QF_onCleanup(void) {
}

// QS callbacks --------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[100];    // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // enable clock for UART0 and GPIOA (used by UART0 pins)
    SYSCTL->RCGCUART |= (1U << 0U); // enable Run mode for UART0
    SYSCTL->RCGCGPIO |= (1U << 0U); // enable Run mode for GPIOA

    // configure UART0 pins for UART operation
    uint32_t tmp = (1U << 0U) | (1U << 1U);
    GPIOA->DIR   &= ~tmp;
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->AMSEL &= ~tmp;  // disable analog function on the pins
    GPIOA->AFSEL |= tmp;   // enable ALT function on the pins
    GPIOA->DEN   |= tmp;   // enable digital I/O on the pins
    GPIOA->PCTL  &= ~0x00U;
    GPIOA->PCTL  |= 0x11U;

    // configure the UART for the desired baud rate, 8-N-1 operation
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD   = tmp / 64U;
    UART0->FBRD   = tmp % 64U;
    UART0->LCRH   = (0x3U << 5U); // configure 8-N-1 operation
    UART0->LCRH  |= (0x1U << 4U); // enable FIFOs
    UART0->CTL    = (1U << 0U)    // UART enable
                    | (1U << 8U)  // UART TX enable
                    | (1U << 9U); // UART RX enable

    // configure UART interrupts (for the RX channel)
    UART0->IM   |= (1U << 4U) | (1U << 6U); // enable RX and RX-TO interrupt
    UART0->IFLS |= (0x2U << 2U);    // interrupt on RX FIFO half-full
    // NOTE: do not enable the UART0 interrupt yet. Wait till QF_onStartup()

    // configure TIMER5 to produce QS time stamp
    SYSCTL->RCGCTIMER |= (1U << 5U); // enable run mode for Timer5
    TIMER5->CTL  = 0U;               // disable Timer1 output
    TIMER5->CFG  = 0x0U;             // 32-bit configuration
    TIMER5->TAMR = (1U << 4U) | 0x02U; // up-counting periodic mode
    TIMER5->TAILR= 0xFFFFFFFFU;      // timer interval
    TIMER5->ICR  = 0x1U;             // TimerA timeout flag bit clears
    TIMER5->CTL |= (1U << 0U);       // enable TimerA module

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
QSTimeCtr QS_onGetTime(void) { // NOTE: invoked with interrupts DISABLED
    return TIMER5->TAV;
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    for (;;) {
        uint16_t b = QS_getByte();
        if (b != QS_EOD) { // NOT end-of-data
            // busy-wait as long as TX FIFO has data to transmit
            while ((UART0->FR & UART_FR_TXFE) == 0U) {
            }
            // place the byte in the UART DR register
            UART0->DR = b;
        }
        else {
            break; // break out of the loop
        }
    }
}
//............................................................................
void QS_onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY
//----------------------------------------------------------------------------

//============================================================================
// NOTE1:
// The configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY constant from the
// FreeRTOS configuration file specifies the highest ISR priority that
// is disabled by the QF framework. The value is suitable for the
// NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
// with the numerical values of priorities equal or higher than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
// QP/FreeRTOS services. These ISRs are "kernel-aware".
//
// Conversely, any ISRs prioritized above the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
// the numerical values of priorities less than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
// not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
// QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
// can communicate with the QF framework is by triggering a "kernel-aware"
// ISR, which can post/publish events.
//
// For more information, see article "Running the RTOS on a ARM Cortex-M Core"
// http://www.freertos.org/RTOS-Cortex-M3-M4.html
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invocations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.

