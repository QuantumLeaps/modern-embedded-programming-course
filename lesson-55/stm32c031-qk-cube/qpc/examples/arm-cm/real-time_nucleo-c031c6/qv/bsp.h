//============================================================================
// "real-time" example to demonstrate timing in QP/C
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-10-02
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#ifndef BSP_H_
#define BSP_H_

#define BSP_TICKS_PER_SEC 2500U

void BSP_init(void);
void BSP_start(void);

void BSP_d1on(void);
void BSP_d1off(void);

void BSP_d2on(void);
void BSP_d2off(void);

void BSP_d3on(void);
void BSP_d3off(void);

void BSP_d4on(void);
void BSP_d4off(void);

void BSP_d5on(void);
void BSP_d5off(void);

void BSP_d6on(void);
void BSP_d6off(void);

void BSP_d7on(void);
void BSP_d7off(void);

// immutable events for Periodic active objects
QEvt const *BSP_getEvtPeriodic1(uint8_t num);
QEvt const *BSP_getEvtPeriodic4(uint8_t num);

#endif // BSP_H_
