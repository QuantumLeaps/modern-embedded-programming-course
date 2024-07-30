/* Board Support Package (BSP) for the NUCLEO-C031C6 board */
#include "qpc.h"   // for the QXK kernel (part of QP/C)

#include "bsp.h"
#include "stm32c0xx.h"  /* CMSIS-compliant header file for the MCU used */
/* add other drivers if necessary... */

// LED marked "LD4" (PA.5) on the NUCLEO-C031C6 board
#define LD4_PIN  5U

// external LED to be inserted between GND (short leg) and
// D12 (longer leg) on the CN9 connector
#define LD5_PIN  6U

// Button B1 (PC.13) on the NUCLEO-C031C6 board
#define B1_PIN   13U

void SysTick_Handler(void) {
    QXK_ISR_ENTRY();  /* inform QXK about entering an ISR */

    QF_TICK_X(0U, (void *)0); /* process time events for rate 0 */

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

void BSP_init(void) {
    // enable GPIOA clock port for the LEDs
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

    // configure Button B1 (PC.13) pins as input, no pull-up, pull-down
    GPIOC->MODER   &= ~(3U << 2*B1_PIN);
    GPIOC->OSPEEDR &= ~(3U << 2*B1_PIN);
    GPIOC->OSPEEDR |=  (1U << 2*B1_PIN);
    GPIOC->PUPDR   &= ~(3U << 2*B1_PIN);
}

void BSP_ledRedOn(void) {
    /* no red LED on board */
}

void BSP_ledRedOff(void) {
    /* no red LED on board */
}

void BSP_ledBlueOn(void) {
    /* no blue LED on board */
}

void BSP_ledBlueOff(void) {
    /* no blue LED on board */
}

void BSP_ledGreenOn(void) {
    GPIOA->BSRR = (1U << LD4_PIN);  /* turn LED on  */
}

void BSP_ledGreenOff(void) {
    GPIOA->BSRR = (1U << (LD4_PIN + 16U));  /* turn LED off */
}

/* callbacks ---------------------------------------------------------------*/
void QF_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set the SysTick interrupt priority (highest) */
    NVIC_SetPriority(SysTick_IRQn, 0U);
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QXK_onIdle(void) {
#ifdef NDBEBUG
    __WFI(); /* stop the CPU and Wait for Interrupt */
#endif
}

//............................................................................
void Q_onAssert(char const *module, int id) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)id;     /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}

void assert_failed(char const *module, int id) {
    Q_onAssert(module, id);
}
