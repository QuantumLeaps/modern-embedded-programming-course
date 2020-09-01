/*****************************************************************************
* BSP for EK-TM4C123GXL with uC/OS-II RTOS
*****************************************************************************/
#include "uc_ao.h"  /* uC/AO API */
#include "bsp.h"

#include <stdbool.h>             /* needed by the TI drivers */
#include "TM4C123GH6PM.h"        /* the device specific header (TI) */
#include "rom.h"                 /* the built-in ROM functions (TI) */
#include "sysctl.h"              /* system control driver (TI) */
#include "gpio.h"                /* GPIO driver (TI) */
/* add other drivers if necessary... */

/* Local-scope objects -----------------------------------------------------*/
/* LEDs on the board */
#define LED_RED      (1U << 1)
#define LED_GREEN    (1U << 3)
#define LED_BLUE     (1U << 2)

/* Buttons on the board */
#define BTN_SW1      (1U << 4)
#define BTN_SW2      (1U << 0)

/* uCOS-II application hooks ===============================================*/
void App_TimeTickHook(void) {
    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    TimeEvent_tick(); /* process all uC/AO time events */

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
            static Event const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            Active_post(AO_TimeBomb, &buttonPressedEvt);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static Event const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
            Active_post(AO_TimeBomb, &buttonReleasedEvt);
        }
    }
    if ((tmp & BTN_SW2) != 0U) {  /* debounced SW2 state changed? */
        if ((buttons.depressed & BTN_SW2) != 0U) { /* is SW2 depressed? */
            /* post the "button-pressed" event from ISR */
            static Event const button2PressedEvt = {BUTTON2_PRESSED_SIG};
            Active_post(AO_TimeBomb, &button2PressedEvt);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static Event const button2ReleasedEvt = {BUTTON2_RELEASED_SIG};
            Active_post(AO_TimeBomb, &button2ReleasedEvt);
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

    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */

    GPIOF->LOCK = 0x4C4F434B; /* unlock GPIOCR register for SW2 */
    *(uint32_t *)(&GPIOF->CR) = 0x01; /* commit the write */
    
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* configure switches */
    GPIOF_AHB->DIR &= ~(BTN_SW1 | BTN_SW2); /* input */
    GPIOF_AHB->DEN |= (BTN_SW1 | BTN_SW2); /* digital enable */
    GPIOF_AHB->PUR |= (BTN_SW1 | BTN_SW2); /* pull-up resistor enable */
}
/*..........................................................................*/
void BSP_start(void) {
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

/*..........................................................................*/
/* error-handling function called by exception handlers in the startup code */
void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    GPIOF_AHB->DATA_Bits[LED_RED | LED_GREEN | LED_BLUE] = 0xFFU; /* all ON */
#ifndef NDEBUG /* debug build? */
    while (loc != 0) { /* tie the CPU in this endless loop */
    }
#endif
    NVIC_SystemReset(); /* reset the CPU */
}
