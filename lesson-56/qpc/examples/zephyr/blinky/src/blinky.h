//============================================================================
// Product: "Blinky" example
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef BLINKY_H_
#define BLINKY_H_

enum BlinkySignals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,          // the last published signal

    TIMEOUT_SIG,
    MAX_SIG               // the last signal
};

void Blinky_ctor(void);
extern QActive * const AO_Blinky; // opaque pointer

#endif // BLINKY_H_
