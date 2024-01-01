/*
* Modified by Quantum Leaps, LLC
* commented out #include "RuntimeError.h" and RUNTIME_ERROR()
*/
/***
 * Excerpted from "Test-Driven Development for Embedded C",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material,
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose.
 * Visit http://www.pragmaticprogrammer.com/titles/jgade for more book information.
***/
/*- ------------------------------------------------------------------ -*/
/*-    Copyright (c) James W. Grenning -- All Rights Reserved          -*/
/*-    For use by owners of Test-Driven Development for Embedded C,    -*/
/*-    and attendees of Renaissance Software Consulting, Co. training  -*/
/*-    classes.                                                        -*/
/*-                                                                    -*/
/*-    Available at http://pragprog.com/titles/jgade/                  -*/
/*-        ISBN 1-934356-62-X, ISBN13 978-1-934356-62-3                -*/
/*-                                                                    -*/
/*-    Authorized users may use this source code in your own           -*/
/*-    projects, however the source code may not be used to            -*/
/*-    create training material, courses, books, articles, and         -*/
/*-    the like. We make no guarantees that this source code is        -*/
/*-    fit for any purpose.                                            -*/
/*-                                                                    -*/
/*-    www.renaissancesoftware.net james@renaissancesoftware.net       -*/
/*- ------------------------------------------------------------------ -*/

//#include "RuntimeError.h"
#include "LedDriver.h"

enum {ALL_LEDS_OFF = 0, ALL_LEDS_ON = ~0};

static uint16_t * ledsAddress;
static uint16_t ledsImage;

void LedDriver_Create(uint16_t * address)
{
    ledsAddress = address;
    ledsImage = ALL_LEDS_OFF;
    *ledsAddress = ledsImage;
}

void LedDriver_Destroy(void)
{
}

enum {FIRST_LED = 1, LAST_LED = 16};

static BOOL IsLedOutOfBounds(int ledNumber)
{
    if ((ledNumber < FIRST_LED) || (ledNumber > LAST_LED))
        {
            //RUNTIME_ERROR("LED Driver: out-of-bounds LED", ledNumber);
            return TRUE;
        }
    return FALSE;
}

static uint16_t convertLedNumberToBit(int ledNumber)
{
    return 1 << (ledNumber - 1);
}

static void updateHardware(void)
{
    *ledsAddress = ledsImage;
}

static void setLedImageBit(int ledNumber)
{
    ledsImage |= convertLedNumberToBit(ledNumber);
}
void LedDriver_TurnOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    setLedImageBit(ledNumber);
    updateHardware();
}

static void clearLedImageBit(int ledNumber)
{
    ledsImage &= ~convertLedNumberToBit(ledNumber);
}
void LedDriver_TurnOff(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    clearLedImageBit(ledNumber);
    updateHardware();
}

void LedDriver_TurnAllOn(void)
{
    ledsImage = ALL_LEDS_ON;
    updateHardware();
}

void LedDriver_TurnAllOff(void)
{
    ledsImage = ALL_LEDS_OFF;
    updateHardware();
}

BOOL LedDriver_IsOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return FALSE;

    return 0 != (ledsImage & convertLedNumberToBit(ledNumber));
}

BOOL LedDriver_IsOff(int ledNumber)
{
     return !LedDriver_IsOn(ledNumber);
}


/*
 * Intermediate examples below this comment
 */

#if 0
BOOL LedDriver_IsOff(int ledNumber)
{
     return FALSE; /* !LedDriver_IsOn(ledNumber); */
}
#endif


#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    return 0 != (ledsImage & convertLedNumberToBit(ledNumber));
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    return TRUE;
    /* return 0 != (ledsImage & convertLedNumberToBit(ledNumber)); */
}
#endif


#if 0
enum {ALL_LEDS_ON = ~0, ALL_LEDS_OFF = ~ALL_LEDS_ON};

static uint16_t * ledsAddress;
static uint16_t ledsImage;

void LedDriver_Create(uint16_t * address)
{
    ledsAddress = address;
    ledsImage = ALL_LEDS_OFF;
    *ledsAddress = ledsImage;
}

void LedDriver_Destroy(void)
{
}

#if 0
enum {FIRST_LED = 1, LAST_LED = 16};
static BOOL IsLedOutOfBounds(int ledNumber)
{
    return (ledNumber < FIRST_LED) || (ledNumber > LAST_LED);
}
#endif

enum {FIRST_LED = 1, LAST_LED = 16};

static BOOL IsLedOutOfBounds(int ledNumber)
{
    if ((ledNumber < FIRST_LED) || (ledNumber > LAST_LED))
        {
            RUNTIME_ERROR("LED Driver: out-of-bounds LED", ledNumber);
            return TRUE;
        }
    return FALSE;
}

static uint16_t convertLedNumberToBit(int ledNumber)
{
    return 1 << (ledNumber - 1);
}

static void updateHardware(void)
{
    *ledsAddress = ledsImage;
}

static void setLedImageBit(int ledNumber)
{
    ledsImage |= convertLedNumberToBit(ledNumber);
    updateHardware();
}


void LedDriver_TurnOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    setLedImageBit(ledNumber);
}

static void clearLedImageBit(int ledNumber)
{
    ledsImage &= ~convertLedNumberToBit(ledNumber);
    updateHardware();
}

