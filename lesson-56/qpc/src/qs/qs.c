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
#include "qs_pkg.h"       // QS package-scope interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#include "qstamp.h"       // QP time-stamp

Q_DEFINE_THIS_MODULE("qs")

// ensure that the predefined records don't overlap the
// user records (application-specific).
Q_ASSERT_STATIC((enum_t)QS_PRE_MAX <= (enum_t)QS_USER);

//............................................................................
//! @static @public @memberof QS
void QS_initBuf(
    uint8_t * const sto,
    uint_fast32_t const stoSize)
{
    QS_priv_.buf      = sto;
    QS_priv_.end      = (QSCtr)stoSize;

    QS_glbFilter_(-QS_GRP_ALL); // all global filters OFF
    QS_locFilter_(QS_IDS_ALL);  // all local filters ON

    // produce an empty record to "flush" the QS trace buffer
    QS_beginRec_((uint_fast8_t)QS_EMPTY);
    QS_endRec_();

    // produce the reset record to inform QSPY of a new session
    QS_target_info_pre_(QS_TARGET_RESET);

    // hold off flushing after successful initialization (see QS_INIT())
}

//............................................................................
//! @static @public @memberof QS
uint16_t QS_getByte(void) {
    // NOTE: Must be called IN critical section.
    // Also requires system-level memory access (QF_MEM_SYS()).

    uint16_t ret;
    if (QS_priv_.used == 0U) {
        ret = QS_EOD; // set End-Of-Data
    }
    else {
        uint8_t const * const buf = QS_priv_.buf; // put in a temporary
        QSCtr tail = QS_priv_.tail; // put in a temporary (register)
        ret = (uint16_t)buf[tail]; // set the byte to return
        ++tail; // advance the tail
        if (tail == QS_priv_.end) { // tail wrap around?
            tail = 0U;
        }
        QS_priv_.tail = tail; // update the tail
        QS_priv_.used -= 1U;      // one less byte used
    }
    return ret; // return the byte or EOD
}

//............................................................................
//! @static @public @memberof QS
uint8_t const * QS_getBlock(uint16_t * const pNbytes) {
    // NOTE: Must be called IN critical section.
    // Also requires system-level memory access (QF_MEM_SYS()).

    QSCtr const used = QS_priv_.used; // put in a temporary (register)
    uint8_t const *buf = (uint8_t *)0; // no bytes available right now

    if (used != 0U) { // any bytes used in the ring buffer?
        QSCtr tail      = QS_priv_.tail; // put in a temporary (register)
        QSCtr const end = QS_priv_.end;  // put in a temporary (register)
        QSCtr n = (QSCtr)(end - tail);
        if (n > used) {
            n = used;
        }
        if (n > (QSCtr)(*pNbytes)) {
            n = (QSCtr)(*pNbytes);
        }
        *pNbytes = (uint16_t)n; // n-bytes available
        buf = &QS_priv_.buf[tail]; // the bytes are at the tail

        QS_priv_.used = (QSCtr)(used - n);
        tail += n;
        if (tail == end) {
            tail = 0U;
        }
        QS_priv_.tail = tail;
    }
    else { // no bytes available
        *pNbytes = 0U;      // no bytes available right now
    }
    return buf;
}

//! @static @private @memberof QS
QS_Filter QS_filt_;

//============================================================================
//! @cond INTERNAL

//! @static @private @memberof QS
QS QS_priv_;

