About this Example
==================
This example can be built in two different ways:
- from the command prompt with the provided Makefile.
- from the Atollic TRUEStudio-ST32 with the provided project.


The Makefile
============
The provided Makefile should be easy to adapt for your own projects.
It contains three build configurations: Debug (default), Release, and
Spy.

Also, the Makefile has been specifically designed to work as an external
Makefile with the Eclipse CDT.

The various build configurations are built as follows:

make
make CONF=rel
make CONF=spy

make clean
make CONF=rel clean
make CONF=spy clean

***
NOTE:
The installation folder of the GNU-ARM toolset on YOUR machine needs
to be adjusted in the provided Makefile, by editing the symbol: GNU_ARM.
As described in the comment for this symbol, the GNU-ARM toolset is taken
from: http://gnutoolchains.com/arm-eabi

It is highly recommended to use the same GNU-ARM distribution, especially
for ARM Cortex-M projects, due to the support for the hardware FPU
(float-abi=hard).
***


Selecting QK Exception
======================
The QK kernel needs a dedicated exception to return to the thread
context after preemption. The default is to use the NMI exception
for that purpose. However, in case NMI is needed for some other
purpose, the QK port allows you to select a any, otherwise unused
IRQ for that purpose. To choose a given IRQ, you need to define
the macros QK_USE_IRQ_NUM and QK_USE_IRQ_HANDLER. These macros can
be provided on the command-line to the compiler.

For example, (see the vector table), you can define the QK IRQ
as follows:

QK_USE_IRQ_HANDLER=Reserved31_IRQHandler
QK_USE_IRQ_NUM=31


Adjusting Stack and Heap Sizes
==============================
The stack and heap sizes are determined in this project by the GCC linker
script (.ld file), which provides a template of the recommended GCC linker
script for QP applications.


Startup Code
============
The startup code for the STM32C0xx MCU used in this project is
located in the "3rd_party" folder in the following location:

3rd_party\nucleo-c031c6\gnu\startup_stm32c031xx.c

The file startup_stm32c031xx.c provides a template of the recommended
startup for QP applications and should be easily customizable for other
ARM Cortex-M microcontrollers.

The startup file typically does not need to be modified or adapted for
applications. It calls the assert_failed() function, which must be
defined in the application to customize it for the application-specific
error-handling policy.


Adjusting the CPU Clock Speed
=============================
The current setting is to run at 12MHz from the MSI (internal oscillator),
but the CPU clock speed can be modified.

***
NOTE:
The NUCLEO boards have a wide range of possible clock selections, depending
on the solder bridge configuration. Please see Chapter 5.7 "OSC clock" in
the STM32 NUCLEO Boards User Manual (ST document UM1724) for more information.
***

