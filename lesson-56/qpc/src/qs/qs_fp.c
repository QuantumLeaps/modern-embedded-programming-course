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
void QS_f32_fmt_(
    uint8_t const format,
    float32_t const f)
{
    union F32Rep {
        float32_t f;
        uint32_t  u;
    } fu32; // the internal binary representation
    uint8_t chksum      = QS_priv_.chksum; // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf;
    QSCtr head          = QS_priv_.head;
    QSCtr const end     = QS_priv_.end;

    fu32.f = f; // assign the binary representation

    QS_priv_.used += 5U; // 5 bytes about to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_F32_FMT (0x6) in the lower nibble, which can
    // never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(format) // no escaping
    chksum += (uint8_t)format; // update checksum

    // insert 4 bytes...
    for (uint_fast8_t i = 4U; i != 0U; --i) {
        QS_INSERT_ESC_BYTE_((uint8_t)fu32.u)
        fu32.u >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
//! @static @private @memberof QS
void QS_f64_fmt_(
    uint8_t const format,
    float64_t const d)
{
    union F64Rep {
        float64_t d;
        uint32_t  u[2];
    } fu64; // the internal binary representation
    uint8_t chksum      = QS_priv_.chksum;
    uint8_t * const buf = QS_priv_.buf;
    QSCtr head          = QS_priv_.head;
    QSCtr const end     = QS_priv_.end;

    // static constant union to detect endianness of the machine
    static union U32Rep {
        uint32_t u32;
        uint8_t  u8[4];
    } const endian = { (uint32_t)0x7F000000U };

    fu64.d = d; // assign the binary representation

    // is this a big-endian machine?
    if (endian.u8[0] != 0U) {
        // swap fu64.u[0] <-> fu64.u[1]...
        uint32_t const tmp = fu64.u[0];
        fu64.u[0] = fu64.u[1];
        fu64.u[1] = tmp;
    }

    QS_priv_.used += 9U; // 9 bytes about to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_F64_FMT (0x7) in the lower nibble, which can
    // never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(format) // no escaping
    chksum += (uint8_t)format; // update checksum

    // output 4 bytes from fu64.u[0]...
    for (uint_fast8_t i = 4U; i > 0U; --i) {
        QS_INSERT_ESC_BYTE_((uint8_t)fu64.u[0])
        fu64.u[0] >>= 8U;
    }

    // output 4 bytes from fu64.u[1]...
    for (uint_fast8_t i = 4U; i > 0U; --i) {
        QS_INSERT_ESC_BYTE_((uint8_t)fu64.u[1])
        fu64.u[1] >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//! @endcond
