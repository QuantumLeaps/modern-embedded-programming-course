#include <stdint.h>
#include "bsp.h"

enum { SAMPLE_SIG };

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
        static uint8_t prev_button = 1U; /* SW1 is inactive high */
       
        /* buffering of intputs... */
        struct {
            uint16_t sig;
            
            /* event parameters */
            uint32_t now;
            uint8_t button;
        } evt, *e;
        
        evt.sig = SAMPLE_SIG;
        evt.now = BSP_tickCtr();
        evt.button = BSP_SW1();
        
        e = &evt;
        switch (state) {
            case INITIAL:
                start = e->now;
                state = OFF_STATE; /* initial transition */
                break;
            case OFF_STATE:
                if ((e->now - start) > BSP_TICKS_PER_SEC * 3U / 4U) {
                    BSP_ledGreenOn();
                    start = e->now;
                    state = ON_STATE; /* state transition */
                }
                else if ((prev_button != 0U) && (e->button == 0U)) { /* derpressed? */
                    BSP_ledBlueOn();
                    prev_button = e->button;
                }
                else if ((prev_button == 0U) && (e->button != 0U)) { /* released? */
                    BSP_ledBlueOff();
                    prev_button = e->button;
                }
                break;
            case ON_STATE:
                if ((e->now - start) > BSP_TICKS_PER_SEC / 4U) {
                    BSP_ledGreenOff();
                    start = e->now;
                    state = OFF_STATE; /* state transition */
                }
                else if ((prev_button != 0U) && (e->button == 0U)) { /* derpressed? */
                    BSP_ledBlueOn();
                    prev_button = e->button;
                }
                else if ((prev_button == 0U) && (e->button != 0U)) { /* released? */
                    BSP_ledBlueOff();
                    prev_button = e->button;
                }
                break;
            default:
                //error();
                break;
        }
    }
    //return 0;
}
