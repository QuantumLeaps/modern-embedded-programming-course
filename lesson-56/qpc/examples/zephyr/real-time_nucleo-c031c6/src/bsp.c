//============================================================================
// BSP for "real-time" Example, NUCLEO-C031C6 board, Zephyr RTOS
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

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
// add other drivers if necessary...

#ifdef Q_SPY
    #error QP/Spy software tracing not available in this application
#endif

Q_DEFINE_THIS_FILE  // define the name of this file for assertions

// Local-scope objects -----------------------------------------------------

// test pins on GPIO PA (output)
#define TST1_PIN  7U
#define TST2_PIN  6U
#define TST3_PIN  4U
#define TST4_PIN  1U
#define TST5_PIN  0U
#define TST6_PIN  9U
#define TST7_PIN  5U // LED LD2-Green

// Button pins available on the board (just one user Button B1 on PC.13)
// button on GPIO PC (input)
#define B1_PIN    13U

//============================================================================
// Error handler

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS
    Q_PRINTK("\nERROR in %s:%d\n", module, id);

#ifndef NDEBUG
    k_panic(); // debug build: halt the system for error search...
#endif

    sys_reboot(SYS_REBOOT_COLD); // release build: reboot the system
    for (;;) { // explicitly "no-return"
    }
}
//............................................................................
// assertion failure handler for the STM32 library, including the startup code
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//............................................................................
static void custom_tick_callback(struct k_timer *tid); // prototype
static void custom_tick_callback(struct k_timer *tid) {
    Q_UNUSED_PAR(tid);

    BSP_d1on();

    QTIMEEVT_TICK_X(0U, (void *)0);

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
            QACTIVE_POST(AO_Sporadic2, &sporadicA.super, (void *)0);
            QACTIVE_POST(AO_Sporadic2, &sporadicB.super, (void *)0);
        }
        else { // B1 is released
            QACTIVE_POST(AO_Periodic4, BSP_getEvtPeriodic4(0U), (void *)0);
            QACTIVE_POST(AO_Periodic1, BSP_getEvtPeriodic1(0U), (void *)0);
        }
    }

    BSP_d1off();
}

