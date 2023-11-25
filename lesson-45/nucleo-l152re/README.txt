This folder contains the support code for the NUCLEO-L152RE board.


CMSIS-Compliant Device Files
============================
The code also includes the CMSIS-compliant interface to the
STM32L053xx MCU files:

stm32l1xx.h
system_stm32l1xx.h
system_stm32l1xx.c
arm\startup_stm32l1xx.s
gcc\startup_stm32l1xx.c
iar\startup_stm32l1xx.s


Adjusting the CPU Clock Speed
=============================
The current setting is to run at 2MHz from the MSI (internal oscillator),
but the CPU clock speed can be modified by editing the file
system_stm32l1xx.c. Ther file system_stm32l1xx.c.pll provides an example
of clock setting using the PLL driven from the MSE.

***
NOTE:
The NUCLEO boards have a wide range of possible clock selections, depending
on the solder bridge configuration. Please see Chapter 5.7 "OSC clock" in
the STM32 NUCLEO Boards User Manual (ST document UM1724) for more information.
***



 