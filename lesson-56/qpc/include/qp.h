//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_H_
#define QP_H_

//============================================================================
#define QP_VERSION_STR  "8.1.2"
#define QP_VERSION      812U
// <VER>=812 <DATE>=260105
#define QP_RELEASE      0x64F71D43U

//----------------------------------------------------------------------------
// default configuration settings
//! @cond INTERNAL

#ifndef QF_MAX_ACTIVE
#define QF_MAX_ACTIVE 32U
#endif

#if (QF_MAX_ACTIVE > 64U)
#error QF_MAX_ACTIVE exceeds the maximum of 64U;
#endif

#ifndef QF_MAX_TICK_RATE
#define QF_MAX_TICK_RATE 1U
#endif

#if (QF_MAX_TICK_RATE > 15U)
#error QF_MAX_TICK_RATE exceeds the maximum of 15U;
#endif

#ifndef QF_MAX_EPOOL
#define QF_MAX_EPOOL 3U
#endif

#if (QF_MAX_EPOOL > 15U)
#error QF_MAX_EPOOL exceeds the maximum of 15U;
#endif

#ifndef QF_TIMEEVT_CTR_SIZE
#define QF_TIMEEVT_CTR_SIZE 4U
#endif

#if (QF_TIMEEVT_CTR_SIZE > 4U)
#error QF_TIMEEVT_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U;
#endif

#ifndef QF_EVENT_SIZ_SIZE
#define QF_EVENT_SIZ_SIZE 2U
#endif

#if (QF_EVENT_SIZ_SIZE > 4U)
#error QF_EVENT_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U;
#endif

//! @endcond

//----------------------------------------------------------------------------
// global types/utilities

typedef int int_t;
typedef int enum_t;

#define Q_UNUSED_PAR(par_)  ((void)(par_))
#define Q_DIM(array_)       (sizeof(array_) / sizeof((array_)[0U]))
#define Q_UINT2PTR_CAST(type_, uint_) ((type_ *)(uint_))

extern char const QP_versionStr[24];

//----------------------------------------------------------------------------
typedef uint16_t QSignal;

//! @class QEvt
typedef struct QEvt {
    uint32_t sig         : 16; //!< @public @memberof QEvt
    uint32_t poolNum_    :  8; //!< @private @memberof QEvt
    uint32_t refCtr_     :  8; //!< @private @memberof QEvt
    uint32_t filler_;          //!< @private @memberof QEvt
} QEvt;

#define QEVT_INITIALIZER(sig_) { \
    (QSignal)(sig_), \
    (uint8_t)0x00U, \
    (uint8_t)0xE0U, \
    (uint32_t)0xE0E0E0E0U }

//! @public @memberof QEvt
void QEvt_ctor(QEvt * const me, enum_t const sig);

//! @public @memberof QEvt
QEvt *QEvt_init(QEvt * const me, uint8_t const dummy);

//! @private @memberof QEvt
void QEvt_refCtr_inc_(QEvt const* const me);

//! @private @memberof QEvt
void QEvt_refCtr_dec_(QEvt const* const me);

#ifndef Q_UNSAFE
//! @private @memberof QEvt
void QEvt_update_(QEvt * const me);

//! @private @memberof QEvt
bool QEvt_verify_(QEvt const * const me);
#endif

typedef QEvt const * QEvtPtr;

#define QEVT_DYNAMIC           ((uint8_t)0)
#define Q_EVT_CAST(class_)     ((class_ const *)(e))

//----------------------------------------------------------------------------
// QEP (hierarchical event processor) types

typedef uint_fast8_t QState;
typedef QState (*QStateHandler)(void * const me, QEvt const * const e);
typedef QState (*QActionHandler)(void * const me);

struct QXThread; // forward declaration
typedef void (* QXThreadHandler )(struct QXThread * const me);

typedef struct QMState {
    struct QMState const *superstate; //!< @private @memberof QMState
    QStateHandler const stateHandler; //!< @private @memberof QMState
    QActionHandler const entryAction; //!< @private @memberof QMState
    QActionHandler const exitAction;  //!< @private @memberof QMState
    QActionHandler const initAction;  //!< @private @memberof QMState
} QMState;

typedef struct QMTranActTable {
    QMState const *target;            //!< @private @memberof QMTranActTable
    QActionHandler const act[1];      //!< @private @memberof QMTranActTable
} QMTranActTable;

