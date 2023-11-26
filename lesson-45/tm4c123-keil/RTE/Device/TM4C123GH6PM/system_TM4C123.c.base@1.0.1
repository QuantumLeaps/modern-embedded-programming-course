/**************************************************************************//**
 * @file     system_TM4C.c
 * @brief    CMSIS Device System Source File for
 *           Texas Instruments TIVA TM4C123 Device Series
 * @version  V1.01
 * @date     19. March 2015
 *
 * @note
 *                                                             modified by Keil
 ******************************************************************************/

#include <stdint.h>
#include "TM4C123.h"


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//
// This file can be used by the Keil uVision configuration wizard to set
// the following system clock configuration values.  Or the value of the
// macros can be directly edited below if not using the uVision configuration
// wizard.
//
//--------------------- Clock Configuration ----------------------------------
//
//  <e> Clock Configuration
//          <i> Uncheck this box to skip the clock configuration.
//
// The following controls whether the system clock is configured in the
// SystemInit() function.  If it is defined to be 1 then the system clock
// will be configured according to the macros in the rest of this file.
// If it is defined to be 0, then the system clock configuration is bypassed.
//
#define CLOCK_SETUP 1

//********************************* RCC ***************************************
//
//  <h> Run-Mode Clock Configuration (RCC)

//      <o> SYSDIV: System Clock Divisor <2-16>
//          <i> Specifies the divisor used to generate the system clock from
//          <i> either the PLL output of 200 MHz, or the chosen oscillator.
//
// The following value is the system clock divisor.  This will be applied if
// USESYSDIV (see below) is enabled.  The valid range of dividers is 2-16.
//
#define CFG_RCC_SYSDIV 4

//      <q> USESYSDIV: Enable System Clock Divider
//          <i> Check this box to use the System Clock Divider
//
// The following controls whether the system clock divider is used.  If the
// value is 1, then the system clock divider is used, and the value of the
// system divider is defined by SYSDIV (see above).  If the value is 0, then
// the system clock divider is not used.
//
#define CFG_RCC_USESYSDIV 1

//      <q> USEPWMDIV: Enable PWM Clock Divider
//          <i> Check this box to use the PWM Clock Divider
//
// The following controls whether the PWM clock divider is used.  If the
// value is 1, then the PWM clock divider is used, and the value of the
// PWM divider is defined by PWMDIV (see below).  If the value is 0, then
// the PWM clock divider is not used.
//
#define CFG_RCC_USEPWMDIV 1

//      <o> PWMDIV: PWM Unit Clock Divisor
//              <0=> 0: SysClk / 2
//              <1=> 1: SysClk / 4
//              <2=> 2: SysClk / 8
//              <3=> 3: SysClk / 16
//              <4=> 4: SysClk / 32
//              <5=> 5: SysClk / 64
//              <6=> 6: SysClk / 64
//              <7=> 7: SysClk / 64 (default)
//          <i> Specifies the divisor used to generate the PWM time base,
//          <i> from the System Clock
//
// The following value determines the PWM clock divider.  It is used if
// USEPWMDIV is enabled (see above).  Otherwise the PWM clock is the same as
// the system clock.  The value of the divider is determined by the table
// above.
//
#define CFG_RCC_PWMDIV 7

//      <q> PWRDN: PLL Power Down
//          <i> Check this box to disable the PLL.  You must also choose
//          <i> PLL Bypass.
//
// If the following value is 1, then the PLL is powered down.  Keep this value
// as 1 if you do not need to use the PLL.  In this case, BYPASS (see below)
// must also be set to 1.  If you are using the PLL, then this value must be
// set to 0.
//
#define CFG_RCC_PWRDN 0

//      <q> BYPASS: PLL Bypass
//          <i> Check this box to not use the PLL for the System Clock
//
// Set the following value to 1 to bypass the PLL and not use it for the
// system clock.  You must set this to 1 if PWRDN (above) is set to 1.  Set
// this to 0 if you are using the PLL.
//
#define CFG_RCC_BYPASS 0

