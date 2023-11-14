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

    GPIOD_AHB->DATA_Bits[PD0_PIN] = PD0_PIN;

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

            GPIOD_AHB->DATA_Bits[PD1_PIN] = PD1_PIN;
            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW1");
                QS_U8(1U, 1U);
            QS_END()
            GPIOD_AHB->DATA_Bits[PD1_PIN] = 0U;
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const buttonReleasedEvt
                              = QEVT_INITIALIZER(BUTTON_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &buttonReleasedEvt, 0U);
            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW1");
                QS_U8(1U, 0U);
            QS_END()
        }
    }
    if ((tmp & BTN_SW2) != 0U) {  /* debounced SW2 state changed? */
        if ((buttons.depressed & BTN_SW2) != 0U) { /* is SW2 depressed? */
            /* post the "button-pressed" event from ISR */
            static QEvt const button2PressedEvt
                              = QEVT_INITIALIZER(BUTTON2_PRESSED_SIG);
            QACTIVE_POST(AO_TimeBomb, &button2PressedEvt, 0U);
            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW2");
                QS_U8(1U, 1U);
            QS_END()
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const button2ReleasedEvt
                              = QEVT_INITIALIZER(BUTTON2_RELEASED_SIG);
            QACTIVE_POST(AO_TimeBomb, &button2ReleasedEvt, 0U);
            QS_BEGIN_ID(QS_USER, 0)
                QS_STR("SW2");
                QS_U8(1U, 0U);
            QS_END()
        }
    }
    GPIOD_AHB->DATA_Bits[PD0_PIN] = 0U;
}
/*..........................................................................*/
void QV_onIdle(void) {
#ifdef Q_SPY
    QF_INT_ENABLE();
    QS_rxParse();  /* parse all the received bytes */

    if ((UART0->FR & UART_FR_TXFE) != 0U) {  /* TX done? */
        uint16_t fifo = UART_TXFIFO_DEPTH;   /* max bytes we can accept */
        uint8_t const *block;

        QF_INT_DISABLE();
        block = QS_getBlock(&fifo);  /* try to get next block to transmit */
        QF_INT_ENABLE();

        while (fifo-- != 0) {        /* any bytes in the block? */
            UART0->DR = *block++;    /* put into the FIFO */
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
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* enable clock for to the peripherals used by this application... */
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->RCGCGPIO  |= (1U << 3); /* enable Run mode for GPIOD */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 3); /* enable AHB for GPIOD */
    __ISB();
    __DSB();

    /* configure LEDs (digital output) */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DATA_Bits[LED_RED | LED_BLUE | LED_GREEN] = 0U;

    /* configure Test pins (digital output) */
    GPIOD_AHB->DIR |= (PD0_PIN | PD1_PIN);
    GPIOD_AHB->DEN |= (PD0_PIN | PD1_PIN);

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

    QS_INIT((void *)0); /* initialize the QS software tracing */

    /* setup the QS filters... */
    //QS_GLB_FILTER(QS_UA_RECORDS); /* all User records */
    //QS_GLB_FILTER(QS_SM_RECORDS); /* state machine records */
    QS_GLB_FILTER(QS_ALL_RECORDS); /* all QS records */
    QS_GLB_FILTER(-QS_QF_TICK); /* disable */

    /* prodice the QS dictionaries... */
    QS_OBJ_DICTIONARY(AO_TimeBomb);

    QS_SIG_DICTIONARY(BUTTON_PRESSED_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON_RELEASED_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON2_PRESSED_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON2_RELEASED_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
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
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("red");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("red");
        QS_U8(1U, 0U);
    QS_END()
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("blue");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("blue");
        QS_U8(1U, 0U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("green");
        QS_U8(1U, 1U);
    QS_END()
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
    QS_BEGIN_ID(QS_USER, 0)
        QS_STR("green");
        QS_U8(1U, 0U);
    QS_END()
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY

/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[1000]; /* buffer for QS transmit channel */
    static uint8_t qsRxBuf[100];  /* buffer for QS receive channel */

    QS_initBuf  (qsTxBuf, sizeof(qsTxBuf));
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    /* enable clock for UART0 and GPIOA (used by UART0 pins) */
    SYSCTL->RCGCUART   |= (1U << 0); /* enable Run mode for UART0 */
    SYSCTL->RCGCGPIO   |= (1U << 0); /* enable Run mode for GPIOA */

    /* configure UART0 pins for UART operation */
    uint32_t tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->AMSEL &= ~tmp;  /* disable analog function on the pins */
    GPIOA->AFSEL |= tmp;   /* enable ALT function on the pins */
    GPIOA->DEN   |= tmp;   /* enable digital I/O on the pins */
    GPIOA->PCTL  &= ~0x00U;
    GPIOA->PCTL  |= 0x11U;

    /* configure the UART for the desired baud rate, 8-N-1 operation */
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD  = tmp / 64U;
    UART0->FBRD  = tmp % 64U;
    UART0->LCRH  = (0x3U << 5); /* configure 8-N-1 operation */
    UART0->LCRH |= (0x1U << 4); /* enable FIFOs */
    UART0->CTL   = (1U << 0)    /* UART enable */
                    | (1U << 8)  /* UART TX enable */
                    | (1U << 9); /* UART RX enable */

    /* configure UART interrupts (for the RX channel) */
    UART0->IM   |= (1U << 4) | (1U << 6); /* enable RX and RX-TO interrupt */
    UART0->IFLS |= (0x2U << 2);    /* interrupt on RX FIFO half-full */
    /* NOTE: do not enable the UART0 interrupt yet. Wait till QF_onStartup() */

    NVIC_SetPriority(UART0_IRQn, 0U); /* kernel unaware interrupt */
    NVIC_EnableIRQ(UART0_IRQn);  /* UART0 interrupt used for QS-RX */

    /* configure TIMER5 to produce QS time stamp */
    SYSCTL->RCGCTIMER |= (1U << 5);  /* enable run mode for Timer5 */
    TIMER5->CTL  = 0U;               /* disable Timer1 output */
    TIMER5->CFG  = 0x0U;             /* 32-bit configuration */
    TIMER5->TAMR = (1U << 4) | 0x02; /* up-counting periodic mode */
    TIMER5->TAILR= 0xFFFFFFFFU;      /* timer interval */
    TIMER5->ICR  = 0x1U;             /* TimerA timeout flag bit clears*/
    TIMER5->CTL |= (1U << 0);        /* enable TimerA module */

    return 1U; /* return success */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) {  /* NOTE: invoked with interrupts DISABLED */
    return TIMER5->TAV;
}
/*..........................................................................*/
void QS_onFlush(void) {
    while (true) {
        /* try to get next byte to transmit */
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) { /* NOT end-of-data */
            /* busy-wait as long as TX FIFO has data to transmit */
            while ((UART0->FR & UART_FR_TXFE) == 0) {
            }
            /* place the byte in the UART DR register */
            UART0->DR = b;
        }
        else {
            break; /* break out of the loop */
        }
    }
}

/*..........................................................................*/
/*
* ISR for receiving bytes from the QSPY Back-End
* NOTE: This ISR is "QF-unaware" meaning that it does not interact with
* the QF/QK and is not disabled. Such ISRs don't need to call QK_ISR_ENTRY/
* QK_ISR_EXIT and they cannot post or publish events.
*/
void UART0_IRQHandler(void) {
    uint32_t status = UART0->RIS; /* get the raw interrupt status */
    UART0->ICR = status;          /* clear the asserted interrupts */

    while ((UART0->FR & UART_FR_RXFE) == 0) { /* while RX FIFO NOT empty */
        uint32_t b = UART0->DR;
        QS_RX_PUT(b);
    }
    QV_ARM_ERRATUM_838869();
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

#endif /* Q_SPY */