union QAsmAttr {
    QStateHandler   fun;              //!< @private @memberof QAsmAttr
    QActionHandler  act;              //!< @private @memberof QAsmAttr
    QXThreadHandler thr;              //!< @private @memberof QAsmAttr
    QMTranActTable  const *tatbl;     //!< @private @memberof QAsmAttr
    struct QMState  const *obj;       //!< @private @memberof QAsmAttr
    uintptr_t       uint;             //!< @private @memberof QAsmAttr
};

#define Q_USER_SIG      ((enum_t)4)

#define Q_STATE_CAST(handler_) ((QStateHandler)(handler_))
#define Q_ACTION_CAST(action_) ((QActionHandler)(action_))
#define Q_ACTION_NULL          ((QActionHandler)0)

//----------------------------------------------------------------------------
//! @class QAsm
typedef struct {
    struct QAsmVtable const * vptr;   //!< @protected @memberof QAsm
    union QAsmAttr state;             //!< @protected @memberof QAsm
    union QAsmAttr temp;              //!< @protected @memberof QAsm
} QAsm;

// All possible values returned from state/action handlers...
// NOTE: The numerical order is important for algorithmic correctness.
#define Q_RET_SUPER     ((QState)0U)
#define Q_RET_UNHANDLED ((QState)1U)
#define Q_RET_HANDLED   ((QState)2U)
#define Q_RET_TRAN      ((QState)3U)
#define Q_RET_TRAN_HIST ((QState)4U)

// used in QHsm only...
#define Q_RET_IGNORED   ((QState)5U)

// used in QMsm only...
#define Q_RET_ENTRY     ((QState)6U)
#define Q_RET_EXIT      ((QState)7U)
#define Q_RET_TRAN_INIT ((QState)8U)

// Reserved signals by the QP-framework (used in QHsm only)
#define Q_EMPTY_SIG     ((QSignal)0U)
#define Q_ENTRY_SIG     ((QSignal)1U)
#define Q_EXIT_SIG      ((QSignal)2U)
#define Q_INIT_SIG      ((QSignal)3U)

// NOTE: QAsm_ctor() not declared because this is a purely abstract class

struct QAsmVtable {
    void (*init)(QAsm * const me, void const * const e,
                 uint_fast8_t const qsId);
    void (*dispatch)(QAsm * const me, QEvt const * const e,
                     uint_fast8_t const qsId);
    bool (*isIn)(QAsm * const me, QStateHandler const stateHndl);
    QStateHandler (*getStateHandler)(QAsm const * const me);
};

#ifdef Q_SPY
    #define QASM_INIT(me_, par_, qsId_) \
        (*((QAsm *)(me_))->vptr->init)((QAsm *)(me_), (par_), (qsId_))
    #define QASM_DISPATCH(me_, e_, qsId_) \
        (*((QAsm *)(me_))->vptr->dispatch)((QAsm *)(me_), (e_), (qsId_))
    #define QASM_IS_IN(me_, stateHndl_) \
        (*((QAsm *)(me_))->vptr->isIn)((QAsm *)(me_), (stateHndl_))
#else
    #define QASM_INIT(me_, par_, dummy) \
        (*((QAsm *)(me_))->vptr->init)((QAsm *)(me_), (par_), 0U)
    #define QASM_DISPATCH(me_, e_, dummy) \
        (*((QAsm *)(me_))->vptr->dispatch)((QAsm *)(me_), (e_), 0U)
#endif // ndef Q_SPY

#define QASM_GET_STATE_HANDLER(me_) \
    (*((QAsm *)(me_))->vptr->getStateHandler)((QAsm *)(me_))

#define Q_ASM_UPCAST(ptr_) ((QAsm *)(ptr_))

//----------------------------------------------------------------------------
//! @class QHsm
//! @extends QAsm
typedef struct {
    QAsm super; //!< @protected @memberof QHsm
} QHsm;

//! @protected @memberof QHsm
void QHsm_ctor(QHsm * const me,
    QStateHandler const initial);

