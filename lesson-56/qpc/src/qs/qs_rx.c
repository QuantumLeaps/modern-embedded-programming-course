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
#define QP_IMPL       // this is QP implementation
#include "qp_port.h"  // QP port
#include "qp_pkg.h"   // QP package-scope interface
#include "qs_port.h"  // QS port
#include "qs_pkg.h"   // QS package-scope interface
#include "qsafe.h"    // QP Functional Safety (FuSa) Subsystem

//============================================================================
//! @cond INTERNAL

Q_DEFINE_THIS_MODULE("qs_rx")

// enumerated states of the QS-RX state machine
enum {
    ERROR_STATE,
    WAIT4_SEQ,
    WAIT4_REC,
    WAIT4_INFO_FRAME,
    WAIT4_CMD_ID,
    WAIT4_CMD_PARAMS,
    WAIT4_CMD_FRAME,
    WAIT4_RESET_FRAME,
    WAIT4_TICK_RATE,
    WAIT4_TICK_FRAME,
    WAIT4_PPF_OFFS,
    WAIT4_PPF_SIZE,
    WAIT4_PPF_NUM,
    WAIT4_PPF_DATA,
    WAIT4_PPF_FRAME,
    WAIT4_FILTER_LEN,
    WAIT4_FILTER_DATA,
    WAIT4_FILTER_FRAME,
    WAIT4_OBJ_KIND,
    WAIT4_OBJ_ADDR,
    WAIT4_OBJ_FRAME,
    WAIT4_QUERY_KIND,
    WAIT4_QUERY_FRAME,
    WAIT4_EVT_PRIO,
    WAIT4_EVT_SIG,
    WAIT4_EVT_LEN,
    WAIT4_EVT_PAR,
    WAIT4_EVT_FRAME,

#ifdef Q_UTEST
    WAIT4_TEST_SETUP_FRAME,
    WAIT4_TEST_TEARDOWN_FRAME,
    WAIT4_TEST_PROBE_DATA,
    WAIT4_TEST_PROBE_ADDR,
    WAIT4_TEST_PROBE_FRAME,
    WAIT4_TEST_CONTINUE_FRAME,
#endif // Q_UTEST
};

//............................................................................
typedef struct {
    uint32_t param[3];
    uint8_t  idx;
    uint8_t  cmdId;
} CmdVar;

typedef struct {
    uint_fast8_t rate;
} TickVar;

typedef struct {
    uint32_t data;
    uint16_t offs;
    uint8_t  size;
    uint8_t  num;
    uint8_t  idx;
    uint8_t  recId;
} PPFVar; // Peek/Poke/Fill

typedef struct {
    uint8_t data[16];
    uint8_t idx;
    QS_RxRecords recId;
} FltVar;

typedef struct {
    QSObj   addr;
    uint8_t idx;
    uint8_t kind; // see qs.h, enum QS_ObjKind
    QS_RxRecords recId;
} ObjVar;

typedef struct {
    QEvt    *e;
    uint8_t *p;
    QSignal  sig;
    uint16_t len;
    uint8_t  prio;
    uint8_t  idx;
} EvtVar;

// static helper functions...
static void QS_rxParseData_(uint8_t const b);
static void QS_rxParseRec_(uint8_t const b);
static void QS_rxHandleGoodFrame_(uint8_t const state);
static void QS_rxHandleBadFrame_(uint8_t const state);
static void QS_rxReportAck_ (QS_RxRecords const recId);
static void QS_rxReportDone_(QS_RxRecords const recId);
static void QS_rxReportError_(uint8_t const code);
static void QS_queryCurrObj_(enum QS_ObjKind const obj_kind);

static bool QS_rxParseMisc_(uint8_t const b);
static bool QS_rxParseCmd_(uint8_t const b);
static bool QS_rxParseFilter_(uint8_t const b);
static bool QS_rxParseObj_(uint8_t const b);
static bool QS_rxParsePPF_(uint8_t const b);
static bool QS_rxParseEvt_(uint8_t const b);
#ifdef Q_UTEST
static bool QS_rxParseQUTest_(uint8_t const b);
#endif
static void QS_rxPoke_(void);

static void QS_rxHandleGoodInfo_(void);
static void QS_rxHandleGoodCmd_(void);
static void QS_rxHandleGoodTick_(void);
static void QS_rxHandleGoodPPF_(void);
static void QS_rxHandleGoodFilter_(void);
static void QS_rxHandleGoodObj_(void);
static void QS_rxHandleGoodEvt_(void);
#ifdef Q_UTEST
static void QS_rxHandleGoodTestSetup_(void);
static void QS_rxHandleGoodTestProbe_(void);
#endif

typedef struct {
    void const *obj;
    void const *ptr;
    QStateHandler fun;
    uint8_t id;
    uint32_t val1;
    uint32_t val2;
    uint32_t val3;
    uint32_t val4;
} QueryData;

static void QS_queryCurrObjOut_(enum QS_ObjKind const obj_kind,
    QueryData const * const data);
static bool QS_queryCurrAO_(QueryData * const data);
static bool QS_queryCurrEP_(QueryData * const data);

#define QS_RX_TRAN_(target_) (QS_rxPriv_->state = (uint8_t)(target_))

//============================================================================
typedef struct QS_RxAttr {
    void    *currObj[(enum_t)QS_OBJ_SM_AO];
    uint8_t  currId[4];
    uint8_t *buf;
    QSCtr    end;
    QSCtr    head;
    QSCtr    tail;
    uint8_t  state;
    uint8_t  esc;
    uint8_t  seq;
    uint8_t  chksum;
    union Variant {
        CmdVar   cmd;
        TickVar  tick;
        PPFVar   ppf;
        FltVar   flt;
        ObjVar   obj;
        EvtVar   evt;
#ifdef Q_UTEST
        struct QS_TProbe tp;
#endif // Q_UTEST
    } var;
} QS_RxAttr;

//! @endcond
//============================================================================

//! @static @private @memberof QS
static QS_RxAttr l_rxPriv_;
struct QS_RxAttr * const QS_rxPriv_ = &l_rxPriv_;

//............................................................................
//! @static @public @memberof QS
void QS_rxInitBuf(
    uint8_t * const sto,
    uint16_t const stoSize)
{
    QS_rxPriv_->buf = &sto[0];
    QS_rxPriv_->end = (QSCtr)stoSize;

    // generate object dictionary for the "QS_RX" source of trace records
    QS_beginRec_((uint_fast8_t)QS_OBJ_DICT);
        QS_OBJ_PRE(&QS_rxPriv_);
        QS_STR_PRE("QS_RX");
    QS_endRec_();
    // no QS_REC_DONE(), because QS is not running yet

#ifdef Q_UTEST
    QS_tstPriv_.inTestLoop = false;
#endif // Q_UTEST

    QS_RX_TRAN_(WAIT4_SEQ); // initial transition in the QS-RX state machine
}

