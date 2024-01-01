/*============================================================================
* ET: embedded test; BSP for STM32 NUCLEO-C031C6
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005 Quantum Leaps, <state-machine.com>.
*
* SPDX-License-Identifier: MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
============================================================================*/
#include "et.h" /* ET: embedded test */

#include "stm32c0xx.h"  /* CMSIS-compliant header file for the MCU used */
/* add other drivers if necessary... */

/* Local-scope objects -----------------------------------------------------*/
/* LED pins available on the board (just one user LED LD2--Green on PA.5) */
#define LD4_PIN  5U

/* USART2 pins PA.2 and PA.3 */
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

/* Button pins available on the board (just one user Button B1 on PC.13) */
#define B1_PIN   13U

/* USART2 for ET output */
#define UART_DIV_SAMPLING16(__PCLK__, __BAUD__, __CLOCKPRESCALER__) \
  ((((__PCLK__)/UARTPrescTable[(__CLOCKPRESCALER__)]) \
  + ((__BAUD__)/2U)) / (__BAUD__))

static uint16_t const UARTPrescTable[12] = {
    1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};


/*..........................................................................*/
void ET_onInit(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* enable GPIOA clock port for the LD4 User LED */
    RCC->IOPENR |= (1U << 0U);

    /* configure LD4 pin as push-pull output, no pull-up, pull-down */
    GPIOA->MODER   &= ~((3U << 2U*LD4_PIN));
    GPIOA->MODER   |=  ((1U << 2U*LD4_PIN));
    GPIOA->OTYPER  &= ~((1U <<    LD4_PIN));
    GPIOA->OSPEEDR &= ~((3U << 2U*LD4_PIN));
    GPIOA->OSPEEDR |=  ((1U << 2U*LD4_PIN));
    GPIOA->PUPDR   &= ~((3U << 2U*LD4_PIN));

    /* enable peripheral clock for USART2 */
    RCC->APBENR1  |= (1U << 17U); /* enable USART#2 clock */
    RCC->IOPENR   |= (1U << 0U);  /* enable GPIOA clock for UART pins */

    /* Configure PA3 to USART2_RX, PA2 to USART2_TX */
    GPIOA->AFR[0] &= ~((15U << 4U*USART2_RX_PIN) | (15U << 4U*USART2_TX_PIN));
    GPIOA->AFR[0] |=  (( 1U << 4U*USART2_RX_PIN) | ( 1U << 4U*USART2_TX_PIN));
    GPIOA->MODER  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->MODER  |=  (( 2U << 2U*USART2_RX_PIN) | ( 2U << 2U*USART2_TX_PIN));
    GPIOA->OSPEEDR&= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->OTYPER &= ~(( 1U <<    USART2_RX_PIN) | ( 1U <<    USART2_TX_PIN));
    GPIOA->PUPDR  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->PUPDR  |=  (( 1U << 2U*USART2_RX_PIN) | ( 1U << 2U*USART2_TX_PIN));

    USART2->BRR  = UART_DIV_SAMPLING16(SystemCoreClock, 115200U, 0U);
    USART2->CR1  = USART_CR1_TE_Msk | USART_CR1_RE_Msk
                   | USART_CR1_RXNEIE_RXFNEIE_Msk
                   | USART_CR1_UE_Msk;
    USART2->CR2  = 0x00000000U;
    USART2->CR3  = 0x00000000U;
}
/*..........................................................................*/
void ET_onPrintChar(char const ch) {
    while ((USART2->ISR & (1U << 7U)) == 0U) { /* while TXE not empty */
    }
    USART2->TDR = ch; /* put ch into the DR register */
}
/*..........................................................................*/
void ET_onExit(int err) {
    (void)err;
    /* blink the on-board LED2... */
    for (;;) {
        unsigned volatile ctr;
        GPIOA->BSRR = (1U << LD4_PIN); /* LED2 on */
        for (ctr = 100000U; ctr != 0U; --ctr) {}
        GPIOA->BSRR = (1U << (LD4_PIN + 16U)); /* LED2 off */
        for (ctr = 200000U; ctr != 0U; --ctr) {}
    }
}

/*..........................................................................*/
/* fault handler called from the exception handlers in the startup code */
void assert_failed(char const * const module, int const loc) {
    (void)module;
    (void)loc;
    ET_onExit(-1);
}
