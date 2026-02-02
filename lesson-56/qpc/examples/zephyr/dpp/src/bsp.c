//============================================================================
// Example, Zephyr RTOS kernel
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"                 // QP/C real-time event framework
#include "dpp.h"                 // DPP Application interface
#include "bsp.h"                 // Board Support Package

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
// add other drivers if necessary...

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)

Q_DEFINE_THIS_FILE

// Local-scope objects -----------------------------------------------------
static struct gpio_dt_spec const l_led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct k_timer zephyr_tick_timer;
static uint32_t l_rnd; // random seed

#ifdef Q_SPY

    // QSpy source IDs
    static QSpyId const timerID = { QS_ID_AP };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QS_USER,
        PAUSED_STAT,
    };

#endif

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
static void zephyr_tick_function(struct k_timer *tid); // prototype
static void zephyr_tick_function(struct k_timer *tid) {
    Q_UNUSED_PAR(tid);

    QTIMEEVT_TICK_X(0U, &timerID);
}

//============================================================================

void BSP_init(void) {
    int ret = gpio_pin_configure_dt(&l_led0, GPIO_OUTPUT_ACTIVE);
    Q_ASSERT(ret >= 0);

    k_timer_init(&zephyr_tick_timer, &zephyr_tick_function, (void *)0);

    BSP_randomSeed(1234U);

    // initialize the QS software tracing...
    if (QS_INIT((void *)0) == 0) {
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&timerID);

    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QS_GRP_ALL);  // all records
    QS_GLB_FILTER(-QS_QF_TICK); // exclude the clock tick
}
//............................................................................
void BSP_start(void) {
    // initialize event pools
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2*N_PHILO];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QSubscrList subscrSto[MAX_PUB_SIG];
    QActive_psInit(subscrSto, Q_DIM(subscrSto));

    // instantiate and start AOs/threads...

    static QEvtPtr philoQueueSto[N_PHILO][10];
    static K_THREAD_STACK_DEFINE(philoStack[N_PHILO], 512);
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        Philo_ctor(n);
        QActive_start(AO_Philo[n],
            n + 3U,                  // QF-prio. of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            philoStack[n],           // private stack for embOS
            K_THREAD_STACK_SIZEOF(philoStack[n]), // stack size [Zephyr]
            (void *)0);              // no initialization param
    }

    static QEvtPtr tableQueueSto[N_PHILO];
    static K_THREAD_STACK_DEFINE(tableStack, 512);
    Table_ctor();
    QActive_start(AO_Table,
        N_PHILO + 7U,            // QP prio. of the AO
        tableQueueSto,           // event queue storage
        Q_DIM(tableQueueSto),    // queue length [events]
        tableStack,              // private stack for embOS
        K_THREAD_STACK_SIZEOF(tableStack), // stack size [Zephyr]
        (void *)0);              // no initialization param
}
//............................................................................
void BSP_ledOn(void) {
    gpio_pin_set_dt(&l_led0, true);
}
//............................................................................
void BSP_ledOff(void) {
    gpio_pin_set_dt(&l_led0, false);
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') {
        BSP_ledOn();
    }
    else {
        BSP_ledOff();
    }
    Q_PRINTK("Philo[%d]->%s\n", n, stat);

    // app-specific record
    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->prio)
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    if (paused) {
        //BSP_ledOn();
    }
    else {
        //BSP_ledOff();
    }

    QS_BEGIN_ID(PAUSED_STAT, 0U) // app-specific record
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // exercise the FPU with some floating point computations
    // NOTE: this code can be only called from a task that created with
    // the option OS_TASK_OPT_SAVE_FP.
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    return (rnd >> 8);
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void BSP_terminate(int16_t result) {
    Q_UNUSED_PAR(result);
    QF_stop();
}

//============================================================================
// QF callbacks...
void QF_onStartup(void) {
    k_timer_start(&zephyr_tick_timer, K_MSEC(1), K_MSEC(1));
    Q_PRINTK("QF_onStartup\n");
}
//............................................................................
void QF_onCleanup(void) {
    Q_PRINTK("QF_onCleanup\n");
}

// QS callbacks ==============================================================
#ifdef Q_SPY

#include <zephyr/drivers/uart.h>

// select the Zephyr shell UART
// NOTE: you can change this to other UART peripheral if desired
static struct device const *uart_dev =
     DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

//............................................................................
static void uart_cb(const struct device *dev, void *user_data) {
    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (uart_irq_rx_ready(uart_dev)) {
        uint8_t buf[32];
        int n = uart_fifo_read(uart_dev, buf, sizeof(buf));
        for (uint8_t const *p = buf; n > 0; --n, ++p) {
            QS_RX_PUT(*p);
        }
    }
}
//............................................................................
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    Q_REQUIRE(uart_dev != (struct device *)0);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf  (qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[128];  // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // configure interrupt and callback to receive data
    uart_irq_callback_user_data_set(uart_dev, &uart_cb, (void *)0);
    uart_irq_rx_enable(uart_dev);

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
QSTimeCtr QS_onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    return k_cycle_get_32();
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    uint16_t len = 0xFFFFU; // to get as many bytes as available
    uint8_t const *buf;
    while ((buf = QS_getBlock(&len)) != (uint8_t*)0) {
        for (; len != 0U; --len, ++buf) {
            uart_poll_out(uart_dev, *buf);
        }
        len = 0xFFFFU; // to get as many bytes as available
    }
}
//............................................................................
void QS_onReset(void) {
    sys_reboot(SYS_REBOOT_COLD);
}
//............................................................................
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}
//............................................................................
void QF_onIdle(void) {
    QS_rxParse();   // parse any QS-RX bytes

    uint16_t len = 0xFFFFU; // big number to get all available bytes

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    uint8_t const *buf = QS_getBlock(&len);
    QF_CRIT_EXIT();

    // transmit the bytes via the UART...
    for (; len != 0U; --len, ++buf) {
        uart_poll_out(uart_dev, *buf);
    }
}

#endif // Q_SPY

