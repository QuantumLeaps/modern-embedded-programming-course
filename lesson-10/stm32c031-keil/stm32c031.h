//=================================================================
// This header file for STM32C031 MCU contains only the facilities
// used in the projects for the "Modern Embedded Programming" video
// course. This file is NOT intended to be complete.
//==================================================================

#ifndef STM32C031_H__
#define STM32C031_H__

#define RCC_IOPENR_R    (*((unsigned int *)0x40021034U))

#define GPIOA_MODER_R   (*((unsigned int *)0x50000000U))
#define GPIOA_BSRR_R    (*((unsigned int *)0x50000018U))
#define GPIOA_OTYPER_R  (*((unsigned int *)0x50000004U))
#define GPIOA_OSPEEDR_R (*((unsigned int *)0x50000008U))
#define GPIOA_PUPDR_R   (*((unsigned int *)0x5000000CU))

#endif // STM32C031_H__
