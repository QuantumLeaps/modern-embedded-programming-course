/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include "FreeRTOS.h"   /* FreeRTOS API */
#include "task.h"       /* FreeRTOS task API */
#include "semphr.h"     /* FreeRTOS semaphore API */
#include "bsp.h"        /* Board Support Package */

#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

/* on-board switch */
#define BTN_SW1   (1U << 4)

SemaphoreHandle_t SW1_sema;

/* Function Prototype ======================================================*/
void vApplicationTickHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

/* Hooks ===================================================================*/
/* Application hooks used in this project ==================================*/
/* NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook*/
void vApplicationTickHook(void) {
}

void GPIOPortF_IRQHandler(void) {
   if ((GPIOF_AHB->RIS & BTN_SW1) != 0U) { /* interrupt caused by SW1? */
       BaseType_t xHigherPriorityTaskWoken = pdFALSE;
       xSemaphoreGiveFromISR(SW1_sema, &xHigherPriorityTaskWoken);
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    GPIOF_AHB->ICR = 0xFFU; /* clear interrupt sources */
}

void BSP_init(void) {
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run Mode for GPIOF */

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
    NVIC_SetPriority(SysTick_IRQn,
        (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8-__NVIC_PRIO_BITS)));
    NVIC_SetPriority(GPIOF_IRQn,
        (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8-__NVIC_PRIO_BITS)) + 1);

    /* enable IRQs in NVIC... */
    NVIC_EnableIRQ(GPIOF_IRQn);
}
/*..........................................................................*/
void vApplicationIdleHook(void) {
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
/*..........................................................................*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* ERROR!!! */
}
/*..........................................................................*/
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
 * provide an implementation of vApplicationGetIdleTaskMemory() to provide
 * the memory that is used by the Idle task.
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside
     * this function then they must be declared static - otherwise they will
     * be allocated on the stack and so not exists after this function exits.
     */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the
     * Idle task's state will be stored.
     */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = &uxIdleTaskStack[0];

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes.
     */
    *pulIdleTaskStackSize = sizeof(uxIdleTaskStack) / sizeof(uxIdleTaskStack[0]);
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
