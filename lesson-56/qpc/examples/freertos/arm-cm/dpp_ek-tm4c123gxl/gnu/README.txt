About this Example
==================
This example can be built from the command prompt with the provided
Makefile. The example can also be imported as a Makefile-based
project into Eclipse-based IDEs.


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
for ARM Cortex-M4F projects, due to the support for the hardware FPU
(float-abi=hard).
***


Adjusting Stack and Heap Sizes
==============================
The stack and heap sizes are determined in this project by the GCC linker
script (.ld file), which provides a template of the recommended GCC linker
script for QP applications.


Startup Code
============
The startup code for the TM4C123GH6PM MCU used in this project is
located in the "3rd_party" folder in the following location:

3rd_party\ek-tm4c123gxl\gcc\startup_TM4C123GH6PM.c

The file startup_TM4C123GH6PM.c provides a template of the recommended
startup for QP applications and should be easily customizable for other
ARM Cortex-M microcontrollers.

The startup file typically does not need to be modified or adapted for
applications. It provides only weak definitions of all exception and
interrupt handlers, as well as the assert_failed() function.

The weak function assert_failed() defined in this file might be re-defined
in the application to customize it for the application-specific error-
handling policy.

***
NOTE: The function assert_failed() typically should NOT use the stack,
because stack might be corrupted by the time this function is called.
Also, assert_failed() is intended to handle catastrophic errors and
should NOT return.
***
