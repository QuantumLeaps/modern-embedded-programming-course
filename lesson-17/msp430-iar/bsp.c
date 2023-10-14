#include <stdint.h>      /* standard integer types */
#include <msp430x20x3.h> /* MSP430 variant used */
#include "bsp.h"         /* Board Support Package */

__interrupt void Timer0_Handler(void); /* prototype */

void LED_toggle(void) {
    P1OUT ^= LED1;  /* toggle LED1 */
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_Handler(void) {
    LED_toggle();  /* toggle LED1 */
}

void Timer0_Function(void) {
    LED_toggle();  /* toggle LED1 */
}
