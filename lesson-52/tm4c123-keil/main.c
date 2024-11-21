#include <stdint.h> // C99 standard integers
#include "bsp.h"

typedef void (*task_handler)(void);

int main() {
    BSP_init();

    while(1) { // "superloop"
        BSP_intDisable();
        if (ready_set == 0) {
            BSP_goToSleep(); // enter with interrupts DISABLED
            // BSP_goToSleep() exits with interrupts ENABLED
        }
        else {
            task_handler task;
            if (ready_set & ISR_1) {
                ready_set &= ~ISR_1;
                task = &BSP_deployAirbag;
            }
            else if (ready_set & ISR_2) {
                ready_set &= ~ISR_2;
                task = &BSP_engageABS;
            }
            else {
                assert_failed(__FILE__, __LINE__);
            }
            BSP_intEnable();

            (*task)(); // Execute the selected task
        }
    }
}
