/*****************************************************************************
* BSP for STM32 NUCLEO-C031C6 with QP/C framework
*****************************************************************************/
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

#include "stm32c0xx.h"  // CMSIS-compliant header file for the MCU used
/* add other drivers if necessary... */

Q_DEFINE_THIS_FILE  // define the name of this file for assertions

// Local-scope defines -----------------------------------------------------
// LED pins available on the board (just one user LED LD4--Green on PA.5)
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

// Button pins available on the board (just one user Button B1 on PC.13)
#define B1_PIN   13U

#ifdef Q_SPY

    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;

    // QSpy source IDs
    static QSpyId const l_SysTick_Handler = { 0U };

#endif

/* Assertion handler  ======================================================*/
Q_NORETURN Q_onAssert(char const * module, int_t id) {
    /* TBD: Perform corrective actions and damage control
    * SPECIFIC to your particular system.
    */
    (void)module;   /* unused parameter */
    (void)id;       /* unused parameter */
#ifndef NDEBUG /* debug build? */
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
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

            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW1");
                QS_U8(1U, 1U);
            QS_END()
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const releaseEvt = QEVT_INITIALIZER(BUTTON_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &releaseEvt, 0U);
            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW1");
                QS_U8(1U, 0U);
            QS_END()
        }
    }

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif
}
/*..........................................................................*/
void QV_onIdle(void) {
#ifdef Q_SPY
    // interrupts still disabled
    QS_rxParse();  // parse all the received bytes
    QF_INT_ENABLE();

    if ((USART2->ISR & (1U << 7U)) != 0U) { // is TXE empty?
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {   // not End-Of-Data?
            USART2->TDR = b; // put into the DR register
        }
    }
#elif defined NDEBUG
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

    // initialize the QS software tracing...
    if (!QS_INIT((void *)0)) {
        Q_ERROR();
    }

    /* setup the QS filters... */
    //QS_GLB_FILTER(QS_UA_RECORDS); /* all User records */
    //QS_GLB_FILTER(QS_SM_RECORDS); /* state machine records */
    QS_GLB_FILTER(QS_ALL_RECORDS); /* all QS records */
    QS_GLB_FILTER(-QS_QF_TICK); /* disable */

    /* prodice the QS dictionaries... */
    QS_OBJ_DICTIONARY(AO_TimeBomb);

    QS_SIG_DICTIONARY(BUTTON_PRESSED_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON_RELEASED_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
}
/*..........................................................................*/
void QF_onStartup(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    * NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    */
    SystemCoreClockUpdate();
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
    GPIOA->BSRR = (1U << LD4_PIN); // turn LD4 on
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("red");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U)); // turn LD4 off
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("red");
        QS_U8(1U, 0U);
    QS_END()
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    //GPIOA->BSRR = (1U << LD4_PIN);  // turn LD4 on
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("blue");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    //GPIOA->BSRR = (1U << (LD4_PIN + 16U));  // turn LD4 off
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("blue");
        QS_U8(1U, 0U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOA->BSRR = (1U << LD5_PIN);  // turn LD5 on
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("green");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOA->BSRR = (1U << (LD5_PIN + 16U));  // turn LD5 off
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("green");
        QS_U8(1U, 0U);
    QS_END()
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY
//............................................................................
static uint16_t const UARTPrescTable[12] = {
    1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};

#define UART_DIV_SAMPLING16(__PCLK__, __BAUD__, __CLOCKPRESCALER__) \
  ((((__PCLK__)/UARTPrescTable[(__CLOCKPRESCALER__)]) \
  + ((__BAUD__)/2U)) / (__BAUD__))

#define UART_PRESCALER_DIV1  0U

// USART2 pins PA.2 and PA.3
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

//............................................................................
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[100];    // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // enable peripheral clock for USART2
    RCC->IOPENR  |= ( 1U <<  0U);  // Enable GPIOA clock for USART pins
    RCC->APBENR1 |= ( 1U << 17U);  // Enable USART#2 clock

    // Configure PA to USART2_RX, PA to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4U*USART2_RX_PIN) | (15U << 4U*USART2_TX_PIN));
    GPIOA->AFR[0] |=  (( 1U << 4U*USART2_RX_PIN) | ( 1U << 4U*USART2_TX_PIN));
    GPIOA->MODER  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->MODER  |=  (( 2U << 2U*USART2_RX_PIN) | ( 2U << 2U*USART2_TX_PIN));

    // baud rate
    USART2->BRR  = UART_DIV_SAMPLING16(
                       SystemCoreClock, 115200U, UART_PRESCALER_DIV1);
    USART2->CR3  = 0x0000U |      // no flow control
                   (1U << 12U);   // disable overrun detection (OVRDIS)
    USART2->CR2  = 0x0000U;       // 1 stop bit
    USART2->CR1  = ((1U <<  2U) | // enable RX
                    (1U <<  3U) | // enable TX
                    (1U <<  5U) | // enable RX interrupt
                    (0U << 12U) | // 8 data bits
                    (0U << 28U) | // 8 data bits
                    (1U <<  0U)); // enable USART

    QS_tickPeriod_ = SystemCoreClock / BSP_TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; // to start the timestamp at zero

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
QSTimeCtr QS_onGetTime(void) { // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & 0x00010000U) == 0U) { // not set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
void QS_onFlush(void) {
    for (;;) {
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        if (b != QS_EOD) {
            while ((USART2->ISR & (1U << 7U)) == 0U) { // while TXE not empty
                QF_INT_ENABLE();
                QF_CRIT_EXIT_NOP();

                QF_INT_DISABLE();
            }
            USART2->TDR = b; // put into the DR register
            QF_INT_ENABLE();
        }
        else {
            QF_INT_ENABLE();
            break;
        }
    }
}
/*..........................................................................*/
/*! callback function to reset the target (to be implemented in the BSP) */
void QS_onReset(void) {
    NVIC_SystemReset();
}
/*..........................................................................*/
/*! callback function to execute a user command (to be implemented in BSP) */
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    QS_BEGIN_ID(QS_USER + 1U, 0U) /* app-specific record */
        QS_U8(2, cmdId);
        QS_U32(8, param1);
        QS_U32(8, param2);
        QS_U32(8, param3);
    QS_END()
}

/*..........................................................................*/
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QV and is not disabled. Such ISRs cannot post or publish events.

void USART2_IRQHandler(void); // prototype
void USART2_IRQHandler(void) { // used in QS-RX (kernel UNAWARE interrutp)
    // is RX register NOT empty?
    if ((USART2->ISR & (1U << 5U)) != 0U) {
        uint32_t b = USART2->RDR;
        QS_RX_PUT(b);
    }

    QV_ARM_ERRATUM_838869();
}

#endif /* Q_SPY */