//      <o> XTAL: Crystal Value
//              < 0=>  0: 1.0000 MHz  (can not be used with PLL)
//              < 1=>  1: 1.8432 MHz  (can not be used with PLL)
//              < 2=>  2: 2.0000 MHz  (can not be used with PLL)
//              < 3=>  3: 2.4576 MHz  (can not be used with PLL)
//              < 4=>  4: 3.579545 MHz
//              < 5=>  5: 3.6864 MHz
//              < 6=>  6: 4.0000 MHz
//              < 7=>  7: 4.096 MHz
//              < 8=>  8: 4.9152 MHz
//              < 9=>  9: 5.0000 MHz
//              <10=> 10: 5.12 MHz
//              <11=> 11: 6.0000 MHz (default)
//              <12=> 12: 6.144 MHz
//              <13=> 13: 7.3728 MHz
//              <14=> 14: 8.0000 MHz
//              <15=> 15: 8.192 MHz
//              <16=> 16: 10.0 MHz
//              <17=> 17: 12.0 MHz
//              <18=> 18: 12.288 MHz
//              <19=> 19: 13.56 MHz
//              <20=> 20: 14.31818 MHz
//              <21=> 21: 16.0 MHz
//              <22=> 22: 16.384 MHz
//          <i> This is the crystal frequency used for the main oscillator
//
// This value defines the crystal frequency for the main oscillator, according
// to the table in the comments above.  If an external crystal is used, then
// this value must be set to match the value of the crystal.
//
#define CFG_RCC_XTAL 21

//      <o> OSCSRC: Oscillator Source
//              <0=> 0: MOSC Main oscillator
//              <1=> 1: IOSC Internal oscillator (default)
//              <2=> 2: IOSC/4 Internal oscillator / 4 (this is necessary if used as input to PLL)
//              <3=> 3: 30kHz 30-KHz internal oscillator
//          <i> Chooses the oscillator that is used for the system clock,
//          <i> or the PLL input.
//
// The following value chooses the oscillator source according to the table in
// the comments above.
//
#define CFG_RCC_OSCSRC 0

//      <q> IOSCDIS: Internal Oscillator Disable
//          <i> Check this box to turn off the internal oscillator
//
// Set the following value to 1 to turn off the internal oscillator.  This
// value can be set to 1 if you are not using the internal oscillator.
//
#define CFG_RCC_IOSCDIS 1

//      <q> MOSCDIS: Main Oscillator Disable
//          <i> Check this box to turn off the main oscillator
//
// Set the following value to 1 to turn off the main oscillator.  This
// value can be set to 1 if you are not using the main oscillator.
//
#define CFG_RCC_MOSCDIS 0

//  </h>

//********************************* RCC2 **************************************
//
//   <h> Run-Mode Clock Configuration 2 (RCC2)

//      <q> USERCC2: Use RCC2
//          <i> Check this box to override some fields in RCC.  RCC2 provides
//          <i> more bits for the system clock divider, and provides an
//          <i> additional oscillator source.  If you do not need these
//          <i> additional features, then leave this box unchecked.
//
// Set the following value to 1 to use the RCC2 register.  The RCC2 register
// overrides some of the fields in the RCC register if it is used.
//
#define CFG_RCC2_USERCC2 0

//      <o> SYSDIV2: System Clock Divisor <2-64>
//          <i> Specifies the divisor used to generate the system clock from
//          <i> either the PLL output of 200 MHz, or the oscillator.
//
// The following value is the system clock divisor.  This will be applied if
// USESYSDIV in RCC is enabled.  The valid range of dividers is 2-64.
//
#define CFG_RCC_SYSDIV2 4

//      <q> PWRDN2: Power Down PLL
//          <i> Check this box to disable the PLL.  You must also choose
//          <i> PLL Bypass.
//
// If the following value is 1, then the PLL is powered down.  Keep this value
// as 1 if you do not need to use the PLL.  In this case, BYPASS2 (see below)
// must also be set to 1.  If you are using the PLL, then this value must be
// set to 0.
//
#define CFG_RCC_PWRDN2 0

//      <q> BYPASS2: Bypass PLL
//          <i> Check this box to not use the PLL for the System Clock
//
// Set the following value to 1 to bypass the PLL and not use it for the
// system clock.  You must set this to 1 if PWRDN2 (above) is set to 1.  Set
// this to 0 if you are using the PLL.
//
#define CFG_RCC_BYPASS2 0

//      <o> OSCSRC2: Oscillator Source
//              <0=> 0: MOSC Main oscillator
//              <1=> 1: IOSC Internal oscillator (default)
//              <2=> 2: IOSC/4 Internal oscillator / 4 (this is necessary if used as input to PLL)
//              <3=> 3: 30kHz 30-kHz internal oscillator
//              <7=> 7: 32kHz 32.768-kHz external oscillator
//          <i> The oscillator that is used for the system clock, or the PLL input.
//
// The following value chooses the oscillator source according to the table in
// the comments above.
//
#define CFG_RCC_OSCSRC2 0