//! @private @memberof QHsm
void QHsm_init_(QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
void QHsm_dispatch_(QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
bool QHsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl);

//! @private @memberof QHsm
QStateHandler QHsm_getStateHandler_(QAsm const * const me);

//! @protected @memberof QHsm
QState QHsm_top(QHsm const * const me,
    QEvt const * const e);

//! @public @memberof QHsm
QStateHandler QHsm_state(QHsm const * const me);

//! @public @memberof QHsm
QStateHandler QHsm_childState(QHsm * const me,
    QStateHandler const parentHndl);

//! @private @memberof QHsm
size_t QHsm_tran_simple_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
size_t QHsm_tran_complex_(
    QAsm * const me,
    QStateHandler * const path,
    uint_fast8_t const qsId);

//! @private @memberof QHsm
void QHsm_enter_target_(
    QAsm * const me,
    QStateHandler * const path,
    size_t const depth,
    uint_fast8_t const qsId);

#define Q_HSM_UPCAST(ptr_) ((QHsm *)(ptr_))

#define Q_TRAN(target_) \
    ((Q_ASM_UPCAST(me))->temp.fun = Q_STATE_CAST(target_), \
     (QState)Q_RET_TRAN)
#define Q_TRAN_HIST(hist_) \
    ((Q_ASM_UPCAST(me))->temp.fun = (hist_), \
     (QState)Q_RET_TRAN_HIST)
#define Q_SUPER(super_) \
    ((Q_ASM_UPCAST(me))->temp.fun = Q_STATE_CAST(super_), \
     (QState)Q_RET_SUPER)
#define Q_HANDLED()   ((QState)Q_RET_HANDLED)
#define Q_UNHANDLED() ((QState)Q_RET_UNHANDLED)

//----------------------------------------------------------------------------
//! @class QMsm
//! @extends QAsm
typedef struct {
    QAsm super; //!< @protected @memberof QMsm
} QMsm;

//! @protected @memberof QMsm
void QMsm_ctor(QMsm * const me,
    QStateHandler const initial);

//! @private @memberof QMsm
void QMsm_init_(
    QAsm * const me,
    void const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QMsm
void QMsm_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QMsm
bool QMsm_isIn_(
    QAsm * const me,
    QStateHandler const stateHndl);

//! @private @memberof QMsm
QStateHandler QMsm_getStateHandler_(QAsm const * const me);

//! @public @memberof QMsm
QMState const * QMsm_topQMState(void);

//! @public @memberof QMsm
QMState const * QMsm_stateObj(QMsm const * const me);

//! @public @memberof QMsm
QMState const * QMsm_childStateObj(QMsm const * const me,
    QMState const * const parentHndl);

//============================================================================
// QEP-macros

#define Q_MSM_UPCAST(ptr_) ((QMsm *)(ptr_))
#ifdef Q_SPY
    #define QM_ENTRY(state_) \
        ((Q_ASM_UPCAST(me))->temp.obj = (state_), (QState)Q_RET_ENTRY)
    #define QM_EXIT(state_) \
        ((Q_ASM_UPCAST(me))->temp.obj = (state_), (QState)Q_RET_EXIT)
#else
    #define QM_ENTRY(dummy) ((QState)Q_RET_ENTRY)
    #define QM_EXIT(dummy)  ((QState)Q_RET_EXIT)
#endif // ndef Q_SPY

#define QM_TRAN(tatbl_) ((Q_ASM_UPCAST(me))->temp.tatbl \
    = (struct QMTranActTable const *)(tatbl_), (QState)Q_RET_TRAN)

#define QM_TRAN_INIT(tatbl_) ((Q_ASM_UPCAST(me))->temp.tatbl \
    = (struct QMTranActTable const *)(tatbl_), (QState)Q_RET_TRAN_INIT)

#define QM_TRAN_HIST(history_, tatbl_) \
    ((((Q_ASM_UPCAST(me))->state.obj  = (history_)), \
      ((Q_ASM_UPCAST(me))->temp.tatbl = \
          (struct QMTranActTable const *)(tatbl_))), (QState)Q_RET_TRAN_HIST)

#define QM_HANDLED()   ((QState)Q_RET_HANDLED)
#define QM_UNHANDLED() ((QState)Q_RET_UNHANDLED)
#define QM_SUPER()     ((QState)Q_RET_SUPER)
#define QM_STATE_NULL  ((QMState *)0)

//----------------------------------------------------------------------------
// QF (active object framework) types

typedef uint16_t QPrioSpec;

#if (QF_TIMEEVT_CTR_SIZE == 1U)
    typedef uint8_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    typedef uint16_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 4U)
    typedef uint32_t QTimeEvtCtr;
#endif // (QF_TIMEEVT_CTR_SIZE == 4U)

#if (QF_MAX_ACTIVE <= 8U)
    typedef uint8_t QPSetBits;
#elif (8U < QF_MAX_ACTIVE) && (QF_MAX_ACTIVE <= 16U)
    typedef uint16_t QPSetBits;
