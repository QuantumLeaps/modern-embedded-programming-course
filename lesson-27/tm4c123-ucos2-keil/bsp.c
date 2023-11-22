/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include "ucos_ii.h"    /* uC-OS2 API */
#include <stdint.h>     /* C99 standard integers */
#include "bsp.h"        /* Board Support Package */

#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

/* on-board switch */
#define BTN_SW1   (1U << 4)

OS_EVENT *SW1_sema;

/* ISRs =================================================================*/
void GPIOPortF_IRQHandler(void) {
#if OS_CRITICAL_METHOD == 3u  /* Allocate storage for CPU status register */
    OS_CPU_SR cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    OSIntEnter();  /* Tell uC/OS-II that we are starting an ISR */
    OS_EXIT_CRITICAL();

    if ((GPIOF_AHB->RIS & BTN_SW1) != 0U) { /* interrupt caused by SW1? */
        OSSemPost(SW1_sema);
    }
    GPIOF_AHB->ICR = 0xFFU; /* clear interrupt sources */

    OSIntExit();   /* Tell uC/OS-II that we are leaving the ISR */
}

void BSP_init(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run Mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */

    /* make sure the Run Mode and AHB-enable take effects
    * before accessing the peripherals
    */
    __ISB(); /* Instruction Synchronization Barrier */
    __DSB(); /* Data Memory Barrier */

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

void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

/* callbacks ---------------------------------------------------------------*/
void BSP_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set the interrupt priorities of "kernel aware" interrupts */
    NVIC_SetPriority(SysTick_IRQn, CPU_CFG_KA_IPL_BOUNDARY + 1U);
    NVIC_SetPriority(GPIOF_IRQn,   CPU_CFG_KA_IPL_BOUNDARY);

    /* enable IRQs in NVIC... */
    NVIC_EnableIRQ(GPIOF_IRQn);
}

/* Hooks ===================================================================*/
void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }
void App_TimeTickHook   (void)         {}

/*..........................................................................*/
void App_TaskIdleHook(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
    __WFI(); /* Wait-For-Interrupt */
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
#endif
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}
//............................................................................
_Noreturn void assert_failed(char const * const module, int const id);
_Noreturn void assert_failed(char const * const module, int const id) {
    (void)module; // unused parameter
    (void)id;     // unused parameter
#ifndef NDEBUG
    // light up all LEDs
    GPIOF_AHB->DATA_Bits[LED_GREEN | LED_RED | LED_BLUE] = 0xFFU;
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif
    NVIC_SystemReset();
}
