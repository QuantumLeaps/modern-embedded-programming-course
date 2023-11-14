#include "bsp.h"

int main(void) {

    BSP_init();

    BSP_led5On();

    while (1) { // endless loop
        // empty loop because all work
        // is done in SysTick_Handler() in bsp.c
    }
    //return 0; // unreachable code
}