//............................................................................
bool QS_fltCheck_(uint_fast8_t const recIdx,
                  uint_fast32_t const recBit,
                  uint_fast8_t const qsId)
{
    // check the global filter (for the QS record ID) and
    // check the local filter (for the QS ID)
    return ((QS_filt_.glb[recIdx] & recBit) != 0U)
           && ((QS_filt_.loc[qsId >> 5U]
               & ((uint32_t)1U << (qsId & 0x1FU))) != 0U);
}
//............................................................................
bool QS_glbCheck_(uint_fast8_t const recIdx, uint_fast32_t const recBit) {
    // check the global filter (for the QS record ID)
    return (QS_filt_.glb[recIdx] & recBit) != 0U;
}
//............................................................................
bool QS_locCheck_(uint_fast8_t const qsId) {
    // check the local filter (for the QS ID)
    return (QS_filt_.loc[qsId >> 5U] & ((uint32_t)1U << (qsId & 0x1FU))) != 0U;
}
//............................................................................
void QS_glbFilter_(int_fast16_t const filterSpec) {
    // NOTE: the order must match "QS record groups" in qs.h
    static uint32_t const glb_grp_mask[16][4] = {
        { 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU }, // QS_GRP_ALL
        { 0x000003FEU, 0x80000000U, 0x00000003U, 0x00000000U }, // QS_GRP_SM
        { 0x0007FC00U, 0x00002000U, 0x00020000U, 0x00000000U }, // QS_GRP_AO
        { 0x00780000U, 0x00004000U, 0x00000000U, 0x00000000U }, // QS_GRP_EQ
        { 0x03000000U, 0x00008000U, 0x00000000U, 0x00000000U }, // QS_GRP_MP
        { 0x00000000U, 0x0000003FU, 0x00000000U, 0x00000000U }, // QS_GRP_TE
        { 0xFC800000U, 0x00001FC0U, 0x00000000U, 0x00000000U }, // QS_GRP_QF
        { 0x00000000U, 0x003F0000U, 0x00000000U, 0x00000000U }, // QS_GRP_SC
        { 0x00000000U, 0x00000000U, 0x00000780U, 0x00000000U }, // QS_GRP_SEM
        { 0x00000000U, 0x00000000U, 0x0001F800U, 0x00000000U }, // QS_GRP_MTX
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x000001F0U }, // QS_GRP_U0
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x00003E00U }, // QS_GRP_U1
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x0007C000U }, // QS_GRP_U2
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x00F80000U }, // QS_GRP_U3
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x1F000000U }, // QS_GRP_U4
        { 0x00000000U, 0x00000000U, 0x00000000U, 0x1FFFFFF0U }  // QS_GRP_UA
    };
    bool const isRemove = (filterSpec < 0);
    uint8_t const filter = isRemove
        ? ((uint8_t)(-filterSpec) & 0xFFU)
        : ((uint8_t)filterSpec & 0xFFU);

    if ((filter & 0xF0U) == 0xF0U) { // is the filter a QS group of records?
        uint8_t const group = (filter & 0x0FU);
        // NOTE: group number is guaranteed to be < Q_DIM(glb_grp_mask)
        uint32_t const * const mask = glb_grp_mask[group];
        if (isRemove) {
            QS_filt_.glb[0] &= (uint32_t)~mask[0];
            QS_filt_.glb[1] &= (uint32_t)~mask[1];
            QS_filt_.glb[2] &= (uint32_t)~mask[2];
            QS_filt_.glb[3] &= (uint32_t)~mask[3];
        }
        else {
            QS_filt_.glb[0] |= mask[0];
            QS_filt_.glb[1] |= mask[1];
            QS_filt_.glb[2] |= mask[2];
            QS_filt_.glb[3] |= mask[3];
        }
    }
    else { // filter is an individual QS record
        uint8_t const rec = (filter & 0x7FU);
        QS_CRIT_STAT
        QS_CRIT_ENTRY();
        // QS rec number must be below 0x7D, so no need for escaping
        Q_ASSERT_INCRIT(120, rec < 0x7DU);
        QS_CRIT_EXIT();

        if (isRemove) {
            QS_filt_.glb[rec >> 5U] &=
                (uint32_t)(~((uint32_t)1U << (rec & 0x1FU)));
        }
        else {
            QS_filt_.glb[rec >> 5U] |= ((uint32_t)1U << (rec & 0x1FU));
        }
    }
    // ensure that the following records are always on
    QS_filt_.glb[0] |= 0x00000001U;
    QS_filt_.glb[1] |= 0xFC000000U;
    QS_filt_.glb[2] |= 0x0000007FU;

    // ensure that the last 3 records (0x7D, 0x7E, 0x7F) are always off
    QS_filt_.glb[3] &= 0x1FFFFFFFU;
}

