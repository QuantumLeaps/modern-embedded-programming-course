/*****************************************************************************
* BSP for EK-TM4C123GXL with QP/C framework
*****************************************************************************/
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

#include "TM4C123GH6PM_QL.h"        /* the device specific header (TI) */
/* add other drivers if necessary... */

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

#ifdef Q_SPY

    #define UART_BAUD_RATE      115200U
    #define UART_FR_TXFE        (1U << 7)
    #define UART_FR_RXFE        (1U << 4)
    #define UART_TXFIFO_DEPTH   16U

#endif

/* Assertion handler  ======================================================*/
Q_NORETURN Q_onAssert(char const * module, int_t id) {
    /* TBD: Perform corrective actions and damage control
    * SPECIFIC to your particular system.
    */
    (void)module;   /* unused parameter */
    (void)id;       /* unused parameter */
    GPIOF_AHB->DATA_Bits[LED_RED | LED_GREEN | LED_BLUE] = 0xFFU; /* all ON */
#ifndef NDEBUG /* debug build? */
    while (1) { /* tie the CPU in this endless loop */
    }
#endif
    NVIC_SystemReset(); /* reset the CPU */
}
//............................................................................
/* assert-handling function called by exception handlers in the startup code */
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onAssert(module, id);
}


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
    current = ~GPIOF_AHB->DATA_Bits[BTN_SW1 | BTN_SW2]; /* read SW1 & SW2 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & BTN_SW1) != 0U) {  /* debounced SW1 state changed? */
        if ((buttons.depressed & BTN_SW1) != 0U) { /* is SW1 depressed? */
            /* post the "button-pressed" event from ISR */
            static QEvt const buttonPressedEvt
                              = QEVT_INITIALIZER(BUTTON_PRESSED_SIG);
            QACTIVE_POST(AO_TimeBomb, &buttonPressedEvt, 0U);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const buttonReleasedEvt
                              = QEVT_INITIALIZER(BUTTON_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &buttonReleasedEvt, 0U);
        }
    }
    if ((tmp & BTN_SW2) != 0U) {  /* debounced SW2 state changed? */
        if ((buttons.depressed & BTN_SW2) != 0U) { /* is SW2 depressed? */
            /* post the "button-pressed" event from ISR */
            static QEvt const button2PressedEvt
                              = QEVT_INITIALIZER(BUTTON2_PRESSED_SIG);
            QACTIVE_POST(AO_TimeBomb, &button2PressedEvt, 0U);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const button2ReleasedEvt
                              = QEVT_INITIALIZER(BUTTON2_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &button2ReleasedEvt, 0U);
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
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

