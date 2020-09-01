#include <stdint.h>
#include "bsp.h"

#if 0
/* background code: sequential with blocking version */
int main() {
    BSP_init();
    while (1) {
        BSP_ledGreenOn();
        BSP_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
        BSP_delay(BSP_TICKS_PER_SEC * 3U / 4U);
    }

    //return 0;
}
#endif

/* background code: non-blocking version */
int main() {
    BSP_init();
    while (1) {
        /* Blinky polling state machine */
        static enum {
            INITIAL,
            OFF_STATE,
            ON_STATE
        } state = INITIAL;
        static uint32_t start;
        switch (state) {
            case INITIAL:
                start = BSP_tickCtr();
                state = OFF_STATE; /* initial transition */
                break;
            case OFF_STATE:
                if ((BSP_tickCtr() - start) > BSP_TICKS_PER_SEC * 3U / 4U) {
                    BSP_ledGreenOn();
                    start = BSP_tickCtr();
                    state = ON_STATE; /* state transition */
                }
                break;
            case ON_STATE:
                if ((BSP_tickCtr() - start) > BSP_TICKS_PER_SEC / 4U) {
                    BSP_ledGreenOff();
                    start = BSP_tickCtr();
                    state = OFF_STATE; /* state transition */
                }
                break;
            default:
                //error();
                break;
        }
    }
    //return 0;
}
