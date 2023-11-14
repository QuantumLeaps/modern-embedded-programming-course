//============================================================================
// Product: QUTEST port for NUCLEO-C031C6 board
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

#include "stm32c0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

//Q_DEFINE_THIS_MODULE("qutest_port")

// Local-scope defines -------------------------------------------------------
// LED pins available on the board (just one user LED LD4--Green on PA.5)
#define LD4_PIN  5U

// Button pins available on the board (just one user Button B1 on PC.13)
#define B1_PIN   13U

static uint16_t const UARTPrescTable[12] = {
    1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};

#define UART_DIV_SAMPLING16(__PCLK__, __BAUD__, __CLOCKPRESCALER__) \
  ((((__PCLK__)/UARTPrescTable[(__CLOCKPRESCALER__)]) \
  + ((__BAUD__)/2U)) / (__BAUD__))

#define UART_PRESCALER_DIV1  0U

// USART2 pins PA.2 and PA.3
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

//............................................................................
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled. Such ISRs don't need to call QK_ISR_ENTRY/
// QK_ISR_EXIT and they cannot post or publish events.
void USART2_IRQHandler(void); // prototype
void USART2_IRQHandler(void) { // used in QS-RX (kernel UNAWARE interrupt)
    // is RX register NOT empty?
    while ((USART2->ISR & (1U << 5U)) != 0U) {
        uint32_t b = USART2->RDR;
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

    // enable GPIOA clock port for the LED LD4
    RCC->IOPENR |= (1U << 0U);

    // set all used GPIOA pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~(3U << 2U*LD4_PIN);
    GPIOA->MODER   |=  (1U << 2U*LD4_PIN);
    GPIOA->OTYPER  &= ~(1U <<    LD4_PIN);
    GPIOA->OSPEEDR &= ~(3U << 2U*LD4_PIN);
    GPIOA->OSPEEDR |=  (1U << 2U*LD4_PIN);
    GPIOA->PUPDR   &= ~(3U << 2U*LD4_PIN);

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);

    // enable peripheral clock for USART2
    RCC->IOPENR  |= ( 1U <<  0U);  // Enable GPIOA clock for USART pins
    RCC->APBENR1 |= ( 1U << 17U);  // Enable USART#2 clock

    // Configure PA to USART2_RX, PA to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4U*USART2_RX_PIN) | (15U << 4U*USART2_TX_PIN));
    GPIOA->AFR[0] |=  (( 1U << 4U*USART2_RX_PIN) | ( 1U << 4U*USART2_TX_PIN));
    GPIOA->MODER  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->MODER  |=  (( 2U << 2U*USART2_RX_PIN) | ( 2U << 2U*USART2_TX_PIN));

    // baud rate
    USART2->BRR  = UART_DIV_SAMPLING16(
                       SystemCoreClock, 115200U, UART_PRESCALER_DIV1);
    USART2->CR3  = 0x0000U |      // no flow control
                   (1U << 12U);   // disable overrun detection (OVRDIS)
    USART2->CR2  = 0x0000U;       // 1 stop bit
    USART2->CR1  = ((1U <<  2U) | // enable RX
                    (1U <<  3U) | // enable TX
                    (1U <<  5U) | // enable RX interrupt
                    (0U << 12U) | // 8 data bits
                    (0U << 28U) | // 8 data bits
                    (1U <<  0U)); // enable USART

    // explicitly set NVIC priorities of all Cortex-M interrupts used
    NVIC_SetPriorityGrouping(0U);
    NVIC_SetPriority(USART2_IRQn,    0U);

    // enable the UART RX interrupt...
    NVIC_EnableIRQ(USART2_IRQn); // UART2 interrupt used for QS-RX

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
    // wait as long as the UART is busy
    while ((USART2->ISR & (1U << 7U)) == 0U) {
    }
    // delay before returning to allow all produced QS bytes to be received
    for (uint32_t volatile dly_ctr = 10000U; dly_ctr > 0U; --dly_ctr) {
    }
}
//............................................................................
void QS_onFlush(void) {
    for (;;) {
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            while ((USART2->ISR & (1U << 7U)) == 0U) {
            }
            USART2->TDR  = b; // put into the DR register
        }
        else {
            break;
        }
    }
}
//............................................................................
// callback function to reset the target (to be implemented in the BSP)
void QS_onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS_doOutput(void) {
    if ((USART2->ISR & (1U << 7U)) != 0U) { // is TXE empty?
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {   // not End-Of-Data?
            USART2->TDR = b; // put into the DR register
        }
    }
}
//............................................................................
void QS_onTestLoop() {
    QS_rxPriv_.inTestLoop = true;
    while (QS_rxPriv_.inTestLoop) {

        // toggle an LED LD2 on and then off (not enough LEDs, see NOTE02)
        GPIOA->BSRR = (1U << LD4_PIN);         // turn LED[n] on
        GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LED[n] off

        QS_rxParse();  // parse all the received bytes

        if ((USART2->ISR & (1U << 7U)) != 0U) {  // is TXE empty?
            QF_INT_DISABLE();
            uint16_t b = QS_getByte();
            QF_INT_ENABLE();

            if (b != QS_EOD) {  // not End-Of-Data?
                USART2->TDR = (b & 0xFFU);  // put into the DR register
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    QS_rxPriv_.inTestLoop = true;
}
//============================================================================
// NOTE0:
// ARM Cortex-M0+ does NOT provide "kernel-unaware" interrupts, and
// consequently *all* interrupts are "kernel-aware". This means that
// the UART interrupt used for QS-RX is frequently DISABLED (e.g., to
// perform QS-TX). That can lead to lost some of the received bytes, and
// consequently some QUTest tests might be failing.
// A fix for that would be to use DMA for handling QS-RX, but this is
// currently not implemented.
//
