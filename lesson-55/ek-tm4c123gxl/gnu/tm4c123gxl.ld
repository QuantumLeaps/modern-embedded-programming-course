/*****************************************************************************
* Linker script for EK-TM4C123GXL, GNU-ARM linker
*
* Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
*
* This software is dual-licensed under the terms of the open-source GNU
* General Public License (GPL) or under the terms of one of the closed-
* source Quantum Leaps commercial licenses.
*
* Redistributions in source code must retain this top-level comment block.
* Plagiarizing this software to sidestep the license obligations is illegal.
*
* NOTE:
* The GPL does NOT permit the incorporation of this code into proprietary
* programs. Please contact Quantum Leaps for commercial licensing options,
* which expressly supersede the GPL and are designed explicitly for
* closed-source distribution.
*
* Quantum Leaps contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(Reset_Handler) /* entry Point */

MEMORY { /* memory map of Tiva TM4C123GH6PM */
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 32K
}

/* The size of the stack used by the application. NOTE: you need to adjust  */
STACK_SIZE = 2048;

/* The size of the heap used by the application. NOTE: you need to adjust   */
HEAP_SIZE = 0;

SECTIONS {
    /* Startup code into ROM type memory */
    .isr_vector : {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Startup code */
        . = ALIGN(4);
    } >ROM

    /* The program code and other data into ROM type memory */
    .text : {
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;        /* define a global symbols at end of code */
    } >ROM

    /* Constant data into ROM type memory */
    .rodata : {
        . = ALIGN(4);
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >ROM

    /* NOTE:
    * The "READONLY" keyword is only supported in GCC11 and later,
    * remove it if using GCC10 or earlier.
    */
    .ARM.extab (READONLY) : {
        . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } >ROM

    .ARM (READONLY) : {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } >ROM

    .preinit_array (READONLY) : {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
    } >ROM

    .init_array (READONLY) : {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
    } >ROM

    .fini_array (READONLY) : {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
    } >ROM

    /* RAM ------------------------------------------------------------*/
    .stack : {
        . = ALIGN(8);
        __stack_start__ = .;
        . = . + STACK_SIZE;
        . = ALIGN(4);
        __stack_end__ = .;
        . = ALIGN(8);
    } >RAM

    /* Used by the startup to initialize data */
    __data_load = LOADADDR(.data);

    /* Initialized data sections into "RAM" Ram type memory */
    .data : {
        . = ALIGN(4);
        __data_start = .;  /* create a global symbol at data start */
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(.RamFunc)        /* .RamFunc sections */
        *(.RamFunc*)       /* .RamFunc* sections */
        . = ALIGN(4);
        __data_end__ = .;  /* define a global symbol at data end */
        _edata = __data_end__;
    } >RAM AT> ROM

    /* Uninitialized data section into "RAM" Ram type memory */
    . = ALIGN(4);
    .bss : {
        /* This is used by the startup in order to initialize the .bss section */
        __bss_start__ = .; /* define a global symbol at bss start */
        _sbss = __bss_start__;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end__ = .;    /* define a global symbol at bss end */
        _ebss = __bss_end__;
    } >RAM

    .heap : {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        __heap_start__ = .;
        . = . + HEAP_SIZE;
        . = ALIGN(4);
        __heap_end__ = .;
        . = ALIGN(8);
    } >RAM

    /* Remove information from the standard libraries */
    /DISCARD/ : {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }
}
