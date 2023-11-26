/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include "qpc.h"
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

/* Assertion handler  ======================================================*/
Q_NORETURN Q_onAssert(char const * module, int_t id) {
    // TBD: Perform corrective actions and damage control
    // SPECIFIC to your particular system.
    //
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
void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QXK_ISR_ENTRY();  /* inform QXK about entering an ISR */

    QF_TICK_X(0U, (void *)0); /* process time events for rate 0 */

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}
/*..........................................................................*/
// NOTE:
// The CMSIS-comliant name of this ISR should be GPIOF_IRQHandler()
// (see https://arm-software.github.io/CMSIS_5/Core/html/startup_c_pg.html )
// However, the startup code provided the standard "TM4C123x Series CMSIS Pack"
// provides NON-compliant ISR names, such as GPIOF_Handler() used here.
void GPIOF_Handler(void); // prototype
void GPIOF_Handler(void) {
    QXK_ISR_ENTRY();  /* inform QXK about entering an ISR */
    if ((GPIOF_AHB->RIS & BTN_SW1) != 0U) { /* interrupt caused by SW1? */
        /* post an immutable "button-press" event from ISR */
        static QEvt const buttonPressedEvt
            = QEVT_INITIALIZER(BUTTON_PRESS_SIG);
        QACTIVE_POST(AO_Blinky2, &buttonPressedEvt, 0U);
    }
    GPIOF_AHB->ICR = 0xFFU; /* clear interrupt sources */
    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* enable clock for to the peripherals used by this application... */
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    __ISB();
    __DSB();

    /* configure LEDs (digital output) */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    /* configure switch SW1 */
    GPIOF_AHB->DIR &= ~BTN_SW1; /* input */
    GPIOF_AHB->DEN |= BTN_SW1; /* digital enable */
    GPIOF_AHB->PUR |= BTN_SW1; /* pull-up resistor enable */

    /* GPIO interrupt setup for SW1 */
    GPIOF_AHB->IS  &= ~BTN_SW1; /* edge detect for SW1 */
    GPIOF_AHB->IBE &= ~BTN_SW1; /* only one edge generate the interrupt */
    GPIOF_AHB->IEV &= ~BTN_SW1; /* a falling edge triggers the interrupt */
    GPIOF_AHB->IM  |= BTN_SW1;  /* enable GPIOF interrupt for SW1 */
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

/* callbacks ---------------------------------------------------------------*/
void QF_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set the interrupt priorities of "kernel aware" interrupts */
    NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI);
    NVIC_SetPriority(GPIOF_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1);

    /* enable IRQs in NVIC... */
    NVIC_EnableIRQ(GPIOF_IRQn);
}
/*..........................................................................*/
void QF_onCleanup(void) {
}

void QXK_onIdle(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
    //__WFI(); /* stop the CPU and Wait for Interrupt */
}

