/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include "qpc.h"
#include "bsp.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

/* on-board switch */
#define BTN_SW1   (1U << 4)

//SEMA
//static QXSemaphore Morse_sema;

//MUTEX
static QXMutex Morse_mutex;

void SysTick_Handler(void) {
    QXK_ISR_ENTRY();  /* inform QXK about entering an ISR */

    QF_TICK_X(0U, (void *)0); /* process time events for rate 0 */

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

void GPIOPortF_IRQHandler(void) {
    QXK_ISR_ENTRY();  /* inform QXK about entering an ISR */
    if ((GPIOF_AHB->RIS & BTN_SW1) != 0U) { /* interrupt caused by SW1? */
        QXSemaphore_signal(&SW1_sema);
    }
    GPIOF_AHB->ICR = 0xFFU; /* clear interrupt sources */
    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

void BSP_init(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* configure switch SW1 */
    GPIOF_AHB->DIR &= ~BTN_SW1; /* input */
    GPIOF_AHB->DEN |= BTN_SW1; /* digital enable */
    GPIOF_AHB->PUR |= BTN_SW1; /* pull-up resistor enable */

    /* GPIO interrupt setup for SW1 */
    GPIOF_AHB->IS  &= ~BTN_SW1; /* edge detect for SW1 */
    GPIOF_AHB->IBE &= ~BTN_SW1; /* only one edge generate the interrupt */
    GPIOF_AHB->IEV &= ~BTN_SW1; /* a falling edge triggers the interrupt */
    GPIOF_AHB->IM  |= BTN_SW1;  /* enable GPIOF interrupt for SW1 */

    //SEMA
    ///* initialize the Morse_sema semaphore as binary, resource semaphore */
    //QXSemaphore_init(&Morse_sema, /* pointer to semaphore to initialize */
    //                 1U,  /* initial semaphore count (resource semaphore) */
    //                 1U); /* maximum semaphore count (binary semaphore) */

    //MUTEX
    QXMutex_init(&Morse_mutex, 6U); /* priority ceiling 6 */
}

void BSP_ledRedOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}

void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

void BSP_ledBlueToggle(void) {
    //CRIT
    QF_CRIT_STAT_TYPE istat;

    QF_CRIT_ENTRY(istat);
    GPIOF_AHB->DATA ^= LED_BLUE;
    QF_CRIT_EXIT(istat);
}

void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

void BSP_ledGreenToggle(void) {
    //CRIT
    QF_CRIT_STAT_TYPE istat;

    QF_CRIT_ENTRY(istat);
    GPIOF_AHB->DATA ^= LED_GREEN;
    QF_CRIT_EXIT(istat);
}

void BSP_sendMorseCode(uint32_t bitmask) {
    uint32_t volatile delay_ctr;
    enum { DOT_DELAY = 150 };

    //LOCK
    //QSchedStatus sstat;

    //SEMA
    //QXSemaphore_wait(&Morse_sema,  /* pointer to semaphore to wait on */
    //                 QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

    //LOCK
    //sstat = QXK_schedLock(5U); /* priority ceiling 5 */

    //MUTEX
    QXMutex_lock(&Morse_mutex,
                 QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

    for (; bitmask != 0U; bitmask <<= 1) {
        if ((bitmask & (1U << 31)) != 0U) {
            BSP_ledGreenOn();
        }
        else {
            BSP_ledGreenOff();
        }
        for (delay_ctr = DOT_DELAY;
             delay_ctr != 0U; --delay_ctr) {
        }
    }
    BSP_ledGreenOff();
    for (delay_ctr = 7*DOT_DELAY;
         delay_ctr != 0U; --delay_ctr) {
    }

    //SEMA
    //QXSemaphore_signal(&Morse_sema);  /* pointer to semaphore to signal */

    //LOCK
    //QXK_schedUnlock(sstat);

    //MUTEX
    QXMutex_unlock(&Morse_mutex);
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

void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}
