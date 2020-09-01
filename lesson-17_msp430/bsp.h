#ifndef __BSP_H__
#define __BSP_H__

/* Board Support Package for the MSP-EXP430G2 board */

/* Main Clock and Subsystem Clock setting [Hz] */
#define MCLK_HZ    1000000U
#define SMCLK_HZ   1000000U

/* on-board LEDs */
#define LED1   (1U << 0)
#define LED2   (1U << 6)

void LED_toggle(void);
void Timer0_Function(void);


#endif // __BSP_H__