//............................................................................
//! @static @public @memberof QS
void QS_rxParse(void) {
    QSCtr tail = QS_rxPriv_->tail; // move member into temporary
    while (QS_rxPriv_->head != tail) { // QS-RX buffer NOT empty?
        uint8_t b = QS_rxPriv_->buf[tail]; // get byte at the tail
        ++tail;
        if (tail == QS_rxPriv_->end) { // tail at the end?
            tail = 0U; // wrap tail around
        }
        QS_rxPriv_->tail = tail; // update the original

        if (QS_rxPriv_->esc != 0U) { // escaped byte arrived?
            QS_rxPriv_->esc = 0U; // don't escape the next byte
            b ^= QS_ESC_XOR; // escape the byte

            QS_rxPriv_->chksum += b; // update after escaping
            QS_rxParseData_(b); // parse one byte in the QS-RX state machine
        }
        else if (b == QS_ESC) { // escape byte?
            QS_rxPriv_->esc = 1U; // remember to escape *next* byte
        }
        else if (b == QS_FRAME) {
            // get ready for the next frame
            b = QS_rxPriv_->state; // save the current state in b
            QS_rxPriv_->esc = 0U; // don't escape the next byte
            QS_RX_TRAN_(WAIT4_SEQ);

            // validate the complete frame
            if (QS_rxPriv_->chksum == QS_GOOD_CHKSUM) {
                QS_rxPriv_->chksum = 0U;
                QS_rxHandleGoodFrame_(b);
            }
            else { // bad checksum
                QS_rxPriv_->chksum = 0U;
                QS_rxReportError_(0x41U);
                QS_rxHandleBadFrame_(b);
            }
        }
        else { // none of the special bytes
            QS_rxPriv_->chksum += b;
            QS_rxParseData_(b); // parse one byte in the state machine
        }
    }
}
//............................................................................
//! @static @public @memberof QS
void QS_rxParseBuf(uint16_t const len) {
    QS_rxPriv_->tail = 0U; // start parsing from the beginning of the buffer
    QS_rxPriv_->head = len; // parse only the requested number of bytes
    QS_rxParse(); // parse the received bytes
}
//............................................................................
//! @static @public @memberof QS
bool QS_rxPut(uint8_t const b) {
    // NOTE: does not need critical section
    // But requires system-level memory access (QF_MEM_SYS()).

    QSCtr head = QS_rxPriv_->head + 1U;
    if (head == QS_rxPriv_->end) { // end reached?
        head = 0U; // wrap around
    }
    bool success = false; // assume buffer full
    if (head != QS_rxPriv_->tail) { // buffer NOT full?
        QS_rxPriv_->buf[QS_rxPriv_->head] = b;
        QS_rxPriv_->head = head; // update the head to a *valid* index
        success = true; // byte placed in the buffer
    }
    return success;
}
//............................................................................
//! @static @public @memberof QS
uint16_t QS_rxGetFree(void) {
    // NOTE: Must be called IN critical section.
    // Also requires system-level memory access (QF_MEM_SYS()).

    QSCtr const head = QS_rxPriv_->head;
    uint16_t nFree;
    if (head == QS_rxPriv_->tail) { // buffer empty?
        nFree = (uint16_t)(QS_rxPriv_->end - 1U);
    }
    else if (head < QS_rxPriv_->tail) { // head "behind" the tail?
        nFree = (uint16_t)(QS_rxPriv_->tail - (head + 1U));
    }
    else { // must be head "after" the tail
        nFree = (uint16_t)((head + 1U) - QS_rxPriv_->tail);
        nFree = (uint16_t)(QS_rxPriv_->end - nFree);
    }
    return nFree;
}

//............................................................................
//! @static @public @memberof QS
bool QS_setCurrObj(enum QS_ObjKind const obj_kind, void * const obj_ptr) {
    bool status = true;
    switch (obj_kind) {
        case QS_OBJ_SM: // intentionally fall through
        case QS_OBJ_AO: // intentionally fall through
        case QS_OBJ_MP: // intentionally fall through
        case QS_OBJ_EQ: // intentionally fall through
        case QS_OBJ_TE: // intentionally fall through
        case QS_OBJ_AP: QS_rxPriv_->currObj[obj_kind] = obj_ptr; break;
        case QS_OBJ_SM_AO:
            QS_rxPriv_->currObj[QS_OBJ_SM] = obj_ptr;
            QS_rxPriv_->currObj[QS_OBJ_AO] = obj_ptr;
            QS_rxPriv_->currId[0] = 0U;
            break;
        default: status = false; break; // wrong obj_kind
    }
    return status;
}
//............................................................................
//! @static @public @memberof QS
void *QS_getCurrObj(enum QS_ObjKind const obj_kind) {
    void *obj_ptr = (void *)0;
    switch (obj_kind) {
        case QS_OBJ_SM: // intentionally fall through
        case QS_OBJ_AO: // intentionally fall through
        case QS_OBJ_MP: // intentionally fall through
        case QS_OBJ_EQ: // intentionally fall through
        case QS_OBJ_TE: // intentionally fall through
        case QS_OBJ_AP:    obj_ptr = QS_rxPriv_->currObj[obj_kind];  break;
        case QS_OBJ_SM_AO: obj_ptr = QS_rxPriv_->currObj[QS_OBJ_SM]; break;
        default: // empty
            break;
    }
    return obj_ptr;
}
//............................................................................
//! @static @public @memberof QS
bool QS_setCurrId(enum QS_ObjKind const obj_kind, uint8_t const obj_id) {
    bool status = true; // assume successful setting
    switch (obj_kind) {
        case QS_OBJ_AO:
            QS_rxPriv_->currId[0] = obj_id;
            // invalidate the current-obj pointer for the AO
            QS_rxPriv_->currObj[QS_OBJ_AO] = (void *)0;
            break;
        case QS_OBJ_EP:
            QS_rxPriv_->currId[1] = obj_id;
            break;
        default: // no other object kinds support currentId
            status = false; // wrong obj_kind
            break;
    }
    return status;
}
//............................................................................
//! @static @public @memberof QS
uint8_t QS_getCurrId(enum QS_ObjKind const obj_kind) {
    uint8_t obj_id = 0U; // invalid object Id to return
    switch (obj_kind) {
        case QS_OBJ_AO: obj_id = QS_rxPriv_->currId[0]; break;
        case QS_OBJ_EP: obj_id = QS_rxPriv_->currId[1]; break;
        default: // no other object kinds support currentId
            break;
    }
    return obj_id;
}

//============================================================================
//! @cond INTERNAL

