#include <stdint.h>
#include <arm_acle.h>
#include "sched.h"
#include "bsp.h"

#define MAX_TASK 32
static task_handler task_registry[MAX_TASK + 1];
static uint32_t volatile ready_set;

void sched_start(uint8_t prio, task_handler task) {
    task_registry[prio] = task;
}

void sched_post(uint8_t prio) {
    BSP_intDisable();
    ready_set |= (1U << (prio - 1));
    BSP_intEnable();
}

void sched_run(void) {
    while(1) { // "superloop"
        BSP_intDisable();
        if (ready_set == 0) {
            BSP_goToSleep(); // enter with interrupts DISABLED
            // BSP_goToSleep() exits with interrupts ENABLED
        }
        else {
            unsigned p = 32U - __clz(ready_set); // task priority
            if (p == 0U) {
                assert_failed(__FILE__, __LINE__);
            }
            task_handler task = task_registry[p];
            ready_set &= ~(1U << (p - 1U));
            BSP_intEnable();

            (*task)(); // Execute the selected task
        }
    }
}