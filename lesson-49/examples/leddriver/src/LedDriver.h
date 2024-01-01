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


#ifndef D_LedDriver_H
#define D_LedDriver_H
#include <stdint.h>

#define TRUE 1
#define FALSE 0
typedef int BOOL;


void LedDriver_Create(uint16_t * ledsAddress);
void LedDriver_Destroy(void);

void LedDriver_TurnOn(int ledNumber);
void LedDriver_TurnOff(int ledNumber);
void LedDriver_TurnAllOn(void);
void LedDriver_TurnAllOff(void);
BOOL LedDriver_IsOn(int ledNumber);
BOOL LedDriver_IsOff(int ledNumber);
#endif  /* D_LedDriver_H */

/*
 * Intermediate examples below this comment
 */

#if 0
#ifndef D_LedDriver_H
#define D_LedDriver_H

void LedDriver_Create(void);
void LedDriver_Destroy(void);

#endif  /* D_LedDriver_H */

#if 0
#ifndef D_LedDriver_H
#define D_LedDriver_H

void LedDriver_Create(void);
void LedDriver_Destroy(void);
void LedDriver_TurnOn(int ledNumber);
void LedDriver_TurnOff(int ledNumber);

#endif


#endif  /* D_LedDriver_H */


#endif