static void QS_rxParseRec_(uint8_t const b) {
    switch ((QS_RxRecords)b) {
        case QS_RX_INFO: // target info request
            QS_RX_TRAN_(WAIT4_INFO_FRAME);
            break;
        case QS_RX_COMMAND:
            QS_RX_TRAN_(WAIT4_CMD_ID);
            break;
        case QS_RX_RESET:
            QS_RX_TRAN_(WAIT4_RESET_FRAME);
            break;
        case QS_RX_TICK:
            QS_RX_TRAN_(WAIT4_TICK_RATE);
            break;
        case QS_RX_PEEK: // intentionally fall-through
        case QS_RX_POKE: // intentionally fall-through
        case QS_RX_FILL:
            // peek/poke/fill are similar and are be handled
            // by the same part of the QS-RX state machine
            QS_rxPriv_->var.ppf.recId = b; // store (Peek/Poke/Fill)

            // peek/poke/fill require the current AP-obj to be provided
            if (QS_rxPriv_->currObj[QS_OBJ_AP] != (void *)0) {
                QS_rxPriv_->var.ppf.offs = 0U;
                QS_rxPriv_->var.ppf.idx  = 0U;
                QS_RX_TRAN_(WAIT4_PPF_OFFS);
            }
            else { // current AP-obj not provided -- abort
                QS_rxReportError_(b);
                QS_RX_TRAN_(ERROR_STATE);
            }
            break;
        case QS_RX_GLB_FILTER: // intentionally fall-through
        case QS_RX_LOC_FILTER:
            // global and local filters are similar and are be handled
            // by the same part of the QS-RX state machine
            QS_rxPriv_->var.flt.recId = (QS_RxRecords)b;
            QS_RX_TRAN_(WAIT4_FILTER_LEN);
            break;
        case QS_RX_AO_FILTER: // intentionally fall-through
        case QS_RX_CURR_OBJ:
            // the legacy AO-filter and current obj are similar and are
            // be handled by the same part of the QS-RX state machine
            QS_rxPriv_->var.obj.recId = (QS_RxRecords)b;
            QS_RX_TRAN_(WAIT4_OBJ_KIND);
            break;
        case QS_RX_QUERY_CURR:
            QS_rxPriv_->var.obj.recId = QS_RX_QUERY_CURR;
            QS_RX_TRAN_(WAIT4_QUERY_KIND);
            break;
        case QS_RX_EVENT:
            QS_RX_TRAN_(WAIT4_EVT_PRIO);
            break;

#ifdef Q_UTEST
        case QS_RX_TEST_SETUP:
            QS_RX_TRAN_(WAIT4_TEST_SETUP_FRAME);
            break;
        case QS_RX_TEST_TEARDOWN:
            QS_RX_TRAN_(WAIT4_TEST_TEARDOWN_FRAME);
            break;
        case QS_RX_TEST_CONTINUE:
            QS_RX_TRAN_(WAIT4_TEST_CONTINUE_FRAME);
            break;
        case QS_RX_TEST_PROBE:
            if (QS_tstPriv_.tpNum
                < (uint8_t)(sizeof(QS_tstPriv_.tpBuf)
                            / sizeof(QS_tstPriv_.tpBuf[0])))
            {
                QS_rxPriv_->var.tp.data = 0U;
                QS_rxPriv_->var.tp.idx  = 0U;
                QS_RX_TRAN_(WAIT4_TEST_PROBE_DATA);
            }
            else { // number of Test-Probes exceeded
                QS_rxReportError_(b);
                QS_RX_TRAN_(ERROR_STATE);
            }
            break;
#endif // Q_UTEST

        default:
            QS_rxReportError_(0x43U);
            QS_RX_TRAN_(ERROR_STATE);
            break;
    }
}
//............................................................................
static void QS_rxParseData_(uint8_t const b) {
    // the following subroutines parse one byte
    // according to the current state of the QS-RX state machine
    if (QS_rxParseMisc_(b)) {
        // states: WAIT4_SEQ, WAIT4_REC, WAIT4_TICK_RATE, WAIT4_QUERY_KIND
    }
    else if (QS_rxParseCmd_(b)) {
        // states: WAIT4_CMD_ID, WAIT4_CMD_PARAMS, WAIT4_CMD_FRAME
    }
    else if (QS_rxParseFilter_(b)) {
        // states: WAIT4_FILTER_LEN, WAIT4_FILTER_DATA, WAIT4_FILTER_FRAME
    }
    else if (QS_rxParseObj_(b)) {
        // states: WAIT4_OBJ_KIND, WAIT4_OBJ_ADDR, WAIT4_OBJ_FRAME
    }
    else if (QS_rxParsePPF_(b)) { // peek/poke/fill
        // states: WAIT4_PPF_OFFS, WAIT4_PPF_SIZE, WAIT4_PPF_NUM,
        // WAIT4_PPF_DATA, WAIT4_PPF_FRAME
    }
    else if (QS_rxParseEvt_(b)) {
        // states: WAIT4_EVT_PRIO, WAIT4_EVT_SIG, WAIT4_EVT_LEN,
        // WAIT4_EVT_PAR, WAIT4_EVT_FRAME
    }
#ifdef Q_UTEST
    else if (QS_rxParseQUTest_(b)) {
        // states:
    }
#endif
    else { // state machine in an unknown state
        QS_rxReportError_(0x45U);
        QS_RX_TRAN_(ERROR_STATE); // transition to ERROR
    }
}
//............................................................................
static bool QS_rxParseMisc_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_SEQ: {
            ++QS_rxPriv_->seq;
            if (QS_rxPriv_->seq != b) {
                QS_rxReportError_(0x42U);
                QS_rxPriv_->seq = b; // update the sequence
            }
            QS_RX_TRAN_(WAIT4_REC);
            break;
        }
        case WAIT4_REC: {
            QS_rxParseRec_(b);
            break;
        }
        case WAIT4_TICK_RATE: {
            QS_rxPriv_->var.tick.rate = (uint_fast8_t)b;
            QS_RX_TRAN_(WAIT4_TICK_FRAME);
            break;
        }
        case WAIT4_QUERY_KIND: {
            QS_rxPriv_->var.obj.kind = b;
            QS_RX_TRAN_(WAIT4_QUERY_FRAME);
            break;
        }
        case WAIT4_INFO_FRAME:  // intentionally fall through
        case WAIT4_RESET_FRAME: // intentionally fall through
        case WAIT4_QUERY_FRAME: // intentionally fall through
        case WAIT4_TICK_FRAME:  // intentionally fall through
        case ERROR_STATE: {
            // keep ignoring the data until a good frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static bool QS_rxParseCmd_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_CMD_ID: {
            QS_rxPriv_->var.cmd.cmdId    = b;
            QS_rxPriv_->var.cmd.idx      = 0U; // counts *bits*
            QS_rxPriv_->var.cmd.param[0] = 0U;
            QS_rxPriv_->var.cmd.param[1] = 0U;
            QS_rxPriv_->var.cmd.param[2] = 0U;
            QS_RX_TRAN_(WAIT4_CMD_PARAMS);
            break;
        }
        case WAIT4_CMD_PARAMS: {
            QS_rxPriv_->var.cmd.param[QS_rxPriv_->var.cmd.idx >> 5U] |=
                ((uint32_t)b << (QS_rxPriv_->var.cmd.idx & 0x1FU));
            QS_rxPriv_->var.cmd.idx += 8U; // 8 more bits
            // is the expected number of bits for 3 4-byte params reached?
            if (QS_rxPriv_->var.cmd.idx == (8U * 3U * 4U)) {
                QS_RX_TRAN_(WAIT4_CMD_FRAME);
            }
            break;
        }
        case WAIT4_CMD_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static bool QS_rxParseFilter_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_FILTER_LEN: {
            // the filter length (# bytes) must match the expected size
            if (b == sizeof(QS_rxPriv_->var.flt.data)) {
                QS_rxPriv_->var.flt.idx = 0U;
                QS_RX_TRAN_(WAIT4_FILTER_DATA);
            }
            else { // unexpected filter length
                QS_rxReportError_((uint8_t)QS_rxPriv_->var.flt.recId);
                QS_RX_TRAN_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_FILTER_DATA: {
            // receive the filter data one byte at a time
            QS_rxPriv_->var.flt.data[QS_rxPriv_->var.flt.idx] = b;
            ++QS_rxPriv_->var.flt.idx;
            // is the expected # bytes reached?
            if (QS_rxPriv_->var.flt.idx == sizeof(QS_rxPriv_->var.flt.data)) {
                QS_RX_TRAN_(WAIT4_FILTER_FRAME);
            }
            break;
        }
        case WAIT4_FILTER_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static bool QS_rxParseObj_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_OBJ_KIND: {
            QS_rxPriv_->var.obj.kind = b;
            QS_rxPriv_->var.obj.addr = 0U;
            QS_rxPriv_->var.obj.idx  = 0U; // counts *bits* in the obj address
            QS_RX_TRAN_(WAIT4_OBJ_ADDR);
            break;
        }
        case WAIT4_OBJ_ADDR: {
            QS_rxPriv_->var.obj.addr |= ((QSObj)b << QS_rxPriv_->var.obj.idx);
            QS_rxPriv_->var.obj.idx += 8U; // 8 more bits of address
            // do we have the complete obj address?
            if (QS_rxPriv_->var.obj.idx == (uint8_t)(8U * QS_OBJ_PTR_SIZE)) {
                QS_RX_TRAN_(WAIT4_OBJ_FRAME);
            }
            break;
        }
        case WAIT4_OBJ_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static bool QS_rxParsePPF_(uint8_t const b) {
    // PPF is for peek/poke/fill
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_PPF_OFFS: {
            if (QS_rxPriv_->var.ppf.idx == 0U) { // need 1st byte of offset?
                QS_rxPriv_->var.ppf.offs = (uint16_t)b;
                QS_rxPriv_->var.ppf.idx  = 1U;
            }
            else { // already have the 1st byte
                QS_rxPriv_->var.ppf.offs |=
                    (uint16_t)((uint16_t)b << 8U);
                QS_RX_TRAN_(WAIT4_PPF_SIZE);
            }
            break;
        }
        case WAIT4_PPF_SIZE: {
            // any of the supported sizes?
            if ((b == 1U) || (b == 2U) || (b == 4U)) {
                QS_rxPriv_->var.ppf.size = b;
                QS_RX_TRAN_(WAIT4_PPF_NUM);
            }
            else {
                QS_rxReportError_(QS_rxPriv_->var.ppf.recId);
                QS_RX_TRAN_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_PPF_NUM: {
            if (b > 0U) { // valid number of elements?
                QS_rxPriv_->var.ppf.num  = b;
                QS_rxPriv_->var.ppf.data = 0U;
                QS_rxPriv_->var.ppf.idx  = 0U;
                QS_RX_TRAN_(
                    (QS_rxPriv_->var.ppf.recId == (uint8_t)QS_RX_PEEK)
                        ? WAIT4_PPF_FRAME
                        : WAIT4_PPF_DATA);
            }
            else { // invalid number of elements
                QS_rxReportError_(QS_rxPriv_->var.ppf.recId);
                QS_RX_TRAN_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_PPF_DATA: {
            QS_rxPriv_->var.ppf.data |=
                ((uint32_t)b << QS_rxPriv_->var.ppf.idx);
            QS_rxPriv_->var.ppf.idx += 8U; // idx counts BITS

            // did we receive all data bits corresponding to the
            // size of the element?
            if ((uint8_t)(QS_rxPriv_->var.ppf.idx >> 3U)
                == QS_rxPriv_->var.ppf.size)
            {
                // is this for QS_RX_POKE command?
                if (QS_rxPriv_->var.ppf.recId == (uint8_t)QS_RX_POKE) {
                    QS_rxPoke_(); // poke the received element

                    // did we receive the last data of the promised number?
                    --QS_rxPriv_->var.ppf.num;
                    if (QS_rxPriv_->var.ppf.num == 0U) {
                        QS_RX_TRAN_(WAIT4_PPF_FRAME);
                    }
                }
                else { // must be QS_RX_FILL
                    QS_RX_TRAN_(WAIT4_PPF_FRAME);
                }
            }
            break;
        }
        case WAIT4_PPF_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static bool QS_rxParseEvt_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_EVT_PRIO: {
            // "event prio" is used as the AO prio. for posting the event
            // or special cases for: event publishing, dispatching,
            // and initialzing (initial event).
            QS_rxPriv_->var.evt.prio = b;
             QS_rxPriv_->var.evt.sig = 0U;
            QS_rxPriv_->var.evt.idx  = 0U; // counts *bits* for evt. signal
            QS_RX_TRAN_(WAIT4_EVT_SIG);
            break;
        }
        case WAIT4_EVT_SIG: {
            QS_rxPriv_->var.evt.sig |=
                (QSignal)((uint32_t)b << QS_rxPriv_->var.evt.idx);
            QS_rxPriv_->var.evt.idx += 8U;
            if (QS_rxPriv_->var.evt.idx == 16U) { // 16-bit signal received?
                QS_rxPriv_->var.evt.len = 0U; // parameter length [bytes]
                QS_rxPriv_->var.evt.idx = 0U; // count *bits* in len
                QS_RX_TRAN_(WAIT4_EVT_LEN);
            }
            break;
        }
        case WAIT4_EVT_LEN: {
            QS_rxPriv_->var.evt.len |=
                (uint16_t)((uint32_t)b << QS_rxPriv_->var.evt.idx);
            QS_rxPriv_->var.evt.idx += 8U;
            if (QS_rxPriv_->var.evt.idx == 16U) { // 16-bit len received?
                if ((QS_rxPriv_->var.evt.len + sizeof(QEvt))
                    <= QF_poolGetMaxBlockSize())
                { // would that event fit in the Event Pools?
                    // report Ack before generating any other QS records
                    QS_rxReportAck_(QS_RX_EVENT);

                    // dynamically allocate the event
                    QS_rxPriv_->var.evt.e = QF_newX_(
                        (uint_fast16_t)(QS_rxPriv_->var.evt.len + sizeof(QEvt)),
                        0U, // margin
                        (enum_t)QS_rxPriv_->var.evt.sig);
                    if (QS_rxPriv_->var.evt.e != (QEvt *)0) { // allocated?
                        // point evt.p to the event *parameters*
                        QS_rxPriv_->var.evt.p =
                            (uint8_t*)QS_rxPriv_->var.evt.e;
                        QS_rxPriv_->var.evt.p =
                            &QS_rxPriv_->var.evt.p[sizeof(QEvt)];

                        if (QS_rxPriv_->var.evt.len > 0U) { // any params?
                            QS_RX_TRAN_(WAIT4_EVT_PAR);
                        }
                        else {
                            QS_RX_TRAN_(WAIT4_EVT_FRAME);
                        }
                    }
                    else { // event allocation failed
                        QS_rxReportError_((uint8_t)QS_RX_EVENT);
                        QS_RX_TRAN_(ERROR_STATE);
                    }
                }
                else { // event won't fit in any Event Pools
                    QS_rxReportError_((uint8_t)QS_RX_EVENT);
                    QS_RX_TRAN_(ERROR_STATE);
                }
            }
            break;
        }
        case WAIT4_EVT_PAR: { // event parameters
            // collect event parameters one byte at a time
            *QS_rxPriv_->var.evt.p = b;
            ++QS_rxPriv_->var.evt.p;
            --QS_rxPriv_->var.evt.len;
            if (QS_rxPriv_->var.evt.len == 0U) {
                QS_RX_TRAN_(WAIT4_EVT_FRAME);
            }
            break;
        }
        case WAIT4_EVT_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default: // different QS-RX state
            handled = false; // not handled
            break;
    }
    return handled;
}
//............................................................................
static void QS_rxPoke_(void) {
    // poke the memory according to the received data
    uint8_t *ptr = (uint8_t *)QS_rxPriv_->currObj[QS_OBJ_AP]; // data to poke
    ptr = &ptr[QS_rxPriv_->var.ppf.offs]; // shift by the offset
    switch (QS_rxPriv_->var.ppf.size) {
        case 1U: // 1-byte (uint8_t) poke
            *ptr = (uint8_t)QS_rxPriv_->var.ppf.data;
            break;
        case 2U: // 2-byte (uint16_t) poke
            *(uint16_t *)ptr = (uint16_t)QS_rxPriv_->var.ppf.data;
            break;
        case 4U: // 4-byte (uint32_t) poke
            *(uint32_t *)ptr = QS_rxPriv_->var.ppf.data;
            break;
        default: // unsupported size
            Q_ERROR_INCRIT(900);
            break;
    }
    // get ready for the next poke
    QS_rxPriv_->var.ppf.data = 0U;
    QS_rxPriv_->var.ppf.idx  = 0U;
    QS_rxPriv_->var.ppf.offs += (uint16_t)QS_rxPriv_->var.ppf.size;
}
//............................................................................
#ifdef Q_UTEST
static bool QS_rxParseQUTest_(uint8_t const b) {
    bool handled = true; // assume that the byte will be handled
    switch (QS_rxPriv_->state) {
        case WAIT4_TEST_SETUP_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_PROBE_DATA: {
            QS_rxPriv_->var.tp.data |= ((uint32_t)b << QS_rxPriv_->var.tp.idx);
            QS_rxPriv_->var.tp.idx += 8U;
            if (QS_rxPriv_->var.tp.idx == (uint8_t)(8U * sizeof(uint32_t))) {
                QS_rxPriv_->var.tp.addr = 0U;
                QS_rxPriv_->var.tp.idx  = 0U;
                QS_RX_TRAN_(WAIT4_TEST_PROBE_ADDR);
            }
            break;
        }
        case WAIT4_TEST_PROBE_ADDR: {
            QS_rxPriv_->var.tp.addr |= ((QSFun)b << QS_rxPriv_->var.tp.idx);
            QS_rxPriv_->var.tp.idx += 8U;
            if (QS_rxPriv_->var.tp.idx == (uint8_t)(8U * QS_FUN_PTR_SIZE)) {
                QS_RX_TRAN_(WAIT4_TEST_PROBE_FRAME);
            }
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        default:
            handled = false;
            break;
    }
    return handled;
}
#endif

//----------------------------------------------------------------------------
static void QS_rxHandleGoodFrame_(uint8_t const state) {
    switch (state) {
        case WAIT4_INFO_FRAME: {
            QS_rxHandleGoodInfo_();
            break;
        }
        case WAIT4_RESET_FRAME: {
            // no need to report Ack or Done, because Target resets
            QS_onReset(); // reset the Target
            break;
        }
        case WAIT4_CMD_PARAMS: // intentionally fall-through
        case WAIT4_CMD_FRAME: {
            QS_rxHandleGoodCmd_();
            break;
        }
        case WAIT4_TICK_FRAME: { // generate clock tick(s)
            QS_rxHandleGoodTick_();
            break;
        }
        case WAIT4_PPF_FRAME: { // peek/poke/fill
            QS_rxHandleGoodPPF_();
            break;
        }
        case WAIT4_PPF_DATA: { // incorrect peek/poke/fill data
            // received less than expected poke data items
            QS_rxReportError_((uint8_t)QS_RX_POKE);
            break;
        }
        case WAIT4_FILTER_FRAME: { // global/local QS filters
            QS_rxHandleGoodFilter_();
            break;
        }
        case WAIT4_OBJ_FRAME: { // current object
            QS_rxHandleGoodObj_();
            break;
        }
        case WAIT4_QUERY_FRAME: { // query object
            QS_queryCurrObj_((enum QS_ObjKind)QS_rxPriv_->var.obj.kind);
            break;
        }
        case WAIT4_EVT_FRAME: { // event dispatch/post/publish
            QS_rxHandleGoodEvt_();
            break;
        }

#ifdef Q_UTEST
        case WAIT4_TEST_SETUP_FRAME: {
            QS_rxHandleGoodTestSetup_();
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            QS_rxReportAck_((int8_t)QS_RX_TEST_TEARDOWN);
            QS_onTestTeardown(); // application-specific test teardown
            // no need to report Done
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            QS_rxReportAck_((int8_t)QS_RX_TEST_CONTINUE);
            QS_tstPriv_.inTestLoop = false; // exit the QUTest loop
            // no need to report Done
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            QS_rxHandleGoodTestProbe_();
            break;
        }
#endif // Q_UTEST

        case ERROR_STATE: {
            // keep ignoring all bytes until new frame
            break;
        }
        default:
            QS_rxReportError_(0x47U);
            break;
    }
}
//............................................................................
static void QS_rxHandleGoodInfo_(void) {
    // no need to report Ack or Done
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    // generate target info trace record
    QS_target_info_pre_(QS_TARGET_NO_RESET);
    QS_CRIT_EXIT();
}
//............................................................................
static void QS_rxHandleGoodTick_(void) {
    QS_rxReportAck_(QS_RX_TICK);
#if (QF_MAX_TICK_RATE > 0U)
#ifdef Q_UTEST
    QTimeEvt_tick1_((uint_fast8_t)QS_rxPriv_->var.tick.rate, &QS_rxPriv_);
#if (Q_UTEST != 0)
    QS_processTestEvts_(); // process all events produced
#endif // (Q_UTEST != 0)
#else
    QTimeEvt_tick_((uint_fast8_t)QS_rxPriv_->var.tick.rate, &QS_rxPriv_);
#endif // Q_UTEST
    QS_rxReportDone_(QS_RX_TICK);
#else
    QS_rxReportError_(QS_RX_TICK);
#endif // (QF_MAX_TICK_RATE > 0U)
}
//............................................................................
static void QS_rxHandleGoodCmd_(void) {
    QS_rxReportAck_(QS_RX_COMMAND);
    QS_onCommand(QS_rxPriv_->var.cmd.cmdId,
        QS_rxPriv_->var.cmd.param[0],
        QS_rxPriv_->var.cmd.param[1],
        QS_rxPriv_->var.cmd.param[2]);
#ifdef Q_UTEST
#if (Q_UTEST != 0)
    QS_processTestEvts_(); // process all events produced
#endif // (Q_UTEST != 0)
#endif // Q_UTEST
    QS_rxReportDone_(QS_RX_COMMAND);
}
//............................................................................
static void QS_rxHandleGoodPPF_(void) {
    if (QS_rxPriv_->var.ppf.recId == (uint8_t)QS_RX_PEEK) { // PEEK?
        // no need to report Ack or Done
        QS_CRIT_STAT
        QS_CRIT_ENTRY();

        // generate the QS_PEEK_DATA pre-formatted QS trace record
        QS_beginRec_((uint_fast8_t)QS_PEEK_DATA);
            uint8_t const *ptr =
                (uint8_t const *)QS_rxPriv_->currObj[QS_OBJ_AP];
            ptr = &ptr[QS_rxPriv_->var.ppf.offs]; // shift by the offset
            QS_TIME_PRE(); // timestamp
            QS_U16_PRE(QS_rxPriv_->var.ppf.offs); // data offset
            QS_U8_PRE(QS_rxPriv_->var.ppf.size);  // data size
            QS_U8_PRE(QS_rxPriv_->var.ppf.num);   // # data items
            for (uint_fast8_t i = 0U; i < QS_rxPriv_->var.ppf.num; ++i) {
                switch (QS_rxPriv_->var.ppf.size) {
                    case 1: // 1-byte (uint8_t) element
                        QS_U8_PRE(ptr[i]);
                        break;
                    case 2: // 2-byte (uint16_t) element
                        QS_U16_PRE(((uint16_t const *)ptr)[i]);
                        break;
                    case 4: // 4-byte (uint32_t) element
                        QS_U32_PRE(((uint32_t const *)ptr)[i]);
                        break;
                    default: // unsupported size
                        // intentionally empty
                        break;
                }
            }
        QS_endRec_();
        QS_CRIT_EXIT();

        QS_REC_DONE(); // invoke user callback (if defined)
    }
    else if (QS_rxPriv_->var.ppf.recId == (uint8_t)QS_RX_FILL) { // FILL?
        QS_rxReportAck_(QS_RX_FILL);
        uint8_t* ptr = (uint8_t*)QS_rxPriv_->currObj[QS_OBJ_AP];
        ptr = &ptr[QS_rxPriv_->var.ppf.offs];
        for (uint_fast8_t i = 0U; i < QS_rxPriv_->var.ppf.num; ++i) {
            switch (QS_rxPriv_->var.ppf.size) {
            case 1: // 1-byte (uint8_t) element
                ptr[i] = (uint8_t)QS_rxPriv_->var.ppf.data;
                break;
            case 2: // 2-byte (uint16_t) element
                ((uint16_t*)ptr)[i]
                    = (uint16_t)QS_rxPriv_->var.ppf.data;
                break;
            case 4: // 4-byte (uint32_t) element
                ((uint32_t*)ptr)[i] = QS_rxPriv_->var.ppf.data;
                break;
            default: // unsupported size
                // intentionally empty
                break;
            }
        }
    }
    else if (QS_rxPriv_->var.ppf.recId == (uint8_t)QS_RX_POKE) { // POKE?
        QS_rxReportAck_(QS_RX_POKE);
        // no need to report done
    }
    else { // wrong command
        QS_rxReportError_(QS_rxPriv_->var.ppf.recId);
    }
}
//............................................................................
static void QS_rxHandleGoodFilter_(void) {
    QS_rxReportAck_(QS_rxPriv_->var.flt.recId);

    // apply the received filters
    if (QS_rxPriv_->var.flt.recId == QS_RX_GLB_FILTER) {
        for (uint_fast8_t i = 0U; i < Q_DIM(QS_filt_.glb); ++i) {
            // index into flt.data[]
            uint_fast8_t const j = (uint_fast8_t)(i << 2U);
            QS_filt_.glb[i] =
                ((uint32_t)QS_rxPriv_->var.flt.data[j]
                 | ((uint32_t)QS_rxPriv_->var.flt.data[j + 1U] << 8U)
                 | ((uint32_t)QS_rxPriv_->var.flt.data[j + 2U] << 16U)
                 | ((uint32_t)QS_rxPriv_->var.flt.data[j + 3U] << 24U));
        }
        // ensure that the following records are always on
        QS_filt_.glb[0] |= 0x00000001U;
        QS_filt_.glb[1] |= 0xFC000000U;
        QS_filt_.glb[2] |= 0x0000007FU;

        // ensure that the last 3 records (0x7D, 0x7E, 0x7F) are always off
        QS_filt_.glb[3] &= 0x1FFFFFFFU;
    }
    else if (QS_rxPriv_->var.flt.recId == QS_RX_LOC_FILTER) {
        for (uint_fast8_t i = 0U; i < Q_DIM(QS_filt_.loc); ++i) {
            // index into flt.data[]
            uint_fast8_t const j = (uint_fast8_t)(i << 2U);
            QS_filt_.loc[i] =
                ((uint32_t)QS_rxPriv_->var.flt.data[j]
                    | ((uint32_t)QS_rxPriv_->var.flt.data[j + 1U] << 8U)
                    | ((uint32_t)QS_rxPriv_->var.flt.data[j + 2U] << 16U)
                    | ((uint32_t)QS_rxPriv_->var.flt.data[j + 3U] << 24U));
        }
        // ensure that QS_ID==0 is always on
        QS_filt_.loc[0] |= 0x00000001U;
    }
    else {
        QS_rxReportError_((uint8_t)QS_rxPriv_->var.flt.recId);
    }
    // no need to report Done
}
//............................................................................
static void QS_rxHandleGoodObj_(void) {
    QS_rxReportAck_(QS_rxPriv_->var.obj.recId);
    enum QS_ObjKind const obj_kind =
        (enum QS_ObjKind)QS_rxPriv_->var.obj.kind;
    if (QS_rxPriv_->var.obj.recId == QS_RX_CURR_OBJ) {
        if (QS_rxPriv_->var.obj.addr < 0xFFU) { // only ID?
            if (!QS_setCurrId(obj_kind,
                    (uint8_t)QS_rxPriv_->var.obj.addr)) // failure?
            {
                QS_rxReportError_((uint8_t)QS_RX_CURR_OBJ);
            }
        }
        else { // obj address provided
            void * const obj = (void *)QS_rxPriv_->var.obj.addr;
            if (!QS_setCurrObj(obj_kind, obj)) { // failure?
                QS_rxReportError_((uint8_t)QS_RX_CURR_OBJ);
            }
        }
    }
    else if (QS_rxPriv_->var.obj.recId == QS_RX_AO_FILTER) {
        void const * const obj = (void *)QS_rxPriv_->var.obj.addr;
        if (obj != (void *)0) { // object address provided?
            // in case of AO, the local filter is its prio.
            int_fast16_t const prio =
                (int_fast16_t)((QActive const *)obj)->prio;
            // obj.kind == 0 means set the filter, != remove the filter
            QS_locFilter_((QS_rxPriv_->var.obj.kind == 0U)
                          ? prio : -prio);
        }
        else { // no object address
            QS_rxReportError_((uint8_t)QS_RX_AO_FILTER);
        }
    }
    else { // incorrect filter command
        QS_rxReportError_((uint8_t)QS_rxPriv_->var.obj.recId);
    }
}
//............................................................................
static void QS_rxHandleGoodEvt_(void) {
    // NOTE: Ack was already reported in the WAIT4_EVT_LEN state
#ifdef Q_UTEST
    QS_onTestEvt(QS_rxPriv_->var.evt.e); // "massage" the evt
#endif // Q_UTEST
    uint8_t stat = 0U; // status, 0 == success,no-recycle

    if (QS_rxPriv_->var.evt.prio == 0U) { // publish
        QActive_publish_(QS_rxPriv_->var.evt.e, &QS_rxPriv_, 0U);
    }
    else if (QS_rxPriv_->var.evt.prio < QF_MAX_ACTIVE) {
        if (!QACTIVE_POST_X(QActive_registry_[QS_rxPriv_->var.evt.prio],
                   QS_rxPriv_->var.evt.e,
                   0U, // margin
                   &QS_rxPriv_))
        {
            stat = 0x80U; // failure status, no recycle
        }
    }
    else if (QS_rxPriv_->var.evt.prio == 255U) { // special prio
        QAsm * const sm = (QAsm *)QS_rxPriv_->currObj[QS_OBJ_SM];
        if (sm != (void *)0) { // current SM object provided?
            // increment the ref-ctr to simulate the situation
            // when the event is just retrieved from a queue.
            // This is expected for the following QF_gc() call.
            QEvt_refCtr_inc_(QS_rxPriv_->var.evt.e);

            if (sm->vptr != (void *)0) {
                (*sm->vptr->dispatch)(sm, QS_rxPriv_->var.evt.e, 0U);
                stat = 0x01U; // success status, recycle needed
            }
            else {
                QS_rxReportError_((uint8_t)QS_rxPriv_->var.obj.recId);
            }
        }
        else {
            stat = 0x81U;  // failure status, recycle needed
        }
    }
    else if (QS_rxPriv_->var.evt.prio == 254U) { // special prio
        QAsm * const sm = (QAsm *)QS_rxPriv_->currObj[QS_OBJ_SM];
        if (sm != (void *)0) { // current SM object provided?
            // increment the ref-ctr to simulate the situation
            // when the event is just retrieved from a queue.
            // This is expected for the following QF_gc() call.
            QEvt_refCtr_inc_(QS_rxPriv_->var.evt.e);

            if (sm->vptr != (void *)0) {
                (*sm->vptr->init)(sm, QS_rxPriv_->var.evt.e, 0U);
                stat = 0x01U; // success status, recycle needed
            }
            else {
                QS_rxReportError_((uint8_t)QS_rxPriv_->var.obj.recId);
            }
        }
        else {
            stat = 0x81U;  // failure status, recycle needed
        }
    }
    else if (QS_rxPriv_->var.evt.prio == 253U) { // special prio
        QActive * const ao = (QActive *)QS_rxPriv_->currObj[QS_OBJ_AO];
        if (ao != (void *)0) { // current AO object provided?
            if (!QACTIVE_POST_X(ao,
                    QS_rxPriv_->var.evt.e,
                    0U, // margin
                    &QS_rxPriv_))
            {
                // failed QACTIVE_POST() recycles the event
                stat = 0x80U; // failure status, no recycle
            }
        }
        else {
            stat = 0x81U;  // failure status, recycle needed
        }
    }
    else {
        stat = 0x81U;  // failure status, recycle needed
    }

#if (QF_MAX_EPOOL > 0U)
    if ((stat & 0x01U) != 0U) { // recycle needed?
        QF_gc(QS_rxPriv_->var.evt.e);
    }
#endif
    if ((stat & 0x80U) != 0U) { // failure?
        QS_rxReportError_((uint8_t)QS_RX_EVENT);
    }
    else {
#ifdef Q_UTEST
#if (Q_UTEST != 0)
        QS_processTestEvts_(); // process all events produced
#endif // (Q_UTEST != 0)
#endif // Q_UTEST
        QS_rxReportDone_(QS_RX_EVENT);
    }
}
//............................................................................
#ifdef Q_UTEST
static void QS_rxHandleGoodTestSetup_(void) {
    QS_rxReportAck_((int8_t)QS_RX_TEST_SETUP);
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_tstPriv_.tpNum    = 0U; // clear the Test-Probes
    QS_tstPriv_.testTime = 0U; // clear the time tick
    QS_CRIT_EXIT();
    // don't clear current objects
    QS_onTestSetup(); // application-specific test setup
    // no need to report Done (simple command)
}
//............................................................................
static void QS_rxHandleGoodTestProbe_(void) {
    QS_rxReportAck_((int8_t)QS_RX_TEST_PROBE);
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    Q_ASSERT_INCRIT(815, QS_tstPriv_.tpNum
        < (sizeof(QS_tstPriv_.tpBuf) / sizeof(QS_tstPriv_.tpBuf[0])));
    QS_tstPriv_.tpBuf[QS_tstPriv_.tpNum] = QS_rxPriv_->var.tp;
    ++QS_tstPriv_.tpNum;
    QS_CRIT_EXIT();
    // no need to report Done
}
#endif

//----------------------------------------------------------------------------
static void QS_rxHandleBadFrame_(uint8_t const state) {
    QS_rxReportError_(0x50U); // error for all bad frames

    switch (state) {
        case WAIT4_EVT_FRAME: {
            QS_CRIT_STAT
            QS_CRIT_ENTRY();
            Q_ASSERT_INCRIT(910, QS_rxPriv_->var.evt.e != (QEvt *)0);
            QS_CRIT_EXIT();
#if (QF_MAX_EPOOL > 0U)
            QF_gc(QS_rxPriv_->var.evt.e); // don't leak allocated evt
#endif
            break;
        }
        default: {
            // intentionally empty
            break;
        }
    }
}

//----------------------------------------------------------------------------
static void QS_queryCurrObj_(enum QS_ObjKind const obj_kind) {
    // automatic data object to collect the query information
    // cleared before querying the specified object kind
    QueryData data = {
        (void const *)0,
        (void const *)0,
        (QStateHandler)0,
        0U,
        0U,
        0U,
        0U,
        0U
    };
    bool error = true; // assume error

    // NOTE: the following code needs to run *outside* critical section
    switch (obj_kind) {
        case QS_OBJ_SM: // State Machine query
            data.obj = QS_rxPriv_->currObj[QS_OBJ_SM];
            if ((data.obj != (void *)0) // current SM-obj provided?
                && (((QAsm const *)data.obj)->vptr != (void *)0))
            {
                // virtual call to get the current state
                data.fun = (*((QAsm const *)data.obj)->vptr
                            ->getStateHandler)((QAsm const *)data.obj);
                error = false; // no error
            }
            break;

        case QS_OBJ_AO: // Active Object query
            error = QS_queryCurrAO_(&data);
            break;

#ifdef QMPOOL_H_
        case QS_OBJ_MP: // Memory Pool query
            data.obj = QS_rxPriv_->currObj[QS_OBJ_MP];
            if (data.obj != (void *)0) { // current MP-obj provided?
                data.val2 = (uint16_t)((QMPool const *)data.obj)->nFree;
                data.val1 =
                    (uint16_t)(((QMPool const *)data.obj)->nTot - data.val2);
                data.val3 = (uint16_t)((QMPool const *)data.obj)->nMin;
                data.val4 = (uint16_t)((QMPool const *)data.obj)->blockSize;
                error = false; // no error
            }
            break;
#endif

#ifdef QEQUEUE_H_
        case QS_OBJ_EQ: // Event Queue query
            data.obj = QS_rxPriv_->currObj[QS_OBJ_EQ];
            if (data.obj != (void *)0) { // current EQ-obj provided?
                data.val1 = QEQueue_getUse((QEQueue const *)data.obj);
                data.val2 = QEQueue_getFree((QEQueue const *)data.obj);
                data.val3 = ((QEQueue const *)data.obj)->nMin;
                error = false; // no error
            }
            break;
#endif

#if (QF_MAX_TICK_RATE > 0U)
        case QS_OBJ_TE: // Time Event query
            data.obj = QS_rxPriv_->currObj[QS_OBJ_TE];
            if (data.obj != (void *)0) { // current TE-obj provided?
                data.ptr  = ((QTimeEvt const *)data.obj)->act;
                data.val1 = ((QTimeEvt const *)data.obj)->ctr;
                data.val2 = ((QTimeEvt const *)data.obj)->interval;
                data.val3 = ((QTimeEvt const *)data.obj)->super.sig;
                data.val4 = ((QTimeEvt const *)data.obj)->tickRate;
                error = false; // no error
            }
            break;
#endif

#if (QF_MAX_EPOOL > 0U)
        case QS_OBJ_EP: // Event Pool query
            error = QS_queryCurrEP_(&data);
            break;
#endif
        case QS_OBJ_AP:    // Application object query
            // intentionally fall through
        case QS_OBJ_SM_AO: // State-Machine / Active Object query
            // intentionally fall through
        default:
            // do nothing
            break;
    }

    if (!error) {
        QS_queryCurrObjOut_(obj_kind, &data);
    }
    else {
        QS_rxReportError_((uint8_t)QS_RX_QUERY_CURR);
    }
}
//............................................................................
static void QS_queryCurrObjOut_(enum QS_ObjKind const obj_kind,
    QueryData const * const data)
{
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_beginRec_((uint_fast8_t)QS_QUERY_DATA);
    QS_TIME_PRE();       // timestamp
    QS_U8_PRE(obj_kind); // object kind
    switch (obj_kind) {
        case QS_OBJ_SM:
            QS_OBJ_PRE(data->obj);  // state machine object
            QS_FUN_PRE(data->fun);  // current state
            break;
        case QS_OBJ_AO:
            QS_U8_PRE(data->id);    // AO's prio.
            QS_U16_PRE(data->val1); // AO's queue use
            QS_U16_PRE(data->val2); // AO's queue free
            QS_U16_PRE(data->val3); // AO's queue min
            break;
        case QS_OBJ_MP:
            QS_OBJ_PRE(data->obj);  // memory pool object
            QS_U16_PRE(data->val1); // pool use
            QS_U16_PRE(data->val2); // pool free
            QS_U16_PRE(data->val3); // pool min
            QS_U16_PRE(data->val4); // pool block size
            break;
        case QS_OBJ_EQ:
            QS_OBJ_PRE(data->obj);  // event queue object
            QS_U16_PRE(data->val1); // queue use
            QS_U16_PRE(data->val2); // queue free
            QS_U16_PRE(data->val3); // queue min
            break;
        case QS_OBJ_TE:
            QS_OBJ_PRE(data->obj);  // time event object
            QS_OBJ_PRE(data->ptr);  // associated AO
            QS_TEC_PRE(data->val1); // current counter
            QS_TEC_PRE(data->val2); // interval
            QS_SIG_PRE(data->val3); // signal
            QS_U8_PRE (data->val4); // associated tick rate
            break;
        case QS_OBJ_EP:
            QS_U8_PRE(data->id);    // event pool number
            QS_U16_PRE(data->val1); // pool use
            QS_U16_PRE(data->val2); // pool free
            QS_U16_PRE(data->val3); // pool min
            QS_U16_PRE(data->val4); // pool block size
            break;
        case QS_OBJ_AP:    // intentionally fall through
        case QS_OBJ_SM_AO: // intentionally fall through
        default:           // intentionally empty
            break;
    }
    QS_endRec_();
    QS_CRIT_EXIT();
    QS_REC_DONE(); // user callback (if defined)
}
//............................................................................
static bool QS_queryCurrAO_(QueryData * const data) {
    data->obj = QS_rxPriv_->currObj[QS_OBJ_AO];
    if (data->obj != (void *)0) { // is current-obj provided
        data->id = ((QActive const *)data->obj)->prio;
    }
    else { // current-obj not provided
        data->id = QS_rxPriv_->currId[0]; // current-id for AO
    }
    // NOTE: id might be still 0, which is special case to query
    // *all* AO's event queue in the system
    data->val1 = QActive_getQueueUse(data->id);
    if (data->id != 0U) { // not the special case
        data->val2 = QActive_getQueueFree(data->id);
        data->val3 = QActive_getQueueMin(data->id);
    }
    return false; // no error
}
//............................................................................
static bool QS_queryCurrEP_(QueryData * const data) {
    data->id = QS_rxPriv_->currId[1]; // current-id for EP
    if ((0U < data->id) && (data->id <= QF_MAX_EPOOL)) { // in range?
        data->val1 = QF_getPoolUse(data->id);
        data->val2 = QF_getPoolFree(data->id);
        data->val3 = QF_getPoolMin(data->id);
        data->val4 =
            QF_EPOOL_EVENT_SIZE_(QF_priv_.ePool_[data->id - 1U]);
    }
    else { // special case of ID==0
        // NOTE: this is special case to query *all* event pools
        // in the system
        data->val1 = QF_getPoolUse(0U);
        data->val4 = QF_poolGetMaxBlockSize();
    }
    return false; // no error
}

//----------------------------------------------------------------------------
static void QS_rxReportAck_(QS_RxRecords const recId) {
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_beginRec_((uint_fast8_t)QS_RX_STATUS);
        QS_U8_PRE(recId); // record ID
    QS_endRec_();
    QS_CRIT_EXIT();
    QS_REC_DONE(); // user callback (if defined)
}
//............................................................................
static void QS_rxReportError_(uint8_t const code) {
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_beginRec_((uint_fast8_t)QS_RX_STATUS);
        QS_U8_PRE(0x80U | (uint8_t)code); // error code
    QS_endRec_();
    QS_CRIT_EXIT();
    QS_REC_DONE(); // user callback (if defined)
}
//............................................................................
static void QS_rxReportDone_(QS_RxRecords const recId) {
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_beginRec_((uint_fast8_t)QS_TARGET_DONE);
        QS_TIME_PRE();    // timestamp
        QS_U8_PRE(recId); // record ID
    QS_endRec_();
    QS_CRIT_EXIT();
    QS_REC_DONE(); // user callback (if defined)
}

//! @endcond
