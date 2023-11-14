//============================================================================
// Product: QUTEST port for the EK-TM4C123GXL board
// Last updated for version 7.3.0
// Last updated on  2023-08-18
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef Q_SPY
    #error "Q_SPY must be defined to compile qutest_port.c"
#endif // Q_SPY

#define QP_IMPL        // this is QP implementation
#include "qp_port.h"   // QP port
#include "qsafe.h"     // QP Functional Safety (FuSa) Subsystem
#include "qs_port.h"   // QS port
#include "qs_pkg.h"    // QS package-scope interface

#include "TM4C123GH6PM.h"  // the device specific header (TI)
#include "rom.h"           // the built-in ROM functions (TI)
#include "sysctl.h"        // system control driver (TI)
#include "gpio.h"          // GPIO driver (TI)
// add other drivers if necessary...

//Q_DEFINE_THIS_MODULE("qutest_port")

// ISRs defined in this BSP --------------------------------------------------
void UART0_IRQHandler(void);

// Local-scope objects -------------------------------------------------------
#define LED_RED     (1U << 1)
#define LED_BLUE    (1U << 2)
#define LED_GREEN   (1U << 3)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

#define UART_BAUD_RATE      115200U
#define UART_FR_TXFE        (1U << 7)
#define UART_FR_RXFE        (1U << 4)
#define UART_TXFIFO_DEPTH   16U

//............................................................................
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled. Such ISRs don't need to call QK_ISR_ENTRY/
// QK_ISR_EXIT and they cannot post or publish events.
//
void UART0_IRQHandler(void) {
    uint32_t status = UART0->RIS; // get the raw interrupt status
    UART0->ICR = status;          // clear the asserted interrupts

    while ((UART0->FR & UART_FR_RXFE) == 0) { // while RX FIFO NOT empty
        uint32_t b = UART0->DR;
        QS_RX_PUT(b);
    }
}

//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// QS callbacks ==============================================================
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf  (qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[256];    // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // enable clock for to the peripherals used by this application...
    SYSCTL->RCGCGPIO |= (1U << 5U); // enable Run mode for GPIOF

    // configure the LEDs and push buttons
    GPIOF->DIR |= (LED_RED | LED_GREEN | LED_BLUE);// set direction: output
    GPIOF->DEN |= (LED_RED | LED_GREEN | LED_BLUE); // digital enable
    GPIOF->DATA_Bits[LED_RED]   = 0U; // turn the LED off
    GPIOF->DATA_Bits[LED_GREEN] = 0U; // turn the LED off
    GPIOF->DATA_Bits[LED_BLUE]  = 0U; // turn the LED off

    // configure the Buttons
    GPIOF->DIR &= ~(BTN_SW1 | BTN_SW2); //  set direction: input
    ROM_GPIOPadConfigSet(GPIOF_BASE, (BTN_SW1 | BTN_SW2),
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

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

    // explicitly set NVIC priorities of all Cortex-M interrupts used
    NVIC_SetPriorityGrouping(0U);
    NVIC_SetPriority(UART0_IRQn, 0U); // kernel unaware interrupt

    // enable the UART RX interrupt...
    NVIC_EnableIRQ(UART0_IRQn); // UART0 interrupt used for QS-RX

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
    // wait as long as the UART is busy
    while ((UART0->FR & UART_FR_TXFE) == 0) {
    }
    // delay before returning to allow all produced QS bytes to be received
    for (uint32_t volatile dly_ctr = 100000U; dly_ctr > 0U; --dly_ctr) {
    }
}
//............................................................................
void QS_onFlush(void) {
    uint16_t fifo = UART_TXFIFO_DEPTH; // Tx FIFO depth
    uint8_t const *block;
    while ((block = QS_getBlock(&fifo)) != (uint8_t *)0) {
        GPIOF->DATA_Bits[LED_GREEN] = LED_GREEN;
        // busy-wait as long as TX FIFO has data to transmit
        while ((UART0->FR & UART_FR_TXFE) == 0) {
        }

        while (fifo-- != 0) {    // any bytes in the block?
            UART0->DR = *block++; // put into the TX FIFO
        }
        fifo = UART_TXFIFO_DEPTH; // re-load the Tx FIFO depth
        GPIOF->DATA_Bits[LED_GREEN] = 0U;
    }
}
//............................................................................
// callback function to reset the target (to be implemented in the BSP)
void QS_onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS_doOutput(void) {
    if ((UART0->FR & UART_FR_TXFE) != 0U) {  // TX done?
        uint16_t fifo = UART_TXFIFO_DEPTH;   // max bytes we can accept
        uint8_t const *block;

        QF_INT_DISABLE();
        block = QS_getBlock(&fifo); // try to get next block to transmit
        QF_INT_ENABLE();

        while (fifo-- != 0) {       // any bytes in the block?
            UART0->DR = *block++;   // put into the FIFO
        }
    }
}
//............................................................................
void QS_onTestLoop() {
    QS_rxPriv_.inTestLoop = true;
    while (QS_rxPriv_.inTestLoop) {

        // toggle the User LED on and then off, see NOTE01
        GPIOF->DATA_Bits[LED_BLUE] = LED_BLUE;  // turn the Blue LED on
        GPIOF->DATA_Bits[LED_BLUE] = 0U;        // turn the Blue LED off

        QS_rxParse();  // parse all the received bytes

        if ((UART0->FR & UART_FR_TXFE) != 0U) {  // TX done?
            uint16_t fifo = UART_TXFIFO_DEPTH;   // max bytes we can accept
            uint8_t const *block;

            QF_INT_DISABLE();
            block = QS_getBlock(&fifo); // try to get next block to transmit
            QF_INT_ENABLE();

            while (fifo-- != 0) {       // any bytes in the block?
                UART0->DR = *block++;   // put into the FIFO
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    QS_rxPriv_.inTestLoop = true;
}