void LedDriver_TurnOff(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    clearLedImageBit(ledNumber);
}


void LedDriver_TurnAllOn(void)
{
    ledsImage = ALL_LEDS_ON;
    updateHardware();
}

void LedDriver_TurnAllOff(void)
{
    ledsImage = ALL_LEDS_OFF;
    updateHardware();
}

BOOL LedDriver_IsOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return FALSE;

    return 0 != (ledsImage & convertLedNumberToBit(ledNumber));
}

BOOL LedDriver_IsOff(int ledNumber)
{
     return !LedDriver_IsOn(ledNumber);
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    return ledsImage & (convertLedNumberToBit(ledNumber));
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return FALSE;

    return ledsImage & (convertLedNumberToBit(ledNumber));
}
#endif


#if 0
void LedDriver_TurnOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    ledsImage |= convertLedNumberToBit(ledNumber);
    updateHardware();
}
void LedDriver_TurnOff(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    ledsImage &= ~(convertLedNumberToBit(ledNumber));
    updateHardware();
}
#endif

#if 0
#include "LedDriver.h"

void LedDriver_Create(uint16_t * address)
{
}

void LedDriver_Destroy(void)
{
}
#endif

#if 0
void LedDriver_Create(uint16_t * address)
{
    *address = 0;
}
#endif

#if 0
void LedDriver_Create(uint16_t * address)
{
    ledsAddress = address;
    *ledsAddress = 0;
}
#endif


#if 0
void LedDriver_TurnOn(int ledNumber)
{
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    *ledsAddress = 1;
}
#endif


#if 0
#include "LedDriver.h"

static uint16_t * ledsAddress;

void LedDriver_Create(uint16_t * address)
{
    ledsAddress = address;
    *ledsAddress = 0;
}

void LedDriver_Destroy(void)
{
}

void LedDriver_TurnOn(int ledNumber)
{
    *ledsAddress = 1;
}

void LedDriver_TurnOff(int ledNumber)
{
    *ledsAddress = 0;
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    *ledsAddress |= (1 << ledNumber);
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    *ledsAddress |= 1 << (ledNumber - 1);
}
#endif

#if 0
static uint16_t convertLedNumberToBit(int ledNumber)
{
    return  1 << (ledNumber - 1);
}
void LedDriver_TurnOn(int ledNumber)
{
    *ledsAddress |= convertLedNumberToBit(ledNumber);
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    ledsImage |= convertLedNumberToBit(ledNumber);
    *ledsAddress = ledsImage;
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    if (ledNumber <= 0 || ledNumber > 16)
        return;

    ledsImage |= convertLedNumberToBit(ledNumber);
    updateHardware();
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    /* if (IsLedOutOfBounds(ledNumber)) */
    /*  return; */

    if (ledNumber <= 0 || ledNumber > 16)
        return;

    ledsImage |= convertLedNumberToBit(ledNumber);
    updateHardware();
}
#endif

#if 0
void LedDriver_TurnOff(int ledNumber)
{
    *ledsAddress = 0;
}
#endif

#if 0
void LedDriver_TurnOff(int ledNumber)
{
    *ledsAddress &= ~(convertLedNumberToBit(ledNumber));
}
#endif

#if 0
void LedDriver_TurnOff(int ledNumber)
{
    ledsImage &= ~(convertLedNumberToBit(ledNumber));
    *ledsAddress = ledsImage;
}
#endif
#if 0

void LedDriver_TurnOff(int ledNumber)
{
    if (ledNumber <= 0 || ledNumber > 16)
        return;

    ledsImage &= ~(convertLedNumberToBit(ledNumber));
    updateHardware();
}
#endif

#if 0
void LedDriver_TurnAllOn(void)
{
    *ledsAddress = 0xffff;
}
#endif

#if 0
enum {ALL_LEDS_ON = ~0, ALL_LEDS_OFF = ~ALL_LEDS_ON};

void LedDriver_TurnAllOn(void)
{
    *ledsAddress = ALL_LEDS_ON;
}
#endif

#if 0
void LedDriver_TurnAllOn(void)
{
    ledsImage = ALL_LEDS_ON;
    *ledsAddress = ledsImage;
}
#endif

#if 0
void LedDriver_TurnAllOn(void)
{
}
#endif

#if 0
void LedDriver_TurnAllOff(void)
{
    ledsImage = ALL_LEDS_OFF;
    updateHardware();
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    return FALSE;
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    return TRUE;
}
#endif

#if 0
BOOL LedDriver_IsOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return FALSE;

    return FALSE;
}
#endif

#if 0
void LedDriver_TurnOn(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    ledsImage |= convertLedNumberToBit(ledNumber);
    updateHardware();
}

void LedDriver_TurnOff(int ledNumber)
{
    if (IsLedOutOfBounds(ledNumber))
        return;

    ledsImage &= ~convertLedNumberToBit(ledNumber);
    updateHardware();
}
#endif

#if 0
void LedDriver_TurnAllOn(void)
{
    ledsImage = ALL_LEDS_ON;
    *ledsAddress = ledsImage;
}

void LedDriver_TurnAllOff(void)
{
    ledsImage = ALL_LEDS_OFF;
    *ledsAddress = ledsImage;
}
#endif

