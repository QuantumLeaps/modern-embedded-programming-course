/* Board Support Package (BSP) for the STM32 NUCLEO-C031C6 board */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */
#include "sched.h"

#include "bsp.h"
#include "stm32c0xx.h"  // CMSIS-compliant header file for the MCU used
/* add other drivers if necessary... */

// Local-scope defines -----------------------------------------------------
// LED pins available on the board (just one user LED LD4--Green on PA.5)
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

// Button pins available on the board (just one user Button B1 on PC.13)
#define B1_PIN   13U

uint32_t volatile ready_set;

/* ISRs  ===================================================================*/
void EXTI4_15_IRQHandler(void) {
    uint32_t pins = EXTI->FPR1;
    if (pins & (1U << B1_PIN)) {
        EXTI->FPR1 = (1U << B1_PIN); // clear the interrupt source
        sched_post(TASK_AIRBAG_PRIO);
    }
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    // enable GPIOA clock port for the LED LD4
    RCC->IOPENR |= (1U << 0U);

    // NUCLEO-C031C6 board has LED LD4 on GPIOA pin LD4_PIN
    // and external LED LD5 on GPIO LD5_PIN
    // set the LED pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->MODER   |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->OTYPER  &= ~((1U <<    LD4_PIN) | (1U <<    LD5_PIN));
    GPIOA->OSPEEDR &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->OSPEEDR |=  ((1U << 2U*LD4_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->PUPDR   &= ~((3U << 2U*LD4_PIN) | (3U << 2U*LD5_PIN));

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |= (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN); // pin mode
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);

    // configure EXTI line for interrupt on Button B1
    EXTI->EMR1  &= ~(1U << B1_PIN); // disable event for B1
    EXTI->IMR1  |=  (1U << B1_PIN); // enable EXIT line for B1
    EXTI->RTSR1 &= ~(1U << B1_PIN); // disable raising edge trigger
    EXTI->FTSR1 |=  (1U << B1_PIN); // enable falling edge trigger
    EXTI->EXTICR[3] = (EXTI->EXTICR[3] & (~(7U << 8U))) | (2U << 8U);

    // enable the EXIT IRQ
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void BSP_deployAirbag(void) {
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LD4 on
}

void BSP_engageABS(void) {
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
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
    // turn LD4 and LD5 on
    GPIOA->BSRR = ((1U << LD4_PIN) | (1U << LD5_PIN));
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
