//============================================================================
// BSP for "real-time" Example
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"                 // QP/C real-time event framework
#include "bsp.h"                 // Board Support Package
#include "app.h"                 // Application interface

#include "TM4C123GH6PM.h"        /* the device specific header (TI) */
/* add other drivers if necessary... */

Q_DEFINE_THIS_FILE  // define the name of this file for assertions

/* Local-scope defines -----------------------------------------------------*/

/* test pins on GPIOF */
#define TST1_PIN  (1U << 1U) /* LED Red */
#define TST3_PIN  (1U << 2U) /* LED Blue */
#define TST7_PIN  (1U << 3U) /* LED Green */

/* test pins on GPIOD */
#define TST4_PIN  (1U << 0U)
#define TST5_PIN  (1U << 1U)
#define TST6_PIN  (1U << 2U)

/* Buttons on GPIOF */
#define BTN_SW1   (1U << 4)
#define BTN_SW2   (1U << 0)

//============================================================================
// Error handler and ISRs...

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

#ifndef NDEBUG
    // light up the user LED
    GPIOF_AHB->DATA_Bits[TST1_PIN | TST3_PIN | TST7_PIN] = 0xFFU; /* all ON */

    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif

    NVIC_SystemReset();
}
//............................................................................
// assertion failure handler for the STM32 library, including the startup code
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//............................................................................
#ifdef __UVISION_VERSION
// dummy initialization of the ctors (not used in C)
void _init(void);
void _init(void) {
}
#endif // __UVISION_VERSION

// ISRs used in the application ============================================

void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    BSP_d1on();

    QTIMEEVT_TICK_X(0U, &l_SysTick_Handler); // time events at rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };

    /* read SW1 & SW2 */
    uint32_t current = ~GPIOF_AHB->DATA_Bits[BTN_SW1 | BTN_SW2];
    uint32_t tmp = buttons.depressed; // save the depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

    if ((tmp & BTN_SW1) != 0U) {  /* debounced SW1 state changed? */
        if ((buttons.depressed & BTN_SW1) != 0U) { /* is SW1 depressed? */
            // immutable sporadic-press event
            static SporadicSpecEvt const sporadicA = {
                QEVT_INITIALIZER(SPORADIC_A_SIG),
                .toggles = 189U,
            };
            // immutable forward-press event
            static SporadicSpecEvt const sporadicB = {
                QEVT_INITIALIZER(SPORADIC_B_SIG),
                .toggles = 89U,
            };
            QACTIVE_POST(AO_Sporadic2, &sporadicA.super, &l_SysTick_Handler);
            QACTIVE_POST(AO_Sporadic2, &sporadicB.super, &l_SysTick_Handler);
        }
        else { /* the button is released */
            QACTIVE_POST(AO_Periodic4, BSP_getEvtPeriodic4(0U), &l_SysTick_Handler);
            QACTIVE_POST(AO_Periodic1, BSP_getEvtPeriodic1(0U), &l_SysTick_Handler);
        }
    }

    QV_ARM_ERRATUM_838869();

    BSP_d1off();
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* enable clock for to the peripherals used by this application... */
    SYSCTL->RCGCGPIO  |= (1U << 5U); /* enable Run mode for GPIOF */
    SYSCTL->RCGCGPIO  |= (1U << 3U); /* enable Run mode for GPIOD */
    SYSCTL->GPIOHBCTL |= (1U << 5U); /* enable AHB for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 3U); /* enable AHB for GPIOD */
    __ISB();
    __DSB();

    /* configure test pins on GPIOF (digital output) */
    GPIOF_AHB->DIR |= (TST1_PIN | TST3_PIN | TST7_PIN);
    GPIOF_AHB->DEN |= (TST1_PIN | TST3_PIN | TST7_PIN);

    /* configure button on GPIOF (digital input) */
    GPIOF_AHB->DIR &= ~(BTN_SW1); /* input */
    GPIOF_AHB->DEN |= (BTN_SW1); /* digital enable */
    GPIOF_AHB->PUR |= (BTN_SW1); /* pull-up resistor enable */

    /* configure test pins on GPIOD (digital output) */
    GPIOD_AHB->DIR |= (TST4_PIN | TST5_PIN | TST6_PIN);
    GPIOD_AHB->DEN |= (TST4_PIN | TST5_PIN | TST6_PIN);

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
//............................................................................
void BSP_start(void) {
    // instantiate and start QP/C active objects...
    Periodic1_ctor();
    static QEvtPtr periodic1QSto[10]; // Event queue storage
    QActive_start(
        AO_Periodic1,          // AO pointer to start
        1U,                    // QF-prio
        periodic1QSto,         // storage for the AO's queue
        Q_DIM(periodic1QSto),  // queue length
        (void *)0, 0U,         // stack storage, size (not used)
        BSP_getEvtPeriodic1(0U)); // initialization param

    Sporadic2_ctor();
    static QEvtPtr sporadic2QSto[8]; // Event queue storage
    QActive_start(
        AO_Sporadic2,          // AO pointer to start
        2U,                    // QF-prio
        sporadic2QSto,         // storage for the AO's queue
        Q_DIM(sporadic2QSto),  // queue length
        (void *)0, 0U,         // stack storage, size (not used)
        (void const *)0);      // initialization param -- not used

    Sporadic3_ctor();
    static QEvtPtr sporadic3QSto[8]; // Event queue storage
    QActive_start(
        AO_Sporadic3,          // AO pointer to start
        3U,                    // QF-prio
        sporadic3QSto,         // storage for the AO's queue
        Q_DIM(sporadic3QSto),  // queue length
        (void *)0, 0U,         // stack storage, size (not used)
        (void const *)0);      // initialization param -- not used

    Periodic4_ctor();
    static QEvtPtr periodic4QSto[8]; // Event queue storage
    QActive_start(
        AO_Periodic4,          // AO pointer to start
        4U,                    // QF-prio
        periodic4QSto,         // storage for the AO's queue
        Q_DIM(periodic4QSto),  // queue length
        (void *)0, 0U,         // stack storage, size (not used)
        BSP_getEvtPeriodic4(0U)); // initialization event
}
//............................................................................
QEvt const *BSP_getEvtPeriodic1(uint8_t num) {
    // immutable PERIODIC_SPEC events for Periodic1
    static PeriodicSpecEvt const periodicSpec1[] = {
        {
            QEVT_INITIALIZER(PERIODIC_SPEC_SIG),
            .toggles = 40U,
            .ticks = 5U,
        },
        {
            QEVT_INITIALIZER(PERIODIC_SPEC_SIG),
            .toggles = 30U,
            .ticks = 7U,
        }
    };
    Q_REQUIRE_ID(500, num < Q_DIM(periodicSpec1)); // must be in range
    return &periodicSpec1[num].super;
}
//............................................................................
QEvt const *BSP_getEvtPeriodic4(uint8_t num) {
    // immutable PERIODIC_SPEC events for Periodic4
    static PeriodicSpecEvt const periodicSpec4[] = {
        {
            QEVT_INITIALIZER(PERIODIC_SPEC_SIG),
            .toggles = 20U,
            .ticks = 2U,
        },
        {
            QEVT_INITIALIZER(PERIODIC_SPEC_SIG),
            .toggles = 10U,
            .ticks = 1U,
        },
    };
    Q_REQUIRE_ID(600, num < Q_DIM(periodicSpec4)); // must be in range
    return &periodicSpec4[num].super;
}