#elif (16U < QF_MAX_ACTIVE)
    typedef uint32_t QPSetBits;
#endif // (16U < QF_MAX_ACTIVE)

#ifndef QF_LOG2
    uint_fast8_t QF_LOG2(QPSetBits const bitmask);
#endif // ndef QF_LOG2

//----------------------------------------------------------------------------
//! @class QPSet
typedef struct {
    //! @private @memberof QPSet
    QPSetBits bits0;
#if (QF_MAX_ACTIVE > 32U)
    //! @private @memberof QPSet
    QPSetBits bits1;
#endif
} QPSet;

//! @public @memberof QPSet
void QPSet_setEmpty(QPSet * const me);

//! @public @memberof QPSet
bool QPSet_isEmpty(QPSet const * const me);

//! @public @memberof QPSet
bool QPSet_notEmpty(QPSet const * const me);

//! @public @memberof QPSet
bool QPSet_hasElement(QPSet const * const me, uint_fast8_t const n);

//! @public @memberof QPSet
void QPSet_insert(QPSet * const me, uint_fast8_t const n);

//! @public @memberof QPSet
void QPSet_remove(QPSet * const me, uint_fast8_t const n);

//! @public @memberof QPSet
uint_fast8_t QPSet_findMax(QPSet const * const me);

//! @struct QSubscrList
typedef struct {
    QPSet set;     //!< @private @memberof QSubscrList
} QSubscrList;

struct QEQueue; // forward declaration

//----------------------------------------------------------------------------
//! @class QActive
//! @extends QAsm
typedef struct QActive {
    QAsm super;    //!< @protected @memberof QActive
    uint8_t prio;  //!< @protected @memberof QActive
    uint8_t pthre; //!< @protected @memberof QActive

#ifdef QACTIVE_THREAD_TYPE
    QACTIVE_THREAD_TYPE thread; //!< @protected @memberof QActive
#endif

#ifdef QACTIVE_OS_OBJ_TYPE
    QACTIVE_OS_OBJ_TYPE osObject; //!< @protected @memberof QActive
#endif

#ifdef QACTIVE_EQUEUE_TYPE
    QACTIVE_EQUEUE_TYPE eQueue; //!< @protected @memberof QActive
#endif // def QACTIVE_EQUEUE_TYPE
} QActive;

//! @protected @memberof QActive
void QActive_ctor(QActive * const me,
    QStateHandler const initial);

//! @public @memberof QActive
void QActive_setAttr(QActive * const me,
    uint32_t attr1,
    void const * attr2);

//! @public @memberof QActive
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    uint_fast16_t const qLen,
    void * const stkSto,
    uint_fast16_t const stkSize,
    void const * const par);

#ifdef QACTIVE_CAN_STOP
//! @protected @memberof QActive
void QActive_stop(QActive * const me);
#endif // def QACTIVE_CAN_STOP

//! @private @memberof QActive
void QActive_register_(QActive * const me);

//! @private @memberof QActive
void QActive_unregister_(QActive * const me);

//! @private @memberof QActive
bool QActive_post_(QActive * const me,
    QEvt const * const e,
    uint_fast16_t const margin,
    void const * const sender);

//! @private @memberof QActive
void QActive_postLIFO_(QActive * const me,
    QEvt const * const e);

//! @private @memberof QActive
QEvt const * QActive_get_(QActive * const me);

//! @static @public @memberof QActive
void QActive_psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal);

//! @static @private @memberof QActive
void QActive_publish_(
    QEvt const * const e,
    void const * const sender,
    uint_fast8_t const qsId);

//! @static @public @memberof QActive
uint16_t QActive_getQueueUse(uint_fast8_t const prio);

//! @static @public @memberof QActive
uint16_t QActive_getQueueFree(uint_fast8_t const prio);

//! @static @public @memberof QActive
uint16_t QActive_getQueueMin(uint_fast8_t const prio);

//! @protected @memberof QActive
void QActive_subscribe(QActive const * const me,
    enum_t const sig);

//! @protected @memberof QActive
void QActive_unsubscribe(QActive const * const me,
    enum_t const sig);

//! @protected @memberof QActive
void QActive_unsubscribeAll(QActive const * const me);

//! @protected @memberof QActive
bool QActive_defer(QActive const * const me,
    struct QEQueue * const eq,
    QEvt const * const e);

//! @protected @memberof QActive
bool QActive_recall(QActive * const me,
    struct QEQueue * const eq);

