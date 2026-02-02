//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// licenses. Please contact Quantum Leaps for more information about the
// available licensing options.
//
// RESTRICTIONS
// You may NOT :
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software,
// (b) remove or alter any trademark, logo, copyright or other proprietary
//     notices, legends, symbols or labels present in this software,
// (c) plagiarize this software to sidestep the licensing obligations.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL           // this is QP implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface

//============================================================================
//! @cond INTERNAL

//............................................................................
//! @static @private @memberof QS
void QS_u64_raw_(uint64_t const d) {
    uint8_t chksum      = QS_priv_.chksum;
    uint8_t * const buf = QS_priv_.buf;
    QSCtr head          = QS_priv_.head;
    QSCtr const end     = QS_priv_.end;

    QS_priv_.used += 8U; // 8 bytes are about to be added
    uint64_t u64 = d;
    for (uint_fast8_t i = 8U; i != 0U; --i) {
        uint8_t const b = (uint8_t)u64;
        QS_INSERT_ESC_BYTE_(b)
        u64 >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//! @static @private @memberof QS
void QS_u64_fmt_(
    uint8_t const format,
    uint64_t const d)
{
    uint8_t chksum      = QS_priv_.chksum;
    uint8_t * const buf = QS_priv_.buf;
    QSCtr head          = QS_priv_.head;
    QSCtr const end     = QS_priv_.end;

    QS_priv_.used += 9U; // 9 bytes are about to be added

    // insert the format byte (NOTE: might need escaping)
    QS_INSERT_ESC_BYTE_(format)

    // output 8 bytes of data...
    uint64_t u64 = d;
    for (uint_fast8_t i = 8U; i != 0U; --i) {
        uint8_t const b = (uint8_t)u64;
        QS_INSERT_ESC_BYTE_(b)
        u64 >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//! @endcond