//  </h>
//
//  </e>

//-------- <<< end of configuration section >>> ------------------------------

//
// The following macros are used to program the RCC and RCC2 registers in
// the SystemInit() function.  Edit the macros above to change these values.
//
#define RCC_Val                                                               \
(                                                                             \
    ((CFG_RCC_SYSDIV - 1)   << 23) |                                          \
    (CFG_RCC_USESYSDIV      << 22) |                                          \
    (CFG_RCC_USEPWMDIV      << 20) |                                          \
    (CFG_RCC_PWMDIV         << 17) |                                          \
    (CFG_RCC_PWRDN          << 13) |                                          \
    (CFG_RCC_BYPASS         << 11) |                                          \
    (CFG_RCC_XTAL           << 6)  |                                          \
    (CFG_RCC_OSCSRC         << 4)  |                                          \
    (CFG_RCC_IOSCDIS        << 1)  |                                          \
    (CFG_RCC_MOSCDIS        << 1)\
)

#define RCC2_Val                                                              \
(                                                                             \
    (CFG_RCC2_USERCC2      << 31) |                                           \
    ((CFG_RCC_SYSDIV2 - 1)  << 23) |                                          \
    (CFG_RCC_PWRDN2         << 13) |                                          \
    (CFG_RCC_BYPASS2        << 11) |                                          \
    (CFG_RCC_OSCSRC2        << 4)\
)


/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTALM       (16000000UL)            /* Main         oscillator freq */
#define XTALI       (16000000UL)            /* Internal     oscillator freq */
#define XTAL30K     (   30000UL)            /* Internal 30K oscillator freq */
#define XTAL32K     (   32768UL)            /* external 32K oscillator freq */

#define PLL_CLK    (400000000UL)
#define ADC_CLK     (PLL_CLK/25)
#define CAN_CLK     (PLL_CLK/50)

 /* Determine clock frequency according to clock register values */
  #if (RCC2_Val & (1UL<<31))                              /* is rcc2 used ? */
    #if (RCC2_Val & (1UL<<11))                           /* check BYPASS */
              #if   (((RCC2_Val>>4) & 0x07) == 0x0)
                #if   (((RCC_Val>>6) & 0x1F) == 0x0)
                      #define __CORE_CLK_PRE  1000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x1)
                      #define __CORE_CLK_PRE  1843200UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x2)
                      #define __CORE_CLK_PRE  2000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x3)
                      #define __CORE_CLK_PRE  2457600UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x4)
                      #define __CORE_CLK_PRE  3579545UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x5)
                      #define __CORE_CLK_PRE  3686400UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x6)
                      #define __CORE_CLK_PRE  4000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x7)
                      #define __CORE_CLK_PRE  4096000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x8)
                      #define __CORE_CLK_PRE  4915200UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x9)
                      #define __CORE_CLK_PRE  5000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xA)
                      #define __CORE_CLK_PRE  5120000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xB)
                      #define __CORE_CLK_PRE  6000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xC)
                      #define __CORE_CLK_PRE  6144000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xD)
                      #define __CORE_CLK_PRE  7372800UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xE)
                      #define __CORE_CLK_PRE  8000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xF)
                      #define __CORE_CLK_PRE  8192000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x10)
                      #define __CORE_CLK_PRE  10000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x11)
                      #define __CORE_CLK_PRE  12000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x12)
                      #define __CORE_CLK_PRE  12288000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x13)
                      #define __CORE_CLK_PRE  13560000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x14)
                      #define __CORE_CLK_PRE  14318180UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x15)
                      #define __CORE_CLK_PRE  16000000UL
                #else
                      #define __CORE_CLK_PRE  16384000UL
                #endif
              #elif (((RCC2_Val>>4) & 0x07) == 0x1)
                  #define __CORE_CLK_PRE  XTALI
              #elif (((RCC2_Val>>4) & 0x07) == 0x2)
                  #define __CORE_CLK_PRE  (XTALI/4)
              #else
                  #define __CORE_CLK_PRE  XTAL30K
              #endif
    #else
      #define __CORE_CLK_PRE   PLL_CLK
    #endif
    #if (RCC_Val & (1UL<<22))                            /* check USESYSDIV */
      #if (RCC2_Val & (1UL<<11))
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC2_Val>>23) & (0x3F)) + 1))
      #else
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC2_Val>>23) & (0x3F)) + 1) / 2)
      #endif
    #else
      #define __CORE_CLK  __CORE_CLK_PRE
    #endif
  #else
    #if (RCC_Val & (1UL<<11))                           /* check BYPASS */
              #if   (((RCC_Val>>4) & 0x03) == 0x0)
                #if   (((RCC_Val>>6) & 0x1F) == 0x0)
                      #define __CORE_CLK_PRE  1000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x1)
                      #define __CORE_CLK_PRE  1843200UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x2)
                      #define __CORE_CLK_PRE  2000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x3)
                      #define __CORE_CLK_PRE  2457600UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x4)
                      #define __CORE_CLK_PRE  3579545UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x5)
                      #define __CORE_CLK_PRE  3686400UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x6)
                      #define __CORE_CLK_PRE  4000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x7)
                      #define __CORE_CLK_PRE  4096000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x8)
                      #define __CORE_CLK_PRE  4915200UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x9)
                      #define __CORE_CLK_PRE  5000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xA)
                      #define __CORE_CLK_PRE  5120000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xB)
                      #define __CORE_CLK_PRE  6000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xC)
                      #define __CORE_CLK_PRE  6144000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xD)
                      #define __CORE_CLK_PRE  7372800UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xE)
                      #define __CORE_CLK_PRE  8000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0xF)
                      #define __CORE_CLK_PRE  8192000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x10)
                      #define __CORE_CLK_PRE  10000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x11)
                      #define __CORE_CLK_PRE  12000000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x12)
                      #define __CORE_CLK_PRE  12288000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x13)
                      #define __CORE_CLK_PRE  13560000UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x14)
                      #define __CORE_CLK_PRE  14318180UL
                #elif (((RCC_Val>>6) & 0x1F) == 0x15)
                      #define __CORE_CLK_PRE  16000000UL
                #else
                      #define __CORE_CLK_PRE  16384000UL
                #endif
              #elif (((RCC_Val>>4) & 0x03) == 0x1)
                  #define __CORE_CLK_PRE  XTALI
              #elif (((RCC_Val>>4) & 0x03) == 0x2)
                  #define __CORE_CLK_PRE  (XTALI/4)
              #else
                  #define __CORE_CLK_PRE  XTAL30K
              #endif
    #else
      #define __CORE_CLK_PRE   PLL_CLK
    #endif
    #if (RCC_Val & (1UL<<22))                            /* check USESYSDIV */
      #if (RCC_Val & (1UL<<11))                          /* check BYPASS */
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC_Val>>23) & (0x0F)) + 1))
      #else
        #define __CORE_CLK  (__CORE_CLK_PRE / (((RCC_Val>>23) & (0x0F)) + 1) / 2)
      #endif
    #else
      #define __CORE_CLK  __CORE_CLK_PRE
    #endif
  #endif


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = __CORE_CLK;  /*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Get the OSC clock
 *----------------------------------------------------------------------------*/
