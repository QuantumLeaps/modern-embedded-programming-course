/*============================================================================
* ET: embedded test example for "LedDriver" from the book
* "Test Driven Development for Embedded C" by James W. Grenning
============================================================================*/

#include "LedDriver.h"  /* Code Under Test (CUT) */
#include "et.h"         /* ET: embedded test */

static uint16_t virtualLeds;

void setup(void) {
    LedDriver_Create(&virtualLeds);
}

void teardown(void) {
}

/* test group --------------------------------------------------------------*/
TEST_GROUP("LedDriver") {

TEST("Leds Off After Create") {
    uint16_t virtualLeds = 0xffff;
    LedDriver_Create(&virtualLeds);
    VERIFY(0 == virtualLeds);
}

TEST("Turn On Led One") {
    LedDriver_TurnOn(1);
    VERIFY(1 == virtualLeds);
}

TEST("Turn Off Led One") {
    LedDriver_TurnOn(1);
    LedDriver_TurnOff(1);
    VERIFY(0 == virtualLeds);
}

TEST("Turn On Multiple Leds") {
    LedDriver_TurnOn(9);
    LedDriver_TurnOn(8);
    VERIFY(0x180 == virtualLeds);
}

TEST("Turn Off Multiple Leds") {
    LedDriver_TurnAllOn();
    LedDriver_TurnOff(9);
    LedDriver_TurnOff(8);
    VERIFY((~0x180 & 0xffff) == virtualLeds);
}

TEST("Turn Off Any Led") {
    LedDriver_TurnAllOn();
    LedDriver_TurnOff(8);
    VERIFY(0xff7f == virtualLeds);
}

TEST("Led Memory Is Not Readable") {
    virtualLeds = 0xffff;
    LedDriver_TurnOn(8);
    VERIFY(0x80 == virtualLeds);
}

TEST("Upper And Lower Bounds") {
    LedDriver_TurnOn(1);
    LedDriver_TurnOn(16);
    VERIFY(0x8001 == virtualLeds);
}

TEST("Out Of Bounds Turn On Does No Harm") {
    LedDriver_TurnOn(-1);
    LedDriver_TurnOn(0);
    LedDriver_TurnOn(17);
    LedDriver_TurnOn(3141);
    VERIFY(0 == virtualLeds);
}

TEST("Out Of Bounds Turn Off Does No Harm") {
    LedDriver_TurnAllOn();

    LedDriver_TurnOff(-1);
    LedDriver_TurnOff(0);
    LedDriver_TurnOff(17);
    LedDriver_TurnOff(3141);
    VERIFY(0xffff == virtualLeds);
}

SKIP_TEST("Out Of Bounds To-Do") {
    /* TODO: what should we do during runtime? */
    VERIFY(0);
}

TEST("Is On") {
    VERIFY(!LedDriver_IsOn(11));
    LedDriver_TurnOn(11);
    VERIFY(LedDriver_IsOn(11));
}

TEST("Is Off") {
    VERIFY(LedDriver_IsOff(12));
    LedDriver_TurnOn(12);
    VERIFY(!LedDriver_IsOff(12));
}

TEST("Out Of Bounds Leds Are Always Off") {
    VERIFY(LedDriver_IsOff(0));
    VERIFY(LedDriver_IsOff(17));
    VERIFY(!LedDriver_IsOn(0));
    VERIFY(!LedDriver_IsOn(17));
}

TEST("All On") {
    LedDriver_TurnAllOn();
    VERIFY(0xffff == virtualLeds);
}

TEST("All Off") {
    LedDriver_TurnAllOn();
    LedDriver_TurnAllOff();
    VERIFY(0 == virtualLeds);
}

} /* TEST_GROUP() */
