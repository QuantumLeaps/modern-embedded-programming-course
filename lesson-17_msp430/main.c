#include <stdint.h>      /* standard integer types */
#include <msp430x20x3.h> /* MSP430 variant used */
#include "bsp.h"         /* Board Support Package */

int main() {
    WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */
    P1DIR |= (LED1 | LED2);   /* set LED1 and LED2 pins to output  */
    P1OUT &= ~(LED1 | LED2);  /* turn LED1 and LED2 off */

    /* configure Timer A */
    TACTL  = (ID_3 | TASSEL_2 | MC_1);  /* SMCLK, /8 divider, upmode, start */
    TACCR0 = SMCLK_HZ/8U/2U - 1U;
    CCTL0  = CCIE;  /* CCR0 interrupt enabled */

    Timer0_Function();

    __enable_interrupt(); /* enable interrupts to the CPU */
    while (1) {
        P1OUT |= LED2;    /* turn LED2 on */
        P1OUT &= ~LED2;   /* turn LED2 off */
    }
    //return 0;
}