// QF callbacks ==============================================================
void QF_onStartup(void) {
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

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
void BSP_d1on(void) { /* LED-Red */
    GPIOF_AHB->DATA_Bits[TST1_PIN] = 0xFFU;
    /* don't use the FPU in the ISR */
}
void BSP_d1off(void) {
    GPIOF_AHB->DATA_Bits[TST1_PIN] = 0x00U;
}
/*..........................................................................*/
void BSP_d3on(void) { /* LED-Blue */
    GPIOF_AHB->DATA_Bits[TST3_PIN] = 0xFFU;
}
void BSP_d3off(void) {
    GPIOF_AHB->DATA_Bits[TST3_PIN] = 0x00U;
}
/*..........................................................................*/
void BSP_d4on(void) {
    GPIOD_AHB->DATA_Bits[TST4_PIN] = 0xFFU;
}
void BSP_d4off(void) {
    GPIOD_AHB->DATA_Bits[TST4_PIN] = 0x00U;
}
/*..........................................................................*/
void BSP_d5on(void) {
    GPIOD_AHB->DATA_Bits[TST5_PIN] = 0xFFU;
}
void BSP_d5off(void) {
    GPIOD_AHB->DATA_Bits[TST5_PIN] = 0x00U;
}
/*..........................................................................*/
void BSP_d6on(void) {
    GPIOD_AHB->DATA_Bits[TST6_PIN] = 0xFFU;
}
void BSP_d6off(void) {
    GPIOD_AHB->DATA_Bits[TST6_PIN] = 0x00U;
}
/*..........................................................................*/
void BSP_d7on(void) {  /* LED2-Green */
    GPIOF_AHB->DATA_Bits[TST7_PIN] = 0xFFU;
}
void BSP_d7off(void) {
    GPIOF_AHB->DATA_Bits[TST7_PIN] = 0x00U;
}
//............................................................................
void QF_onCleanup(void) {
}
//............................................................................
void QV_onIdle(void) { // CAUTION: called with interrupts DISABLED, see NOTE0
    BSP_d7on(); // LED LD2
#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    QV_CPU_SLEEP(); // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
    BSP_d7off();
}

//============================================================================
// NOTE0:
// The QV_onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV_onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
