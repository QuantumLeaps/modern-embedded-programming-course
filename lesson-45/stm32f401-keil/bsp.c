/*****************************************************************************
* BSP for STM32 NUCLEO-F401RE with QP/C framework
*****************************************************************************/
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

#include "stm32f4xx.h"  /* CMSIS-compliant header file for the MCU used */
/* add other drivers if necessary... */

#include <stdio.h> /* for printf() */

// Local-scope defines -----------------------------------------------------
// LED pins available on the board (just one user LED LD2--Green on PA.5)
#define LD2_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

// Button pins available on the board (just one user Button B1 on PC.13)
#define B1_PIN   13U


/* Error/Assertions  =======================================================*/
/* assertion-handling function */
Q_NORETURN Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
#ifndef NDEBUG /* debug build? */
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
    while (loc != 0) { /* tie the CPU in this endless loop */
    }
#endif
    NVIC_SystemReset(); /* reset the CPU */
}
//............................................................................
// assert-handling function called by exception handlers in the startup code
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onAssert(module, id);
}


// ISRs  ======================================================================
void SysTick_Handler(void) {
    QTIMEEVT_TICK_X(0U, &l_SysTick_Handler); // time events at rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };

    uint32_t current = ~GPIOC->IDR; // read Port C with state of Button B1
    uint32_t tmp = buttons.depressed; // save the depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

    if ((tmp & (1U << B1_PIN)) != 0U) { // debounced B1 state changed?
        if ((current & (1U << B1_PIN)) != 0U) { // is B1 depressed?
            /* post the "button-pressed" event from ISR */
            static QEvt const pressEvt = QEVT_INITIALIZER(BUTTON_PRESSED_SIG);
            QACTIVE_POST(AO_TimeBomb, &pressEvt, 0U);
            printf("BTN %s %d\n", "SW1", 1);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const releaseEvt = QEVT_INITIALIZER(BUTTON_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &releaseEvt, 0U);
            printf("BTN %s %d\n", "SW1", 0);
        }
    }
}
//............................................................................
void QV_onIdle(void) {
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M MCU.
    */
    QV_CPU_SLEEP();  /* atomically go to sleep and enable interrupts */
#else
    QF_INT_ENABLE(); /* just enable interrupts */
#endif
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    // enable GPIOA clock port for the LED LD2
    RCC->AHB1ENR |= (1U << 0U);

    // NUCLEO-F401RE board has LED LD2 on GPIOA pin LD2_PIN
    // and external LED LD5 on GPIO LD5_PIN
    // set the LED pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2U*LD2_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->MODER   |=  ((1U << 2U*LD2_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->OTYPER  &= ~((1U <<    LD2_PIN) | (1U <<    LD5_PIN));
    GPIOA->OSPEEDR &= ~((3U << 2U*LD2_PIN) | (3U << 2U*LD5_PIN));
    GPIOA->OSPEEDR |=  ((1U << 2U*LD2_PIN) | (1U << 2U*LD5_PIN));
    GPIOA->PUPDR   &= ~((3U << 2U*LD2_PIN) | (3U << 2U*LD5_PIN));

    // enable GPIOC clock port for the Button B1
    RCC->AHB1ENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);


}
//............................................................................
void QF_onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(SysTick_IRQn,  QF_AWARE_ISR_CMSIS_PRI + 1U);
    /* ... */

    /* enable IRQs in the NVIC... */
    /* ... */
}
//............................................................................
void QF_onCleanup(void) {
}

//............................................................................
void BSP_ledRedOn(void) {
    GPIOA->BSRR = (1U << LD2_PIN); // turn LD2 on
    printf("LED-%s is %d\n", "red", 1); /* -> "LED-red is 1\n" */
}
//............................................................................
void BSP_ledRedOff(void) {
    GPIOA->BSRR = (1U << (LD2_PIN + 16U)); // turn LD2 off
    printf("LED-%s is %d\n", "red", 0);
}
//............................................................................
void BSP_ledBlueOn(void) {
    //GPIOA->BSRR = (1U << LD2_PIN);  // turn LD2 on
    printf("LED-%s is %d\n", "blue", 1);
}
//............................................................................
void BSP_ledBlueOff(void) {
    //GPIOA->BSRR = (1U << (LD2_PIN + 16U));  // turn LD2 off
    printf("LED-%s is %d\n", "blue", 0);
}
//............................................................................
void BSP_ledGreenOn(void) {
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
    printf("LED-%s is %d\n", "green", 1);
}
//............................................................................
void BSP_ledGreenOff(void) {
    GPIOA->BSRR = (1U << (LD5_PIN + 16U));  // turn LD5 off
    printf("LED-%s is %d\n", "green", 0);
}

// support for printf() ======================================================
int fputc(int c, FILE *stream) {
    (void)stream; // unused parameter
    ITM_SendChar(c); // sent c to the ITM
    return c;
}

