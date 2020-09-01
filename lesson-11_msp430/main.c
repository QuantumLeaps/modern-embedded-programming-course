#include <stdint.h>
#include <msp430x20x3.h>  /* MSP430 variant used */

#define LED1   (1U << 0)
#define LED2   (1U << 6)

uint8_t  u8a, u8b;
uint16_t u16c, u16d;
uint32_t u32e, u32f;

int8_t  s8;
int16_t s16;
int32_t s32;

int main() {
    WDTCTL = WDTPW + WDTHOLD; /* stop watchdog timer */

    u16c = 40000U;
    u16d = 30000U;
    u32e = (uint32_t)u16c + u16d;

    u16c = 100U;
    s32  = 10 - (int16_t)u16c;

    P1DIR |= LED1;     /* set LED1 pin to output  */

    while (1) {
        unsigned volatile i;  /* volatile to prevent optimization */

        P1OUT |= LED1;         /* turn LED1 on */
        for (i = 20000U; i != 0U; --i) { /* wait a bit */
        }

        P1OUT &= ~LED1;        /* turn LED1 off */
        for (i = 20000U; i != 0U; --i) { /* wait a bit */
        }
    }
}
