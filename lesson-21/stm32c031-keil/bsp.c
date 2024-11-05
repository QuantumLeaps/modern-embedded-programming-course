/* Board Support Package (BSP) for the STM32 NUCLEO-C031C6 board */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */

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

static uint32_t volatile l_tickCtr;

/* ISRs  ===============================================*/
void SysTick_Handler(void) {
    ++l_tickCtr;
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
    RCC->IOPENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    __enable_irq();
}

uint32_t BSP_tickCtr(void) {
    uint32_t tickCtr;

    __disable_irq();
    tickCtr = l_tickCtr;
    __enable_irq();

    return tickCtr;
}

void BSP_delay(uint32_t ticks) {
    uint32_t start = BSP_tickCtr();
    while ((BSP_tickCtr() - start) < ticks) {
    }
}

/*..........................................................................*/
void BSP_ledRedOn(void) {
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
}

void BSP_ledRedOff(void) {
    GPIOA->BSRR = (1U << (LD5_PIN + 16U));  // turn LD5 off
}

void BSP_ledBlueOn(void) {
    //GPIOA->BSRR = (1U << LD4_PIN);  // turn LD4 on
}

void BSP_ledBlueOff(void) {
    //GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LD4 off
}

void BSP_ledGreenOn(void) {
    GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on
}

void BSP_ledGreenOff(void) {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LD4 off
}


//............................................................................
_Noreturn void assert_failed(char const * const module, int const id);
_Noreturn void assert_failed(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
