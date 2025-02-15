/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */
#include "sched.h"

#include "bsp.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

// Buttons on the board (GPIOF)
#define BTN_SW1   (1U << 4U)
#define BTN_SW2   (1U << 0U)

/* ISRs  ===================================================================*/
void GPIOPortF_IRQHandler(void); // prototype
void GPIOPortF_IRQHandler(void) {
    uint32_t pins = GPIOF_AHB->MIS;
    GPIOF_AHB->ICR = 0xFFU; // clear interrupt sources
    if ((pins & BTN_SW1) != 0U) {
        sched_post(TASK_AIRBAG_PRIO);
    }
    if ((pins & BTN_SW2) != 0U) {
        sched_post(TASK_ABS_PRIO);
    }
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    // configure switches...
    // unlock access to the SW2 pin because it is PROTECTED
    GPIOF_AHB->LOCK = 0x4C4F434BU; // unlock GPIOCR register for SW2
    // commit the write (cast const away)
    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x01U;

    GPIOF_AHB->DIR &= ~(BTN_SW1 | BTN_SW2); // input pin
    GPIOF_AHB->DEN |= (BTN_SW1 | BTN_SW2); // digital enable
    GPIOF_AHB->PUR |= (BTN_SW1 | BTN_SW2); // pull-up resistor enable

    *(uint32_t volatile *)&GPIOF_AHB->CR = 0x00U;
    GPIOF_AHB->LOCK = 0x0; // lock GPIOCR register for SW2

    // GPIO interrupt setup for SW1 and SW2
    GPIOF_AHB->IS  &= ~(BTN_SW1 | BTN_SW2); // edge detect
    GPIOF_AHB->IBE &= ~(BTN_SW1 | BTN_SW2); // only one edge generate the interrupt
    GPIOF_AHB->IEV &= ~(BTN_SW1 | BTN_SW2); // a falling edge triggers the interrupt
    GPIOF_AHB->IM  |= (BTN_SW1 | BTN_SW2);  // enable GPIOF interrupt

    // enable IRQs in the NVIC...
    NVIC_EnableIRQ(GPIOF_IRQn);
    // ...
}

void BSP_deployAirbag(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void BSP_engageABS(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

uint32_t goToSleepCtr = 0;
void BSP_goToSleep() {
    ++goToSleepCtr;
    // disable clock to the unused peripherals
    // setup the desired sleep mode
    __WFI(); // wait for interrupt; EXECUTION STOPS!
    // enable clock to the used peripherals
    __enable_irq(); // enable interrupts with PRIMASK
}

void BSP_intDisable(void) {
    __disable_irq();
}

void BSP_intEnable(void) {
    __enable_irq();
}

//............................................................................
_Noreturn void assert_failed(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    // light up all LEDs
    GPIOF_AHB->DATA_Bits[LED_GREEN | LED_RED | LED_BLUE] = 0xFFU;
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
