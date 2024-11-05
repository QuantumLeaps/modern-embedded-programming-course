This directory contains the support code for the EK-TM4C123GXL board
(TivaC LauchPad).

The sub-directories contain code that is specific to the particular
ARM toolchains, such as ARM (MDK-ARM), GCC, and IAR.


CMSIS-Compliant Device Files
============================
The code also includes the CMSIS-compliant interface to the TM4C123GH6PM
MCU files:

TM4C123GH6PM.h
system_TM4C123GH6PM.h
system_TM4C123GH6PM.c
arm\startup_TM4C123GH6PM.s
gcc\startup_TM4C123GH6PM.c
iar\startup_TM4C123GH6PM.s


Adjusting the CPU Clock Speed
-----------------------------
The current setting is to run at 50MHz from PLL, but the CPU clock speed
can be modified by editing the file system_TM4C123GH6PM.c.
