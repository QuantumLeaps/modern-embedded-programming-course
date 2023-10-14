#ifndef __BSP_H__
#define __BSP_H__

/* Board Support Package for the EK-TM4C123GXL board */

/* system clock setting [Hz] */
#define SYS_CLOCK_HZ 16000000U

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

#endif // __BSP_H__