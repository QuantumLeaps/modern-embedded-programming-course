/*****************************************************************************
* BSP for STM32 NUCLEO-L152RE with QP/C framework
*****************************************************************************/
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

#include "stm32l1xx.h"  /* CMSIS-compliant header file for the MCU used */
/* add other drivers if necessary... */

#include <stdio.h> /* for printf() */

/* Local-scope objects -----------------------------------------------------*/
/* LED pins available on the board (just one user LED LD2--Green on PA.5) */
#define LED_LD2  (1U << 5)

/* Button pins available on the board (just one user Button B1 on PC.13) */
#define BTN_B1   (1U << 13)


/* ISRs  ===============================================*/
void SysTick_Handler(void) {
    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    QF_TICK_X(0U, (void *)0); /* process all QP/C time events */

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    current = ~GPIOC->IDR; /* read Port C with the state of Button B1 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & BTN_B1) != 0U) {  /* debounced B1 state changed? */
        if ((buttons.depressed & BTN_B1) != 0U) { /* is B1 depressed? */
            /* post the "button-pressed" event from ISR */
            static QEvt const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            QACTIVE_POST(AO_TimeBomb, &buttonPressedEvt, 0U);
            printf("BTN %s %d\n", "SW1", 1);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
            QACTIVE_POST(AO_TimeBomb, &buttonReleasedEvt, 0U);
            printf("BTN %s %d\n", "SW1", 0);
        }
    }
}
/*..........................................................................*/
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
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* enable GPIOA clock port for the LED LD2 */
    RCC->AHBENR |= (1U << 0);

    /* configure LED (PA.5) pin as push-pull output, no pull-up, pull-down */
    GPIOA->MODER   &= ~((3U << 2*5));
    GPIOA->MODER   |=  ((1U << 2*5));
    GPIOA->OTYPER  &= ~((1U <<   5));
    GPIOA->OSPEEDR &= ~((3U << 2*5));
    GPIOA->OSPEEDR |=  ((1U << 2*5));
    GPIOA->PUPDR   &= ~((3U << 2*5));

    /* enable GPIOC clock port for the Button B1 */
    RCC->AHBENR |=  (1U << 2);

    /* configure Button (PC.13) pins as input, no pull-up, pull-down */
    GPIOC->MODER   &= ~(3U << 2*13);
    GPIOC->OSPEEDR &= ~(3U << 2*13);
    GPIOC->OSPEEDR |=  (1U << 2*13);
    GPIOC->PUPDR   &= ~(3U << 2*13);

}
/*..........................................................................*/
void QF_onStartup(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    * NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE1 */
    NVIC_SetPriority(SysTick_IRQn,  QF_AWARE_ISR_CMSIS_PRI + 1U);
    /* ... */

    /* enable IRQs in the NVIC... */
    /* ... */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}

/*..........................................................................*/
void BSP_ledRedOn(void) {
    GPIOA->BSRRL |= LED_LD2;
    printf("LED-%s is %d\n", "red", 1); /* -> "LED-red is 1\n" */
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    GPIOA->BSRRH |= LED_LD2;
    printf("LED-%s is %d\n", "red", 0);
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    // LED not available
    printf("LED-%s is %d\n", "blue", 1);
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    // LED not available
    printf("LED-%s is %d\n", "blue", 0);
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    // LED not available
    printf("LED-%s is %d\n", "green", 1);
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    // LED not available
    printf("LED-%s is %d\n", "green", 0);
}

/*..........................................................................*/
/* error-handling function called by exception handlers in the startup code */
void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    GPIOA->BSRRL |= LED_LD2;
#ifndef NDEBUG /* debug build? */
    while (loc != 0) { /* tie the CPU in this endless loop */
    }
#endif
    NVIC_SystemReset(); /* reset the CPU */
}

/* support for printf() ====================================================*/
int fputc(int c, FILE *stream) {
    (void)stream; /* unused parameter */
    ITM_SendChar(c); /* sent c to the ITM */
    return c;
}

