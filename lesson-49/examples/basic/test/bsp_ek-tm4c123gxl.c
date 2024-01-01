/*============================================================================
* ET: embedded test; BSP for EK-TM4C123GXL
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

#include "TM4C123GH6PM.h"        /* the device specific header (TI) */
/* add other drivers if necessary... */

//DBC_MODULE_NAME("bsp_ek-tm4c123gxl")

/* Local-scope objects -----------------------------------------------------*/
/* LEDs on the board */
#define LED_RED      (1U << 1)
#define LED_GREEN    (1U << 3)
#define LED_BLUE     (1U << 2)

/* Buttons on the board */
#define BTN_SW1      (1U << 4)
#define BTN_SW2      (1U << 0)

/* Test pins */
#define PD0_PIN      (1U << 0)
#define PD1_PIN      (1U << 1)

/* UART for testing */
#define UART_BAUD_RATE      115200U
#define UART_FR_TXFE        (1U << 7)
#define UART_FR_RXFE        (1U << 4)
#define UART_BUSY           (1U << 3)
#define UART_TXFF           (1U << 5)
#define UART_TXFIFO_DEPTH   16U

/*..........................................................................*/
void ET_onInit(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* enable clock for to the peripherals used by this application... */
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    __ISB();
    __DSB();

    /* configure LEDs (digital output) */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    /* configure switches... */

    /* unlock access to the SW2 pin because it is PROTECTED */
    GPIOF_AHB->LOCK = 0x4C4F434BU; /* unlock GPIOCR register for SW2 */
    /* commit the write (cast const away) */
    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x01U;

    GPIOF_AHB->DIR &= ~(BTN_SW1 | BTN_SW2); /* input */
    GPIOF_AHB->DEN |= (BTN_SW1 | BTN_SW2); /* digital enable */
    GPIOF_AHB->PUR |= (BTN_SW1 | BTN_SW2); /* pull-up resistor enable */

    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x00U;
    GPIOF_AHB->LOCK = 0x0; /* lock GPIOCR register for SW2 */

    /* enable clock for UART0 and GPIOA (used by UART0 pins) */
    SYSCTL->RCGCUART   |= (1U << 0); /* enable Run mode for UART0 */
    SYSCTL->RCGCGPIO   |= (1U << 0); /* enable Run mode for GPIOA */

    /* configure UART0 pins for UART operation */
    uint32_t tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->AMSEL &= ~tmp;  /* disable analog function on the pins */
    GPIOA->AFSEL |= tmp;   /* enable ALT function on the pins */
    GPIOA->DEN   |= tmp;   /* enable digital I/O on the pins */
    GPIOA->PCTL  &= ~0x00U;
    GPIOA->PCTL  |= 0x11U;

    /* configure the UART for the desired baud rate, 8-N-1 operation */
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD  = tmp / 64U;
    UART0->FBRD  = tmp % 64U;
    UART0->LCRH  = (0x3U << 5); /* configure 8-N-1 operation */
    UART0->CTL   = (1U << 0)    /* UART enable */
                    | (1U << 8)  /* UART TX enable */
                    | (1U << 9)  /* UART RX enable */
                    | (1U << 4); /* EOT end of transmission */
}
/*..........................................................................*/
void ET_onPrintChar(char const ch) {
    /* busy-wait as long as UART busy */
    while ((UART0->FR & UART_BUSY) != 0) {
    }
    UART0->DR = ch; /* write the byte into Data Register */
}
/*..........................................................................*/
void ET_onExit(int err) {
    (void)err;
    /* turn all LEDs off */
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;
    /* blink the on-board LED2... */
    for (;;) {
        unsigned volatile ctr;
        GPIOF_AHB->DATA_Bits[LED_GREEN] ^= LED_GREEN;
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