// BSP functions =============================================================
void BSP_init(void) {
    // NOTE: the CPU clock is configured in Zephyr (48MHz)

    // Configure the MPU to prevent NULL-pointer dereferencing ...
    MPU->RBAR = 0x0U                          // base address (NULL)
                | MPU_RBAR_VALID_Msk          // valid region
                | (MPU_RBAR_REGION_Msk & 7U); // region #7
    MPU->RASR = (7U << MPU_RASR_SIZE_Pos)     // 2^(7+1) region
                | (0x0U << MPU_RASR_AP_Pos)   // no-access region
                | MPU_RASR_ENABLE_Msk;        // region enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk       // enable background region
                | MPU_CTRL_ENABLE_Msk;        // enable the MPU
    __ISB();
    __DSB();

    // enable GPIO port PA clock
    RCC->IOPENR |= (1U << 0U);

    // set all used GPIOA pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER &=
        ~((3U << 2U*TST1_PIN) | (3U << 2U*TST2_PIN) | (3U << 2U*TST3_PIN) |
          (3U << 2U*TST4_PIN) | (3U << 2U*TST5_PIN) | (3U << 2U*TST6_PIN) |
          (3U << 2U*TST7_PIN));
    GPIOA->MODER |=
         ((1U << 2U*TST1_PIN) | (1U << 2U*TST2_PIN) | (1U << 2U*TST3_PIN) |
          (1U << 2U*TST4_PIN) | (1U << 2U*TST5_PIN) | (1U << 2U*TST6_PIN) |
          (1U << 2U*TST7_PIN));
    GPIOA->OTYPER &=
        ~((1U <<    TST1_PIN) | (1U <<    TST2_PIN) | (1U <<    TST3_PIN) |
          (1U <<    TST4_PIN) | (1U <<    TST5_PIN) | (1U <<    TST6_PIN) |
          (1U <<    TST7_PIN));
    GPIOA->PUPDR &=
        ~((3U << 2U*TST1_PIN) | (3U << 2U*TST2_PIN) | (3U << 2U*TST3_PIN) |
          (3U << 2U*TST4_PIN) | (3U << 2U*TST5_PIN) | (3U << 2U*TST6_PIN) |
          (3U << 2U*TST7_PIN));

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Button B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);
}
//............................................................................
void BSP_start(void) {
    // instantiate and start QP/C active objects...
    Periodic1_ctor();
    static QEvtPtr periodic1QSto[10]; // Event queue storage
    static K_THREAD_STACK_DEFINE(periodic1Stack, 512);
    QActive_start(
        AO_Periodic1,          // AO pointer to start
        Q_PRIO(1U, 4U),        // QF-prio/Zephyr-prio
        periodic1QSto,         // storage for the AO's queue
        Q_DIM(periodic1QSto),  // queue length
        periodic1Stack,        // stack storage (needed for FreeRTOS)
        K_THREAD_STACK_SIZEOF(periodic1Stack), // stack size [Zephyr]
        BSP_getEvtPeriodic1(0U)); // initialization param

    Sporadic2_ctor();
    static QEvtPtr sporadic2QSto[8]; // Event queue storage
    static K_THREAD_STACK_DEFINE(sporadic2Stack, 512);
    QActive_start(
        AO_Sporadic2,          // AO pointer to start
        Q_PRIO(2U, 3U),        // QF-prio/Zephyr-prio
        sporadic2QSto,         // storage for the AO's queue
        Q_DIM(sporadic2QSto),  // queue length
        sporadic2Stack,        // stack storage (needed for FreeRTOS)
        K_THREAD_STACK_SIZEOF(sporadic2Stack), // stack size [Zephyr]
        (void const *)0);      // initialization param -- not used

    Sporadic3_ctor();
    static QEvtPtr sporadic3QSto[8]; // Event queue storage
    static K_THREAD_STACK_DEFINE(sporadic3Stack, 512);
    QActive_start(
        AO_Sporadic3,          // AO pointer to start
        Q_PRIO(3U, 2U),        // QF-prio/Zephyr-prio
        sporadic3QSto,         // storage for the AO's queue
        Q_DIM(sporadic3QSto),  // queue length
        sporadic3Stack,        // stack storage (needed for FreeRTOS)
        K_THREAD_STACK_SIZEOF(sporadic3Stack), // stack size [Zephyr]
        (void const *)0);      // initialization param -- not used

    Periodic4_ctor();
    static QEvtPtr periodic4QSto[8]; // Event queue storage
    static K_THREAD_STACK_DEFINE(periodic4Stack, 512);
    QActive_start(
        AO_Periodic4,          // AO pointer to start
        Q_PRIO(4U, 1U),        // QF-prio/Zephyr-prio
        periodic4QSto,         // storage for the AO's queue
        Q_DIM(periodic4QSto),  // queue length
        periodic4Stack,        // stack storage (needed for FreeRTOS)
        K_THREAD_STACK_SIZEOF(periodic4Stack), // stack size [Zephyr]
        BSP_getEvtPeriodic4(0U)); // initialization event
}
//............................................................................
void BSP_d1on(void)  { GPIOA->BSRR = (1U << TST1_PIN); __NOP(); __NOP(); }
void BSP_d1off(void) { GPIOA->BSRR = (1U << (TST1_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d2on(void)  { GPIOA->BSRR = (1U << TST2_PIN); __NOP(); __NOP(); }
void BSP_d2off(void) { GPIOA->BSRR = (1U << (TST2_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d3on(void)  { GPIOA->BSRR = (1U << TST3_PIN); __NOP(); __NOP(); }
void BSP_d3off(void) { GPIOA->BSRR = (1U << (TST3_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d4on(void)  { GPIOA->BSRR = (1U << TST4_PIN); __NOP(); __NOP(); }
void BSP_d4off(void) { GPIOA->BSRR = (1U << (TST4_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d5on(void)  { GPIOA->BSRR = (1U << TST5_PIN); __NOP(); __NOP(); }
void BSP_d5off(void) { GPIOA->BSRR = (1U << (TST5_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d6on(void)  { GPIOA->BSRR = (1U << TST6_PIN); __NOP(); __NOP(); }
void BSP_d6off(void) { GPIOA->BSRR = (1U << (TST6_PIN + 16U)); __NOP(); __NOP(); }
//............................................................................
void BSP_d7on(void)  { GPIOA->BSRR = (1U << TST7_PIN); __NOP(); __NOP(); }
void BSP_d7off(void) { GPIOA->BSRR = (1U << (TST7_PIN + 16U)); __NOP(); __NOP(); }

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
    static struct k_timer zephyr_tick_timer;
    k_timer_init(&zephyr_tick_timer, &custom_tick_callback, NULL);
    k_timer_start(&zephyr_tick_timer, K_TICKS(1), K_TICKS(1));
}
//............................................................................
void QF_onCleanup(void) {
}
//............................................................................
#ifdef QF_IDLE
void QF_onIdle(void) {
    BSP_d7on();
    BSP_d7off();
}
#endif
