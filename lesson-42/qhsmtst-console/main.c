//============================================================================
// Product: QHsmTst Example
// Last updated for version 7.3.0
// Last updated on  2023-08-15
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"      // QP/C framework
#include "qhsmtst.h"  // QHsmTst state machine

#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>   // for exit()

Q_DEFINE_THIS_FILE

// local objects -----------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QSignal sig);

//............................................................................
int main(int argc, char *argv[]) {

    QHsmTst_ctor(); // instantiate the QHsmTst object

    if (argc > 1) {   // file name provided?
        FOPEN_S(l_outFile, argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { // interactive version?
        l_outFile = stdout; // use the stdout as the output file

        PRINTF_S("QHsmTst example, built on %s at %s,\n"
               "QP: %s.\nEnter x or X to quit...\n",
               __DATE__, __TIME__, QP_VERSION_STR);

        QASM_INIT(the_sm, (void *)0, 0U); // the top-most initial tran.

        for (;;) { // event loop
            PRINTF_S("\n%s", ">>>");
            char inp[4];
            scanf("%1s", inp); // input the event

            QEvt e = QEVT_INITIALIZER(0U);
            if ('a' <= inp[0] && inp[0] <= 'i') { // in range?
                e.sig = (QSignal)(inp[0] - 'a' + A_SIG);
            }
            else if ('A' <= inp[0] && inp[0] <= 'I') { // in range?
                e.sig = (QSignal)(inp[0] - 'A' + A_SIG);
            }
            else if ((inp[0] == 'x') || (inp[0] == 'X')) { // x or X?
                e.sig = TERMINATE_SIG; // terminate the interactive test
            }
            else {
                e.sig = IGNORE_SIG;
            }

            QASM_DISPATCH(the_sm, &e, 0U); // dispatch the event
        }
    }
    else { // batch version
        PRINTF_S("QHsmTst example, built on %s at %s, QP %s\n"
               "output saved to %s\n",
               __DATE__, __TIME__, QP_VERSION_STR,
               argv[1]);

        FPRINTF_S(l_outFile, "QHsmTst example, QP %s\n",
                  QP_VERSION_STR);

        QASM_INIT(the_sm, (void *)0, 0U); // the top-most initial tran.

        // testing of dynamic transitions...
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

    return 0;
}
//............................................................................
Q_NORETURN Q_onError(char const * const module, int_t const id) {
    FPRINTF_S(stderr, "ERROR in %s:%d", module, id);
    exit(-1);
}
//............................................................................
void BSP_display(char const *msg) {
    FPRINTF_S(l_outFile, "%s", msg);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    PRINTF_S("\n%s\n", "Bye, Bye!");
    exit(result);
}
//............................................................................
static void dispatch(QSignal sig) {
    QEvt e = QEVT_INITIALIZER(0U);
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    e.sig = sig;
    FPRINTF_S(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    QASM_DISPATCH(the_sm, &e, 0U); // dispatch the event
}

