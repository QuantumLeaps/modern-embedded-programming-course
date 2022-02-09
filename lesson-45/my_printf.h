/*****************************************************************************
* software tracing with printf()
*****************************************************************************/
#ifndef MY_PRINTF_H
#define MY_PRINTF_H

#ifdef SPY /* tracing enabled? */

#include <stdio.h> /* for printf() */

#define MY_PRINTF(format_, ...) printf(format_, ##__VA_ARGS__)
#define MY_PRINTF_INIT()        printf_init()

void printf_init();

#else /* tracing disabled */

#define MY_PRINTF(format_, ...) (0)
#define MY_PRINTF_INIT()        ((void)0)

#endif /* SPY */

#endif /* MY_PRINTF_H */

