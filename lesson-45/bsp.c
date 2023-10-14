/*****************************************************************************
* BSP for EK-TM4C123GXL with QP/C framework
*****************************************************************************/
#include "qpc.h"  /* QP/C API */
#include "bsp.h"

#include "TM4C123GH6PM.h"        /* the device specific header (TI) */
#include "rom.h"                 /* the built-in ROM functions (TI) */
#include "sysctl.h"              /* system control driver (TI) */
#include "gpio.h"                /* GPIO driver (TI) */
/* add other drivers if necessary... */

#include "my_printf.h" /* software tracing with printf() */

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
            static QEvt const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            QACTIVE_POST(AO_TimeBomb, &buttonPressedEvt, 0U);
            MY_PRINTF("BTN %s %d\n", "SW1", 1);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
            QACTIVE_POST(AO_TimeBomb, &buttonReleasedEvt, 0U);
            MY_PRINTF("BTN %s %d\n", "SW1", 0);
        }
    }
    if ((tmp & BTN_SW2) != 0U) {  /* debounced SW2 state changed? */
        if ((buttons.depressed & BTN_SW2) != 0U) { /* is SW2 depressed? */
            /* post the "button-pressed" event from ISR */
            static QEvt const button2PressedEvt = {BUTTON2_PRESSED_SIG};
            QACTIVE_POST(AO_TimeBomb, &button2PressedEvt, 0U);
            MY_PRINTF("BTN %s %d\n", "SW2", 1);
        }
        else { /* the button is released */
            /* post the "button-released" event from ISR */
            static QEvt const button2ReleasedEvt = {BUTTON2_RELEASED_SIG};
            QACTIVE_POST(AO_TimeBomb, &button2ReleasedEvt, 0U);
            MY_PRINTF("BTN %s %d\n", "SW2", 0);
        }
    }
    GPIOD_AHB->DATA_Bits[PD0_PIN] = 0U;
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

    MY_PRINTF_INIT();
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
    MY_PRINTF("LED-%s is %d\n", "red", 1); /* -> "LED-red is 1\n" */
}

/*..........................................................................*/
void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
    MY_PRINTF("LED-%s is %d\n", "red", 0);
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
    MY_PRINTF("LED-%s is %d\n", "blue", 1);
}

/*..........................................................................*/
void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
    MY_PRINTF("LED-%s is %d\n", "blue", 0);
}

/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
    MY_PRINTF("LED-%s is %d\n", "green", 1);
}

/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
    MY_PRINTF("LED-%s is %d\n", "green", 0);
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
/* support for MY_PRINTF() ====================================================*/
#ifdef SPY

#define UART_BAUD_RATE      115200U
#define UART_FR_TXFE        (1U << 7)
#define UART_FR_RXFE        (1U << 4)
#define UART_BUSY           (1U << 3)
#define UART_TXFF           (1U << 5)
#define UART_TXFIFO_DEPTH   16U


int fputc(int c, FILE *stream) {
    (void)stream; /* unused parameter */

    GPIOD_AHB->DATA_Bits[PD1_PIN] = PD1_PIN;

    /* busy-wait as long as UART busy */
    while ((UART0->FR & UART_BUSY) != 0) {
    }
    UART0->DR = c; /* write the byte into Data Register */

    GPIOD_AHB->DATA_Bits[PD1_PIN] = 0;

    return c;
}

void printf_init() {
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
    UART0->CTL   = (1U << 0)    /* UART enable */
                    | (1U << 8)  /* UART TX enable */
                    | (1U << 9)  /* UART RX enable */
                    | (1U << 4); /* EOT end of transmission */
}
#endif
