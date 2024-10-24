/*****************************************************************************
* BSP for NUCLEO-C031C6 with uC/OS-II RTOS
*****************************************************************************/
#include "ucos_ii.h"  /* uC/OS-II API, port and compile-time configuration */
#include "qassert.h"  /* embedded-system-friendly assertions */
#include "bsp.h"      /* Board Support Package */

#include <stdbool.h>             /* needed by the TI drivers */
#include "stm32c0xx.h"  /* CMSIS-compliant header file for the MCU used */
/* add other drivers if necessary... */

// LED marked "LD4" (PA.5) on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

// Button B1 (PC.13) on the NUCLEO-C031C6 board
#define B1_PIN   13U

/* uCOS-II application hooks ===============================================*/
void App_TimeTickHook(void) {
    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    current = ~GPIOC->IDR; // read Port C with state of Button B1
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & (1U << B1_PIN)) != 0U) { // debounced B1 state changed?
        if ((current & (1U << B1_PIN)) != 0U) { // is B1 depressed?
            /* post the "button-pressed" semaphore */
            OSSemPost(BSP_semaPress);
        }
        else { /* the button is released */
            /* post the "button-release" semaphore */
            OSSemPost(BSP_semaRelease);
        }
    }
}
/*..........................................................................*/
void App_TaskIdleHook(void) {
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    __WFI(); /* Wait-For-Interrupt */
#endif
}
/*..........................................................................*/
void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }


/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LEDs
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

    // configure Button B1 (PC.13) pins as input, no pull-up, pull-down
    GPIOC->MODER   &= ~(3U << 2*B1_PIN);
    GPIOC->OSPEEDR &= ~(3U << 2*B1_PIN);
    GPIOC->OSPEEDR |=  (1U << 2*B1_PIN);
    GPIOC->PUPDR   &= ~(3U << 2*B1_PIN);

    // configure Button B1 interrupt as falling edge
    EXTI->EMR1 &= ~(1U << B1_PIN);
    EXTI->IMR1 |= (1U << B1_PIN);
    EXTI->RTSR1 &= ~(1U << B1_PIN);
    EXTI->FTSR1 |= (1U << B1_PIN);
    EXTI->EXTICR[3] &= ~(7U << 8); // EXTI port C line 13
    EXTI->EXTICR[3] |= (2U << 8);  // EXTI port C line 13
}
/*..........................................................................*/
void BSP_start(void) {
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    * NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    */
    SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE1 */
    NVIC_SetPriority(SysTick_IRQn,  CPU_CFG_KA_IPL_BOUNDARY + 1U);
    /* ... */

    /* enable IRQs in the NVIC... */
    /* ... */
}

/*..........................................................................*/
void BSP_ledRedOn(void) {
    /* no red LED on board */
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    /* no red LED on board */
}
/*..........................................................................*/

void BSP_ledBlueOn(void) {
    /* no blue LED on board */
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    /* no blue LED on board */
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOA->BSRR = (1U << LD4_PIN);  /* turn LED on  */
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U));  /* turn LED off */
}

//............................................................................
Q_NORETURN Q_onAssert(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    GPIOA->BSRR = (1U << LD4_PIN);  // turn LED on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
// error-handling function called by exception handlers in the startup code
Q_NORETURN assert_failed(char const * const module, int const id);
Q_NORETURN assert_failed(char const * const module, int const id) {
    Q_onAssert(module, id);
}