__INLINE static uint32_t getOscClk (uint32_t xtal, uint32_t oscSrc) {
  uint32_t oscClk = XTALI;

  switch (oscSrc) {                      /* switch OSCSRC */
    case 0:                              /* MOSC Main oscillator */
      switch (xtal) {                    /* switch XTAL */
        case 0x0:
          oscClk = 1000000UL;
          break;
        case 0x1:
          oscClk = 1843200UL;
          break;
        case 0x2:
          oscClk = 2000000UL;
          break;
        case 0x3:
          oscClk = 2457600UL;
          break;
        case 0x4:
          oscClk = 3579545UL;
          break;
        case 0x5:
          oscClk = 3686400UL;
          break;
        case 0x6:
          oscClk = 4000000UL;
          break;
        case 0x7:
          oscClk = 4096000UL;
          break;
        case 0x8:
          oscClk = 4915200UL;
          break;
        case 0x9:
          oscClk = 5000000UL;
          break;
        case 0xA:
          oscClk = 5120000UL;
          break;
        case 0xB:
          oscClk = 6000000UL;
          break;
        case 0xC:
          oscClk = 6144000UL;
          break;
        case 0xD:
          oscClk = 7372800UL;
          break;
        case 0xE:
          oscClk = 8000000UL;
          break;
        case 0xF:
          oscClk = 8192000UL;
          break;
        case 0x10:
          oscClk = 10000000UL;
          break;
        case 0x11:
          oscClk = 12000000UL;
          break;
        case 0x12:
          oscClk = 12288000UL;
          break;
        case 0x13:
          oscClk = 13560000UL;
          break;
        case 0x14:
          oscClk = 14318180UL;
          break;
        case 0x15:
          oscClk = 16000000UL;
          break;
        case 0x16:
          oscClk = 16384000UL;
          break;
       }
      break;
    case 1:                         /* IOSC Internal oscillator */
      oscClk = XTALI;
      break;
    case 2:                         /* IOSC/4 Internal oscillator/4 */
      oscClk = XTALI/4;
      break;
    case 3:                         /* 30kHz internal oscillator  */
      oscClk = XTAL30K;
      break;
  }

  return oscClk;
}