//............................................................................
void QS_locFilter_(int_fast16_t const filterSpec) {
    // NOTE: the order must match "QS ID groups" in qs.h
    static uint32_t const loc_grp_mask[5][4] = {
        { 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU }, // QS_IDS_ALL
        { 0xFFFFFFFFU, 0xFFFFFFFFU, 0x00000000U, 0x00000000U }, // QS_IDS_AO
        { 0x00000000U, 0x00000000U, 0x0000FFFFU, 0x00000000U }, // QS_IDS_EP
        { 0x00000000U, 0x00000000U, 0xFFFF0000U, 0x00000000U }, // QS_IDS_EQ
        { 0x00000000U, 0x00000000U, 0x00000000U, 0xFFFFFFFFU }  // QS_IDS_AP
    };
    bool const isRemove = (filterSpec < 0);
    uint8_t const filter = isRemove
        ? ((uint8_t)(-filterSpec) & 0xFFU)
        : ((uint8_t)filterSpec & 0xFFU);

    if ((filter & 0xF0U) == 0xF0U) { // is the filter a QS group of records?
        uint8_t const group = (filter & 0x0FU);

        QS_CRIT_STAT
        QS_CRIT_ENTRY();
        // group number must be in range
        Q_ASSERT_INCRIT(210, group < Q_DIM(loc_grp_mask));
        QS_CRIT_EXIT();

        uint32_t const * const mask = loc_grp_mask[group];
        if (isRemove) {
            QS_filt_.loc[0] &= (uint32_t)~mask[0];
            QS_filt_.loc[1] &= (uint32_t)~mask[1];
            QS_filt_.loc[2] &= (uint32_t)~mask[2];
            QS_filt_.loc[3] &= (uint32_t)~mask[3];
        }
        else {
            QS_filt_.loc[0] |= mask[0];
            QS_filt_.loc[1] |= mask[1];
            QS_filt_.loc[2] |= mask[2];
            QS_filt_.loc[3] |= mask[3];
        }
    }
    else { // filter is an individual QS ID
        uint8_t const qsId = (filter & 0x7FU);
        QS_CRIT_STAT
        QS_CRIT_ENTRY();
        // qsId must be in range
        Q_ASSERT_INCRIT(220, qsId < 0x7FU);
        QS_CRIT_EXIT();

        if (isRemove) {
            QS_filt_.loc[qsId >> 5U] &=
                (uint32_t)(~((uint32_t)1U << (qsId & 0x1FU)));
        }
        else {
            QS_filt_.loc[qsId >> 5U] |= ((uint32_t)1U << (qsId & 0x1FU));
        }
    }
    // ensure that QS_ID==0 is always on
    QS_filt_.loc[0] |= 0x00000001U;
}