//! @protected @memberof QActive
uint16_t QActive_flushDeferred(QActive const * const me,
    struct QEQueue * const eq,
    uint_fast16_t const num);

//! @private @memberof QActive
void QActive_evtLoop_(QActive * const me);

#define Q_ACTIVE_UPCAST(ptr_) ((QActive *)(ptr_))

//----------------------------------------------------------------------------
//! @class QMActive
//! @extends QActive
typedef struct {
    QActive super; //!< @protected @memberof QMActive
} QMActive;

//! @protected @memberof QMActive
void QMActive_ctor(QMActive * const me,
    QStateHandler const initial);

//----------------------------------------------------------------------------
#if (QF_MAX_TICK_RATE > 0U)

//! @class QTimeEvt
//! @extends QEvt
typedef struct QTimeEvt {
    QEvt super;             //!< @protected @memberof QTimeEvt

    struct QTimeEvt *next;  //!< @private @memberof QTimeEvt
    void * act;             //!< @private @memberof QTimeEvt
    QTimeEvtCtr ctr;        //!< @private @memberof QTimeEvt
    QTimeEvtCtr interval;   //!< @private @memberof QTimeEvt
    uint8_t tickRate;       //!< @private @memberof QTimeEvt
    uint8_t flags;          //!< @private @memberof QTimeEvt
} QTimeEvt;

//! @public @memberof QTimeEvt
void QTimeEvt_ctorX(QTimeEvt * const me,
    QActive * const act,
    enum_t const sig,
    uint_fast8_t const tickRate);

//! @public @memberof QTimeEvt
void QTimeEvt_armX(QTimeEvt * const me,
    uint32_t const nTicks,
    uint32_t const interval);

//! @public @memberof QTimeEvt
bool QTimeEvt_disarm(QTimeEvt * const me);

//! @public @memberof QTimeEvt
bool QTimeEvt_rearm(QTimeEvt * const me,
    uint32_t const nTicks);

//! @public @memberof QTimeEvt
bool QTimeEvt_wasDisarmed(QTimeEvt * const me);

//! @public @memberof QTimeEvt
QTimeEvtCtr QTimeEvt_getCtr(QTimeEvt const * const me);

//! @static @private @memberof QTimeEvt
void QTimeEvt_init(void);

//! @static @private @memberof QTimeEvt
void QTimeEvt_tick_(
    uint_fast8_t const tickRate,
    void const * const sender);

//! @private @memberof QTimeEvt
QTimeEvt * QTimeEvt_expire_(QTimeEvt * const me,
    QTimeEvt * const prev_link,
    QActive const * const act,
    uint_fast8_t const tickRate);

#ifdef Q_UTEST
    //! @static @private @memberof QTimeEvt
    void QTimeEvt_tick1_(
        uint_fast8_t const tickRate,
        void const * const sender);
#endif // def Q_UTEST

//! @static @public @memberof QTimeEvt
bool QTimeEvt_noActive(uint_fast8_t const tickRate);

//----------------------------------------------------------------------------
//! @class QTicker
//! @extends QActive
typedef struct {
    QActive super; //!< @protected @memberof QTicker
} QTicker;

//! @public @memberof QTicker
void QTicker_ctor(QTicker * const me,
    uint_fast8_t const tickRate);

//! @private @memberof QTicker
void QTicker_init_(QAsm * const me,
    void const * const par,
    uint_fast8_t const qsId);