void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
    uint32_t rcc, rcc2;

    /* Determine clock frequency according to clock register values */
    rcc  = SYSCTL->RCC;
    rcc2 = SYSCTL->RCC2;

  //if (rcc2 & SYSCTL_RCC2_USERCC2)
    if (rcc2 & (1UL<<31)) {                             /* is rcc2 is used ? */
  //  if (rcc2 & SYSCTL_RCC2_BYPASS2)
      if (rcc2 & (1UL<<11)) {                           /* check BYPASS */
        SystemCoreClock = getOscClk (((rcc>>6) & 0x0F),((rcc2>>4) & 0x07));
      } else {
        SystemCoreClock = PLL_CLK;
      }
      if (rcc & (1UL<<22)) {                            /* check USESYSDIV */
        if (rcc2 & (1UL<<11)) {
          SystemCoreClock = SystemCoreClock / (((rcc2>>23) & (0x3F)) + 1);
        } else {
          SystemCoreClock = SystemCoreClock / (((rcc2>>23) & (0x3F)) + 1) / 2;
        }
      }
    } else {
  //    if (RCC_Val & (1UL<<11)) {                            /* check BYPASS */
      if (rcc & (1UL<<11)) {                            /* check BYPASS */ /* Simulation does not work at this point */
        SystemCoreClock = getOscClk (((rcc>>6) & 0x1F),((rcc>>4) & 0x03));
      } else {
        SystemCoreClock = PLL_CLK;
      }
  //  if (rcc & SYSCTL_RCC_USE_SYSDIV)
      if (rcc & (1UL<<22)) {                            /* check USESYSDIV */
  //    if (rcc2 & SYSCTL_RCC_BYPASS)
        if (rcc & (1UL<<11)) {                          /* check BYPASS */ /* Simulation does not work at this point */
  //      if (RCC_Val & (1UL<<11)) {                          /* check BYPASS */
          SystemCoreClock = SystemCoreClock / (((rcc>>23) & (0x0F)) + 1);
        } else {
          SystemCoreClock = SystemCoreClock / (((rcc>>23) & (0x0F)) + 1) / 2;
        }
      }
    }
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
#if(CLOCK_SETUP)
    uint32_t i;
#endif

  /* FPU settings ------------------------------------------------------------*/
  #if (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                   (3UL << 11*2)  );               /* set CP11 Full Access */
  #endif

#if(CLOCK_SETUP)
    SYSCTL->RCC2 = 0x07802810;    /* set default value */
    SYSCTL->RCC  = 0x078E3AD1;    /* set default value */

    SYSCTL->RCC  = (RCC_Val  | (1UL<<11) | (1UL<<13)) & ~(1UL<<22); /* set value with BYPASS, PWRDN set, USESYSDIV reset */
    SYSCTL->RCC2 = (RCC2_Val | (1UL<<11) | (1UL<<13));              /* set value with BYPASS, PWRDN set */
    for (i = 0; i < 1000; i++);   /* wait a while */

    SYSCTL->RCC  = (RCC_Val  | (1UL<<11)) & ~(1UL<<22);             /* set value with BYPASS, USESYSDIV reset */
    SYSCTL->RCC2 = (RCC2_Val | (1UL<<11));                          /* set value with BYPASS */
    for (i = 0; i < 1000; i++);   /* wait a while */

    SYSCTL->RCC  = (RCC_Val  | (1<<11));                            /* set value with BYPASS */

    if ( (((RCC_Val  & (1UL<<13)) == 0) && ((RCC2_Val & (1UL<<31)) == 0)) ||
         (((RCC2_Val & (1UL<<13)) == 0) && ((RCC2_Val & (1UL<<31)) != 0))   ) {
      while ((SYSCTL->RIS & (1UL<<6)) != (1UL<<6));                 /* wait until PLL is locked */
    }

    SYSCTL->RCC  = (RCC_Val);                                       /* set value */
    SYSCTL->RCC2 = (RCC2_Val);                                      /* set value */
    for (i = 0; i < 10000; i++);   /* wait a while */

#endif
}