//............................................................................
void QS_beginRec_(uint_fast8_t const rec) {
    uint8_t const b = (uint8_t)(QS_priv_.seq + 1U);
    uint8_t chksum  = 0U;                // reset the checksum
    uint8_t * const buf = QS_priv_.buf;  // put in a temporary (register)
    QSCtr head          = QS_priv_.head; // put in a temporary (register)
    QSCtr const end     = QS_priv_.end;  // put in a temporary (register)

    QS_priv_.seq = b; // store the incremented sequence num
    QS_priv_.used += 2U; // 2 bytes about to be added

    QS_INSERT_ESC_BYTE_(b)

    chksum = (uint8_t)(chksum + rec);
    QS_INSERT_BYTE_((uint8_t)rec) // no need for escaping

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_endRec_(void) {
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr   head        = QS_priv_.head;
    QSCtr const end     = QS_priv_.end;
    uint8_t b = QS_priv_.chksum;
    b ^= 0xFFU;   // invert the bits in the checksum

    QS_priv_.used += 2U; // 2 bytes about to be added

    if ((b != QS_FRAME) && (b != QS_ESC)) {
        QS_INSERT_BYTE_(b)
    }
    else {
        QS_INSERT_BYTE_(QS_ESC)
        QS_INSERT_BYTE_((uint8_t)(b ^ QS_ESC_XOR))
        QS_priv_.used += 1U; // account for the ESC byte
    }

    QS_INSERT_BYTE_(QS_FRAME) // do not escape this QS_FRAME

    QS_priv_.head = head; // save the head

    if (QS_priv_.used > end) { // overrun over the old data?
        QS_priv_.used = end;   // the whole buffer is used
        QS_priv_.tail = head;  // shift the tail to the old data
    }
#ifdef QS_FLUSH_ALL
    QS_onFlush();
#endif
}

//............................................................................
void QS_u8_raw_(uint8_t const d) {
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)

    QS_priv_.used += 1U; // 1 byte about to be added
    QS_INSERT_ESC_BYTE_(d)

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_2u8_raw_(
    uint8_t const d1,
    uint8_t const d2)
{
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)

    QS_priv_.used += 2U; // 2 bytes are about to be added
    QS_INSERT_ESC_BYTE_(d1)
    QS_INSERT_ESC_BYTE_(d2)

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_u16_raw_(uint16_t const d) {
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)
    uint16_t x      = d;

    QS_priv_.used += 2U; // 2 bytes are about to be added

    QS_INSERT_ESC_BYTE_((uint8_t)x)
    x >>= 8U;
    QS_INSERT_ESC_BYTE_((uint8_t)x)

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_u32_raw_(uint32_t const d) {
    uint8_t chksum = QS_priv_.chksum;    // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf;  // put in a temporary (register)
    QSCtr head          = QS_priv_.head; // put in a temporary (register)
    QSCtr const end     = QS_priv_.end;  // put in a temporary (register)
    uint32_t x = d;

    QS_priv_.used += 4U; // 4 bytes are about to be added
    for (uint_fast8_t i = 4U; i != 0U; --i) {
        QS_INSERT_ESC_BYTE_((uint8_t)x)
        x >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_obj_raw_(void const * const obj) {
    // output raw obj pointer (w/o format byte),
    // depending on the configured object pointer size
    #if (QS_OBJ_PTR_SIZE == 1U)
        QS_u8_raw_(QS_PTR2UNIT_CAST(uint8_t, obj));
    #elif (QS_OBJ_PTR_SIZE == 2U)
        QS_u16_raw_(QS_PTR2UNIT_CAST(uint16_t, obj));
    #elif (QS_OBJ_PTR_SIZE == 4U)
        QS_u32_raw_(QS_PTR2UNIT_CAST(uint32_t, obj));
    #elif (QS_OBJ_PTR_SIZE == 8U)
        QS_u64_raw_(QS_PTR2UNIT_CAST(uint64_t, obj));
    #else
        QS_u32_raw_(QS_PTR2UNIT_CAST(uint32_t, obj));
    #endif
}

//............................................................................
void QS_str_raw_(char const * const str) {
    uint8_t chksum = QS_priv_.chksum;    // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf;  // put in a temporary (register)
    QSCtr head          = QS_priv_.head; // put in a temporary (register)
    QSCtr const end     = QS_priv_.end;  // put in a temporary (register)
    QSCtr used          = QS_priv_.used; // put in a temporary (register)

    char const *s = str;
    while (*s != '\0') {
        chksum += (uint8_t)*s; // update checksum
        QS_INSERT_BYTE_((uint8_t)*s)  // no escaping for ASCII
        ++s;
        ++used;
    }
    QS_INSERT_BYTE_(0U) // zero-terminate the string
    ++used;

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
    QS_priv_.used   = used;   // save # of used buffer space
}

//............................................................................
void QS_u8_fmt_(
    uint8_t const format,
    uint8_t const d)
{
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr   head    = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)

    QS_priv_.used += 2U; // 2 bytes about to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_I8_FMT or QS_U8_FMT (0x0/0x1) in the lower nibble,
    // which can never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(format) // no escaping
    chksum += (uint8_t)format; // update checksum

    QS_INSERT_ESC_BYTE_(d)

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_u16_fmt_(
    uint8_t const format,
    uint16_t const d)
{
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)
    uint8_t b = (uint8_t)d;

    QS_priv_.used += 3U; // 3 bytes about to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_I16_FMT or QS_U16_FMT (0x2/0x3) in the lower nibble,
    // which can never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(format) // no escaping
    chksum += (uint8_t)format; // update checksum

    QS_INSERT_ESC_BYTE_(b)
    b = (uint8_t)(d >> 8U);
    QS_INSERT_ESC_BYTE_(b)

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_u32_fmt_(
    uint8_t const format,
    uint32_t const d)
{
    uint8_t chksum = QS_priv_.chksum;   // put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)
    uint32_t x = d;

    QS_priv_.used += 5U; // 5 bytes about to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_I32_FMT, QS_U32_FMT, or QS_HEX_FMT (0x4/0x5/0xF)
    // in the lower nibble, which can never be equal to QS_FRAME (0x?E)
    // or QS_ESC (0x?D).
    QS_INSERT_BYTE_(format) // no escaping
    chksum += (uint8_t)format; // update checksum

    // insert 4 bytes...
    for (uint_fast8_t i = 4U; i != 0U; --i) {
        QS_INSERT_ESC_BYTE_((uint8_t)x)
        x >>= 8U;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_str_fmt_(char const * const str) {
    uint8_t chksum = QS_priv_.chksum;
    uint8_t * const buf = QS_priv_.buf; // put in a temporary (register)
    QSCtr head      = QS_priv_.head;    // put in a temporary (register)
    QSCtr const end = QS_priv_.end;     // put in a temporary (register)
    QSCtr used      = QS_priv_.used;    // put in a temporary (register)

    used += 2U; // account for the format byte and the terminating-0

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_STR_FMT (0x8) in the lower nibble,
    // which can never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(QS_STR_FMT)
    chksum += QS_STR_FMT;

    char const *s = str;
    while (*s != '\0') {
        QS_INSERT_BYTE_((uint8_t)*s) // no escaping ASCII
        chksum += (uint8_t)*s; // update checksum
        ++s;
        ++used;
    }
    QS_INSERT_BYTE_(0U) // zero-terminate the string

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
    QS_priv_.used   = used;   // save # of used buffer space
}

//............................................................................
void QS_mem_fmt_(
    uint8_t const * const blk,
    uint8_t const size)
{
    uint8_t chksum      = QS_priv_.chksum; //put in a temporary (register)
    uint8_t * const buf = QS_priv_.buf;    // put in a temporary (register)
    QSCtr head        = QS_priv_.head;     // put in a temporary (register)
    QSCtr const end   = QS_priv_.end;      // put in a temporary (register)
    uint8_t const *pb = blk;

    QS_priv_.used += ((QSCtr)size + 2U); // size+2 bytes to be added

    // NOTE: No escaping needed while inserting the format byte because
    // the format has QS_MEM_FMT (0x9) in the lower nibble,
    // which can never be equal to QS_FRAME (0x?E) or QS_ESC (0x?D).
    QS_INSERT_BYTE_(QS_MEM_FMT)
    chksum += QS_MEM_FMT;

    QS_INSERT_ESC_BYTE_(size)
    // output the 'size' # bytes
    for (uint8_t len = size; len > 0U; --len) {
        QS_INSERT_ESC_BYTE_(*pb)
        ++pb;
    }

    QS_priv_.head   = head;   // save the head
    QS_priv_.chksum = chksum; // save the checksum
}

//............................................................................
void QS_sig_dict_pre_(
    QSignal const sig,
    void const * const obj,
    char const * const name)
{
    // associate signal number (enumeration) with the symbolic name
    // of that enumeration for a given state machine object
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // generate QS_SIG_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_SIG_DICT);
        QS_SIG_PRE(sig);
        QS_OBJ_PRE(obj);
        // skip the leading '&' (address operator) if present
        QS_STR_PRE((*name == '&') ? &name[1] : name);
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}

//............................................................................
void QS_obj_dict_pre_(
    void const * const obj,
    char const * const name)
{
    // associate object address with the symbolic name
    // of that object
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // generate QS_OBJ_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_OBJ_DICT);
        QS_OBJ_PRE(obj);
        // skip the leading '&' (address operator) if present
        QS_STR_PRE((*name == '&') ? &name[1] : name);
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}

//............................................................................
void QS_obj_arr_dict_pre_(
    void const * const obj,
    uint_fast16_t const idx,
    char const * const name)
{
    // associate address of an array element (object) with the
    // symbolic name of the array element (useful in loops that
    // produce dictionaries, such as array[i], which needs to
    // be translated into array[0], array[1], ...

    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // the array index must fit in 3 decimal digits
    Q_REQUIRE_INCRIT(400, idx < 1000U);
    QS_CRIT_EXIT();

    // format idx integer into the idx_str[] char buffer as "xxx\0"
    uint8_t idx_str[4] = { 0U, 0U, 0U, 0U }; // zero-terminate
    uint_fast16_t tmp = idx;
    idx_str[2] = (uint8_t)((uint8_t)'0' + (tmp % 10U));
    tmp /= 10U;
    idx_str[1] =  (uint8_t)((uint8_t)'0' + (tmp % 10U));

    uint8_t i; // index into the idx_str[] string
    if (idx_str[1] == (uint8_t)'0') {
        i = 2U;
    }
    else {
        tmp /= 10U;
        idx_str[0] =  (uint8_t)((uint8_t)'0' + (tmp % 10U));
        if (idx_str[0] == (uint8_t)'0') {
            i = 1U;
        }
        else {
            i = 0U;
        }
    }
    // index into the name[] string
    // initialize to skip the leading '&', if present
    uint8_t j = ((*name == '&') ? 1U : 0U);

    QS_CRIT_ENTRY();

    // generate QS_OBJ_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_OBJ_DICT);
    QS_OBJ_PRE(obj);

    // output chars from name string until '[', inclusive
    while (name[j] != '\0') {
        QS_U8_PRE(name[j]);
        if (name[j] == '[') {
            ++j;
            break;
        }
        ++j;
    }
    // output chars from idx_str until 0U
    while (idx_str[i] != 0U) {
        QS_U8_PRE(idx_str[i]);
        ++i;
    }
    // skip original chars in name[] until ']', exclusive
    while (name[j] != '\0') {
        if (name[j] == ']') {
            break;
        }
        ++j;
    }
    // output the rest of name[] until '\0', exclusive
    while (name[j] != '\0') {
        QS_U8_PRE(name[j]);
        ++j;
    }
    QS_U8_PRE(0U); // zero-terminate the string
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}
//............................................................................
void QS_fun_dict_pre_(
    QSpyFunPtr const fun,
    char const * const name)
{
    // associate function address with the symbolic name
    // of that function
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // generate QS_FUN_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_FUN_DICT);
        QS_FUN_PRE(fun);
        // skip the leading '&' (address operator) if present
        QS_STR_PRE((*name == '&') ? &name[1] : name);
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}
//............................................................................
void QS_usr_dict_pre_(
    enum_t const rec,
    char const * const name)
{
    // associate application-specific (user) record (enumeration)
    // with the symbolic name of that enumeration
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // generate QS_USR_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_USR_DICT);
        QS_U8_PRE(rec);   // user record-ID
        QS_STR_PRE(name); // user record name
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}
//............................................................................
void QS_enum_dict_pre_(
    enum_t const value,
    uint8_t const group,
    char const * const name)
{
    // associate enumeration value in a given group
    // with the symbolic name of that enumeration
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // generate QS_ENUM_DICT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_ENUM_DICT);
        QS_2U8_PRE(value, group);
        QS_STR_PRE(name);
    QS_endRec_();

    QS_CRIT_EXIT();
    QS_onFlush(); // flush the QS output so far
}

//............................................................................
void QS_assertion_pre_(
    char const * const module,
    int_t const id,
    uint16_t const delay)
{
    // NOTE: called in a critical section
    // generate QS_ASSERT_FAIL pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_ASSERT_FAIL);
        QS_TIME_PRE();
        QS_U16_PRE(id);
        QS_STR_PRE((module != (char *)0) ? module : "?");
    QS_endRec_();
    QS_onFlush(); // send all QS data out to the host

    // busy-wait until all QS data makes it over to the host
    uint16_t volatile delay_ctr = delay;
    while (delay_ctr > 0U) {
        delay_ctr -= 1U;
    }
    QS_onCleanup();
}

//............................................................................
void QS_crit_entry_pre_(void) {
    // NOTE: must be called in a critical section
    // generate QS_QF_CRIT_ENTRY pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_QF_CRIT_ENTRY);
        QS_TIME_PRE();
        ++QS_priv_.critNest;
        QS_U8_PRE(QS_priv_.critNest);
    QS_endRec_();
}
//............................................................................
void QS_crit_exit_pre_(void) {
    // NOTE: must be called in a critical section
    // generate QS_QF_CRIT_EXIT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_QF_CRIT_EXIT);
        QS_TIME_PRE();
        QS_U8_PRE(QS_priv_.critNest);
        --QS_priv_.critNest;
    QS_endRec_();
}

//............................................................................
void QS_isr_entry_pre_(uint8_t const isrnest, uint8_t const prio) {
    // NOTE: must be called in a critical section
    // generate QS_QF_ISR_ENTRY pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_QF_ISR_ENTRY);
        QS_TIME_PRE();
        QS_U8_PRE(isrnest);
        QS_U8_PRE(prio);
    QS_endRec_();
}
//............................................................................
void QS_isr_exit_pre_(uint8_t const isrnest, uint8_t const prio) {
    // NOTE: must be called in a critical section
    // generate QS_QF_ISR_EXIT pre-formatted QS record
    QS_beginRec_((uint_fast8_t)QS_QF_ISR_EXIT);
        QS_TIME_PRE();
        QS_U8_PRE(isrnest);
        QS_U8_PRE(prio);
    QS_endRec_();
}

//............................................................................
#define ZERO ((uint8_t)'0')
#define TIME ((uint8_t const *)Q_BUILD_TIME)
#define DATE ((uint8_t const *)Q_BUILD_DATE)

static uint8_t monthNum(void) {
    // convert the 3-letter month to a number 1-12 ...
    uint8_t month = 0U;

    // apply a primitive "hash" of the first 3 characters of the
    // month name (just a sum of the ASCII codes)
    // NOTE: the hash is verified to be unique for all month names.
    switch ((int_t)DATE[0] + (int_t)DATE[1] + (int_t)DATE[2]) {
        case (int_t)'J' + (int_t)'a' + (int_t)'n': month =  1U; break;
        case (int_t)'F' + (int_t)'e' + (int_t)'b': month =  2U; break;
        case (int_t)'M' + (int_t)'a' + (int_t)'r': month =  3U; break;
        case (int_t)'A' + (int_t)'p' + (int_t)'r': month =  4U; break;
        case (int_t)'M' + (int_t)'a' + (int_t)'y': month =  5U; break;
        case (int_t)'J' + (int_t)'u' + (int_t)'n': month =  6U; break;
        case (int_t)'J' + (int_t)'u' + (int_t)'l': month =  7U; break;
        case (int_t)'A' + (int_t)'u' + (int_t)'g': month =  8U; break;
        case (int_t)'S' + (int_t)'e' + (int_t)'p': month =  9U; break;
        case (int_t)'O' + (int_t)'c' + (int_t)'t': month = 10U; break;
        case (int_t)'N' + (int_t)'o' + (int_t)'v': month = 11U; break;
        case (int_t)'D' + (int_t)'e' + (int_t)'c': month = 12U; break;
        default:
            // the DATE[] string contains incorrect month
            // (the month must be formatted per the C Standard specification)
            Q_ERROR_INCRIT(890);
            break;
    }
    return month;
}
//............................................................................
void QS_target_info_pre_(QS_ResetAction const act) {
    // NOTE: called in a critical section

    // build the info byte...
    uint8_t info = 0x02U; // bits[0:1]=0b10
    info |= 0x04U; // bit[2] - qpc
    // bit[3] - qpcpp (not set)
    info |= ((act == QS_TARGET_RESET) ? 0x40U : 0x00U); // bit[6]: reset
    union {
        uint16_t u16;
        uint8_t  u8[2];
    } endian;
    endian.u16 = 0x0102U;
    info |= (endian.u8[0] == 0x01U) ? 0x80U : 0x00U; // bit[7]: big-endian

    QS_beginRec_((uint_fast8_t)QS_TARGET_INFO);
    QS_U8_PRE(info);
    QS_U32_PRE(QP_RELEASE);

    // send the object sizes...
    QS_U8_PRE(2U | (QF_EVENT_SIZ_SIZE << 4U));

#ifdef QF_EQUEUE_CTR_SIZE
    QS_U8_PRE(QF_EQUEUE_CTR_SIZE | (QF_TIMEEVT_CTR_SIZE << 4U));
#else
    QS_U8_PRE(QF_TIMEEVT_CTR_SIZE << 4U);
#endif // QF_EQUEUE_CTR_SIZE

#ifdef QF_MPOOL_CTR_SIZE
    QS_U8_PRE(QF_MPOOL_SIZ_SIZE | (QF_MPOOL_CTR_SIZE << 4U));
#else
    QS_U8_PRE(0U);
#endif // QF_MPOOL_CTR_SIZE

    QS_U8_PRE(QS_OBJ_PTR_SIZE | (QS_FUN_PTR_SIZE << 4U));
    QS_U8_PRE(QS_TIME_SIZE);

    // send the bounds...
    QS_U8_PRE(QF_MAX_ACTIVE);
    QS_U8_PRE(QF_MAX_EPOOL | (QF_MAX_TICK_RATE << 4U));

    // send the build time in three bytes (sec, min, hour)...
    QS_U8_PRE((10U * (uint8_t)(TIME[6] - ZERO))
               + (uint8_t)(TIME[7] - ZERO));
    QS_U8_PRE((10U * (uint8_t)(TIME[3] - ZERO))
               + (uint8_t)(TIME[4] - ZERO));
    if (Q_BUILD_TIME[0] == ' ') {
        QS_U8_PRE(TIME[1] - ZERO);
    }
    else {
        QS_U8_PRE((10U * (uint8_t)(TIME[0] - ZERO))
                  + (uint8_t)(TIME[1] - ZERO));
    }

    // send the build date in three bytes (day, month, year) ...
    if (Q_BUILD_DATE[4] == ' ') {
        QS_U8_PRE(DATE[5] - ZERO);
    }
    else {
        QS_U8_PRE((10U * (uint8_t)(DATE[4] - ZERO))
                   + (uint8_t)(DATE[5] - ZERO));
    }
    QS_U8_PRE(monthNum()); // store the month
    QS_U8_PRE((10U * (uint8_t)(DATE[9] - ZERO))
               + (uint8_t)(DATE[10] - ZERO));
    QS_endRec_();
}

//! @endcond
