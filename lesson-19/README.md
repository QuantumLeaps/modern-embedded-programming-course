Update for Code Composer Studio 11:
-----------------------------------
Code Composer Studio (CCS) 11 no longer comes with the GNU-ARM compiler
installed. But the GNU-ARM toolchain can be installed via the menu:

"Help | Install GCC ARM Compiler Tools..."


Changed CCS Project
-------------------
The CCS project in this lesson has been updated to use the GNU v9.2.1 (Linaro).

Also, the file startup_tm4c_gnu.c has been modified, by commenting out the
call to __libc_init_array(); (line 389)

This call caused the linker error:

error: lesson.out uses VFP register arguments, ...\libc.a(lib_a-init.o) does not 


Quantum Leaps,
2021-11-22
