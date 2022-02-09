/*****************************************************************************
* Product: QHsmTst Example
* Last updated for version 6.9.1
* Last updated on  2020-09-11
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses/>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/
#include "qpc.h"
#include "qhsmtst.h"

#include "safe_std.h" /* portable "safe" <stdio.h>/<string.h> facilities */
#include <stdlib.h>

Q_DEFINE_THIS_FILE

/* local objects -----------------------------------------------------------*/
static FILE *l_outFile = (FILE *)0;
static void dispatch(QSignal sig);

/*..........................................................................*/
int main(int argc, char *argv[]) {

#ifdef Q_SPY
    uint8_t qsBuf[128];
    QS_initBuf(qsBuf, sizeof(qsBuf));
#endif

    QF_init();
    QF_onStartup();

    QHsmTst_ctor(); /* instantiate the QHsmTst object */

    if (argc > 1) {   /* file name provided? */
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { /* interactive version? */
        l_outFile = stdout; /* use the stdout as the output file */

        PRINTF_S("QHsmTst example, built on %s at %s,\n"
               "QP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QP_VERSION_STR);

        QHSM_INIT(the_sm, (void *)0, 0U); /* the top-most initial tran. */

        for (;;) { /* event loop */
            QEvt e;
            int c;

            PRINTF_S("\n", "");
            c = (uint8_t)QF_consoleWaitForKey();
            PRINTF_S("%c: ", (c >= ' ') ? c : 'X');

            if ('a' <= c && c <= 'i') { /* in range? */
                e.sig = (QSignal)(c - 'a' + A_SIG);
            }
            else if ('A' <= c && c <= 'I') { /* in range? */
                e.sig = (QSignal)(c - 'A' + A_SIG);
            }
            else if (c == '\33') { /* the ESC key? */
                e.sig = TERMINATE_SIG; /* terminate the interactive test */
            }
            else {
                e.sig = IGNORE_SIG;
            }

            QHSM_DISPATCH(the_sm, &e, 0U); /* dispatch the event */
        }
    }
    else { /* batch version */
        PRINTF_S("QHsmTst example, built on %s at %s, QP %s\n"
               "output saved to %s\n",
               __DATE__, __TIME__, QP_VERSION_STR,
               argv[1]);

        FPRINTF_S(l_outFile, "QHsmTst example, QP %s\n",
                  QP_VERSION_STR);

        QHSM_INIT(the_sm, (void *)0, 0U); /* the top-most initial tran. */

        /* testing of dynamic transitions... */
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(I_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(G_SIG);
        dispatch(H_SIG);
        dispatch(H_SIG);
        dispatch(C_SIG);
        dispatch(G_SIG);
        dispatch(C_SIG);
        dispatch(C_SIG);

        fclose(l_outFile);
    }

    QF_onCleanup();
    return 0;
}
/*..........................................................................*/
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    FPRINTF_S(stderr, "Assertion failed in %s:%d\n", module, loc);
    QF_onCleanup();
    exit(-1);
}
/*..........................................................................*/
void BSP_display(char const *msg) {
    FPRINTF_S(l_outFile, "%s", msg);
}
/*..........................................................................*/
void BSP_exit(void) {
    PRINTF_S("\n%s\n", "Bye, Bye!");
    QF_onCleanup();
    exit(0);
}
/*..........................................................................*/
static void dispatch(QSignal sig) {
    QEvt e;
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    e.sig = sig;
    FPRINTF_S(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    QHSM_DISPATCH(the_sm, &e, 0U); /* dispatch the event */
}

/*--------------------------------------------------------------------------*/
void QF_onStartup(void) {
    QF_consoleSetup();
}
/*..........................................................................*/
void QF_onCleanup(void) {
    QF_consoleCleanup();
}
/*..........................................................................*/
void QF_onClockTick(void) {
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY /* define QS callbacks */

/*! callback function to execute user commands */
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    switch (cmdId) {
       case 0U: {
           break;
       }
       default:
           break;
    }

    /* unused parameters */
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/