//! @private @memberof QTicker
void QTicker_dispatch_(QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QTicker
void QTicker_trig_(QTicker * const me,
    void const * const sender);

#else

void QTimeEvt_init(void); // dummy init

#endif // (QF_MAX_TICK_RATE > 0U)

//----------------------------------------------------------------------------
// QF base facilities

//! @static @public @memberof QF
void QF_init(void);

//! @static @public @memberof QF
void QF_stop(void);

//! @static @public @memberof QF
int_t QF_run(void);

//! @static @public @memberof QF
void QF_onStartup(void);

//! @static @public @memberof QF
void QF_onCleanup(void);

#ifdef QF_ON_CONTEXT_SW
    //! @static @public @memberof QF
    void QF_onContextSw(
        QActive * prev,
        QActive * next);
#endif // def QF_ON_CONTEXT_SW

static inline QPrioSpec Q_PRIO(uint8_t const prio, uint8_t const pthre) {
    // combine the QF prio. preemption-threshld pthre in the upper byte
    return (QPrioSpec)((uint32_t)prio | ((uint32_t)pthre << 8U));
}

#define QF_NO_MARGIN ((uint_fast16_t)0xFFFFU)

//----------------------------------------------------------------------------
// QF dynamic memory facilities

//! @static @public @memberof QF
void QF_poolInit(
    void * const poolSto,
    uint_fast32_t const poolSize,
    uint_fast16_t const evtSize);

//! @static @public @memberof QF
uint16_t QF_poolGetMaxBlockSize(void);

//! @static @public @memberof QF
uint16_t QF_getPoolUse(uint_fast8_t const poolNum);

//! @static @public @memberof QF
uint16_t QF_getPoolFree(uint_fast8_t const poolNum);

//! @static @public @memberof QF
uint16_t QF_getPoolMin(uint_fast8_t const poolNum);

//! @static @private @memberof QF
QEvt * QF_newX_(
    uint_fast16_t const evtSize,
    uint_fast16_t const margin,
    enum_t const sig);

//! @static @public @memberof QF
void QF_gc(QEvt const * const e);

//! @static @private @memberof QF
QEvt const * QF_newRef_(
    QEvt const * const e,
    void const * const evtRef);

//! @static @private @memberof QF
void QF_deleteRef_(void const * const evtRef);

//! @static @public @memberof QF
void QF_gcFromISR(QEvt const * const e);

#ifdef QEVT_PAR_INIT
    #define Q_NEW(evtT_, sig_, ...) \
        (evtT_##_init((evtT_ *)QF_newX_((uint_fast16_t)sizeof(evtT_), \
                      QF_NO_MARGIN, (sig_)), __VA_ARGS__))
    #define Q_NEW_X(evtT_, margin_, sig_, ...) \
        (evtT_##_init((evtT_ *)QF_newX_((uint_fast16_t)sizeof(evtT_), \
                      (margin_), (sig_)), __VA_ARGS__))
#else
    #define Q_NEW(evtT_, sig_) \
        ((evtT_ *)QF_newX_((uint_fast16_t)sizeof(evtT_), \
                           QF_NO_MARGIN, (enum_t)(sig_)))
    #define Q_NEW_X(evtT_, margin_, sig_) \
        ((evtT_ *)QF_newX_((uint_fast16_t)sizeof(evtT_), \
                          (margin_), (enum_t)(sig_)))
#endif // QEVT_PAR_INIT

#define Q_NEW_REF(evtRef_, evtT_) \
    ((evtRef_) = (evtT_ const *)QF_newRef_(e, (evtRef_)))
#define Q_DELETE_REF(evtRef_) do { \
    QF_deleteRef_((evtRef_)); \
    (evtRef_) = (void *)0; \
} while (false)

#ifdef Q_SPY
    #define QACTIVE_POST(me_, e_, sender_) \
        ((void)QActive_post_((me_), (e_), QF_NO_MARGIN, (sender_)))
    #define QACTIVE_POST_X(me_, e_, margin_, sender_) \
        (QActive_post_((me_), (e_), (margin_), (sender_)))
    #define QACTIVE_PUBLISH(e_, sender_) \
        (QActive_publish_((e_), (void const *)(sender_), (sender_)->prio))
    #define QTIMEEVT_TICK_X(tickRate_, sender_) (QTimeEvt_tick_((tickRate_), (sender_)))
    #define QTICKER_TRIG(ticker_, sender_) (QTicker_trig_((ticker_), (sender_)))
#else
    #define QACTIVE_POST(me_, e_, dummy) \
        ((void)QActive_post_((me_), (e_), QF_NO_MARGIN, (void *)0))
    #define QACTIVE_POST_X(me_, e_, margin_, dummy) \
        (QActive_post_((me_), (e_), (margin_), (void *)0))
    #define QACTIVE_PUBLISH(e_, dummy) (QActive_publish_((e_), (void *)0, 0U))
    #define QTIMEEVT_TICK_X(tickRate_, dummy) (QTimeEvt_tick_((tickRate_), (void *)0))
    #define QTICKER_TRIG(ticker_, sender_) (QTicker_trig_((ticker_), (void *)0))
#endif // ndef Q_SPY

#define QACTIVE_POST_LIFO(me_, e_) (QActive_postLIFO_((me_), (e_)))
#define QTIMEEVT_TICK(sender_)     QTIMEEVT_TICK_X(0U, (sender_))

#ifndef QF_CRIT_EXIT_NOP
    #define QF_CRIT_EXIT_NOP() ((void)0)
#endif // ndef QF_CRIT_EXIT_NOP

#endif // QP_H_
