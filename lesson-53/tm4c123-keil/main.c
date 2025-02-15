#include <stdint.h> // C99 standard integers
#include "sched.h"
#include "bsp.h"

int main() {
    BSP_init();

    sched_start(TASK_AIRBAG_PRIO, &BSP_deployAirbag);
    sched_start(TASK_ABS_PRIO,    &BSP_engageABS);

    sched_run(); // does NOT return
}
