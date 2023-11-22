About uC-OS2
=============
uC-OS2 (a.k.a. uC/OS-II or MicroC/OS-II) is a portable, ROMable, scalable, preemptive,
real-time deterministic multitasking kernel for microprocessors, microcontrollers and DSPs.
Offering unprecedented ease-of-use, uC-OS2 is delivered with complete 100% ANSI C
source code and in-depth documentation. uC-OS2 runs on the largest number of processor
architectures, with ports available for download from the Micrium Web site.

## For the complete documentation, visit https://doc.micrium.com/display/ucos/


uC-OS2 Source Code
==================
The uC-OS2 code ( V2.93.00) has been taken from the following GitHub repository:

https://github.com/SiliconLabs/uC-OS2

***
NOTE: The uC-OS2 from GitHub has been altered by Quantum Leaps to reduce its size.
Specifically, the Ports sub-directory has been cleaned up and only the uC-OS2 ports
relevant to the provided examples have been retained.
***


uC-OS2 Ports
=============
The folder "Ports" contains the generic uC-OS2 Ports to ARM-Cortex-M3 and ARM-Cortex-M4.

***
NOTE: The ports have been modified by Quantum Leaps to remove the dependencies on the
Micrium's uC-CPU and uC-LIB components, and instead to inter-operate with the Cortex
Microcontroller Software Interface Standard (CMSIS).
***

Modifications for CMSIS
-----------------------
Specifically, the names of the standard exception handlers PendSV and SysTick
used by the uC/OS-II ports have been renamed to be compliant with the CMSIS
naming convention.

Additionally, the Cortex-M4F ports have been modified to include the FPU
configuration that is expected by the uC/OS-II port to Cortex-M4F, which
was different from the FPU configuration performed by the CMSIS-compliant
startup code. Specifically, the explicit FPU configuration has been
OSInitHookEnd() function in the file os_cpu_c.c.


Licensing Information
=====================
uC-OS2 is provided by Silicon Labs under the Apache License, Version 2.0. Please refer
to the file license.txt in the same folder as this README.md file.
