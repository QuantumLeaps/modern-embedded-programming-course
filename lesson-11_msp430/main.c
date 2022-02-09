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

    u8a  = sizeof(u8a);
    u16c = sizeof(uint16_t);
    u32e = sizeof(uint32_t);

    u8a  = 0xa1U;
    u16c = 0xc1c2U;
    u32e = 0xe1e2e3e4U;

    u8b  = u8a;
    u16d = u16c;
    u32f = u32e;

    u16c = 40000U;
    u16d = 30000U;
    //u32e = u16c + u16d; // NOT portable!
    u32e = (uint32_t)u16c + u16d;

    u16c = 0x7FFFU + 20U;
    //s32  = 10 - u16c;  // NOT portable!
    //s32  = 10 - (int16_t)u16c; // INCORRECT: insufficient dynamic range
    s32  = 10 - (int32_t)u16c; // CORRECT: cast to the lvalue type

    //if (u32e > -1) {  // ALWAYS false!
    if ((int32_t)u32e > -1) {
        u8a = 1U;
    }
    else {
        u8a = 0U;
    }

    u8a = 0xffU;
    //if (~u8a == 0x00U) { // ALWAYS false!
    if ((uint8_t)(~u8a) == 0x00U) {
        u8b = 1U;
    }

    /* blinking the LED */
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
