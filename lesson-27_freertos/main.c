#include "FreeRTOS.h"   /* FreeRTOS API */
#include "task.h"       /* FreeRTOS task API */
#include "semphr.h"     /* FreeRTOS semaphore API */
#include "bsp.h"        /* Board Support Package */

StaticSemaphore_t SW1_sema_sto;

static StackType_t stack_blinky1[configMINIMAL_STACK_SIZE];
static StaticTask_t Blinky1_tcb;
void main_blinky1(void *pvParameters) {
    while (1) {
        uint32_t volatile i;
        for (i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
        }
        vTaskDelay(1U); /* block for 1 tick */
    }
}

StackType_t stack_blinky2[configMINIMAL_STACK_SIZE];
static StaticTask_t Blinky2_tcb;
void main_blinky2(void *pvParameters) {
    while (1) {
        xSemaphoreTake(SW1_sema, portMAX_DELAY);

        for (uint32_t volatile i = 3*1500U; i != 0U; --i) {
            BSP_ledBlueOn();
            BSP_ledBlueOff();
        }
    }
}

StackType_t stack_blinky3[configMINIMAL_STACK_SIZE];
static StaticTask_t Blinky3_tcb;
void main_blinky3(void *pvParameters) {
    while (1) {
        BSP_ledRedOn();
        vTaskDelay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        vTaskDelay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

int main() {
    BSP_init(); /* initialize the Board Support Package */

    /* instantiate and start all FreeRTOS tasks and queues... */
    TaskHandle_t th;


    /* initialize the SW1_sema semaphore as binary, signaling semaphore */
    SW1_sema = xSemaphoreCreateBinaryStatic(&SW1_sema_sto);

    /* initialize and start blinky1 thread */
    th = xTaskCreateStatic(&main_blinky1,
        "Blinky1",
        sizeof(stack_blinky1)/sizeof(stack_blinky1[0]),
        &Blinky1_tcb,                 /* task param */
        5U + tskIDLE_PRIORITY,        /* task priority */
        stack_blinky1,                /* task stack */
        &Blinky1_tcb);                /* task control block */

    /* initialize and start blinky2 thread */
    th = xTaskCreateStatic(&main_blinky2,
        "Blinky2",
        sizeof(stack_blinky2)/sizeof(stack_blinky2[0]),
        &Blinky2_tcb,                 /* task param */
        2U + tskIDLE_PRIORITY,        /* task priority */
        stack_blinky2,                /* task stack */
        &Blinky2_tcb);                /* task control block */

    /* initialize and start blinky3 thread */
    //th = xTaskCreateStatic(&main_blinky3,
    //    "Blinky3",
    //    sizeof(stack_blinky3)/sizeof(stack_blinky3[0]),
    //    &Blinky3_tcb,                 /* task param */
    //    1U + tskIDLE_PRIORITY,        /* task priority */
    //    stack_blinky3,                /* task stack */
    //    &Blinky3_tcb);                /* task control block */

    BSP_onStartup();

    vTaskStartScheduler(); /* start the FreeRTOS scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}
