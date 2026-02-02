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
#ifndef QS_H_
#define QS_H_

#ifndef Q_SPY
    #error Q_SPY must be defined to include qs.h
#endif

//============================================================================
//! @cond INTERNAL

#ifndef QS_CTR_SIZE
#define QS_CTR_SIZE 2U
#endif

#ifndef QS_TIME_SIZE
#define QS_TIME_SIZE 4U
#endif

//! @endcond

//============================================================================
//! Pre-defined QS record IDs (for QS_GLB_FILTER())
enum QS_GlbPredef {
    // [0] QS session (not maskable)
    QS_EMPTY,             //!< QS record for cleanly starting a session

    // [1] SM records
    QS_QEP_STATE_ENTRY,   //!< a state was entered
    QS_QEP_STATE_EXIT,    //!< a state was exited
    QS_QEP_STATE_INIT,    //!< an initial transition was taken in a state
    QS_QEP_INIT_TRAN,     //!< the top-most initial transition was taken
    QS_QEP_INTERN_TRAN,   //!< an internal transition was taken
    QS_QEP_TRAN,          //!< a regular transition was taken
    QS_QEP_IGNORED,       //!< an event was ignored (silently discarded)
    QS_QEP_DISPATCH,      //!< an event was dispatched (begin of RTC step)
    QS_QEP_UNHANDLED,     //!< an event was un-handled due to a guard

    // [10] Active Object (AO) records
    QS_QF_ACTIVE_DEFER,   //!< AO deferred an event
    QS_QF_ACTIVE_RECALL,  //!< AO recalled an event
    QS_QF_ACTIVE_SUBSCRIBE,   //!< an AO subscribed to an event
    QS_QF_ACTIVE_UNSUBSCRIBE, //!< an AO unsubscribed to an event
    QS_QF_ACTIVE_POST,      //!< an event was posted (FIFO) directly to AO
    QS_QF_ACTIVE_POST_LIFO, //!< an event was posted (LIFO) directly to AO
    QS_QF_ACTIVE_GET,     //!< AO got an event and its queue is not empty
    QS_QF_ACTIVE_GET_LAST,//!< AO got an event and its queue is empty
    QS_QF_ACTIVE_RECALL_ATTEMPT, //!< AO attempted to recall an event

    // [19] Event Queue (EQ) records
    QS_QF_EQUEUE_POST,      //!< an event was posted (FIFO) to a raw queue
    QS_QF_EQUEUE_POST_LIFO, //!< an event was posted (LIFO) to a raw queue
    QS_QF_EQUEUE_GET,     //!< get an event and queue still not empty
    QS_QF_EQUEUE_GET_LAST,//!< get the last event from the queue

    // [23] Framework (QF) records
    QS_QF_NEW_ATTEMPT,    //!< an attempt to allocate an event failed

    // [24] Memory Pool (MP) records
    QS_QF_MPOOL_GET,      //!< a memory block was removed from memory pool
    QS_QF_MPOOL_PUT,      //!< a memory block was returned to memory pool

    // [26] Additional Framework (QF) records
    QS_QF_PUBLISH,        //!< an event was published to active objects
    QS_QF_NEW_REF,        //!< new event reference was created
    QS_QF_NEW,            //!< new event was created
    QS_QF_GC_ATTEMPT,     //!< garbage collection attempt
    QS_QF_GC,             //!< garbage collection
    QS_QF_TICK,           //!< QTimeEvt tick was called

    // [32] Time Event (TE) records
    QS_QF_TIMEEVT_ARM,    //!< a time event was armed
    QS_QF_TIMEEVT_AUTO_DISARM, //!< a time event expired and was disarmed
    QS_QF_TIMEEVT_DISARM_ATTEMPT,//!< attempt to disarm a disarmed QTimeEvt
    QS_QF_TIMEEVT_DISARM, //!< true disarming of an armed time event
    QS_QF_TIMEEVT_REARM,  //!< rearming of a time event
    QS_QF_TIMEEVT_POST,   //!< a time event posted itself directly to an AO

    // [38] Additional Framework (QF) records
    QS_QF_DELETE_REF,     //!< an event reference is about to be deleted
    QS_QF_CRIT_ENTRY,     //!< critical section was entered
    QS_QF_CRIT_EXIT,      //!< critical section was exited
    QS_QF_ISR_ENTRY,      //!< an ISR was entered
    QS_QF_ISR_EXIT,       //!< an ISR was exited
    QS_QF_INT_DISABLE,    //!< interrupts were disabled
    QS_QF_INT_ENABLE,     //!< interrupts were enabled

    // [45] Additional Active Object (AO) records
    QS_QF_ACTIVE_POST_ATTEMPT,//!< attempt to post an evt to AO failed

    // [46] Additional Event Queue (EQ) records
    QS_QF_EQUEUE_POST_ATTEMPT,//!< attempt to post evt to QEQueue failed

    // [47] Additional Memory Pool (MP) records
    QS_QF_MPOOL_GET_ATTEMPT,  //!< attempt to get a memory block failed

    // [48] Scheduler (SC) records
    QS_SCHED_PREEMPT,     //!< scheduler asynchronously preempted a task
    QS_SCHED_RESTORE,     //!< scheduler restored preempted task
    QS_SCHED_LOCK,        //!< scheduler was locked
    QS_SCHED_UNLOCK,      //!< scheduler was unlocked
    QS_SCHED_NEXT,        //!< scheduler started next task
    QS_SCHED_IDLE,        //!< scheduler restored the idle task

    // [54] Miscellaneous QS records (not maskable)
    QS_ENUM_DICT,         //!< enumeration dictionary entry

    // [55] Additional QEP records
    QS_QEP_TRAN_HIST,     //!< a tran. to history was taken
    QS_RESERVED_56,       //!< reserved (previously QS_QEP_TRAN_EP)
    QS_RESERVED_57,       //!< reserved (previously QS_QEP_TRAN_XP)

    // [58] Miscellaneous QS records (not maskable)
    QS_TEST_PAUSED,       //!< test has been paused
    QS_TEST_PROBE_GET,    //!< reports that Test-Probe has been used
    QS_SIG_DICT,          //!< signal dictionary entry
    QS_OBJ_DICT,          //!< object dictionary entry
    QS_FUN_DICT,          //!< function dictionary entry
    QS_USR_DICT,          //!< user QS record dictionary entry
    QS_TARGET_INFO,       //!< reports the Target information
    QS_TARGET_DONE,       //!< reports completion of a user callback
    QS_RX_STATUS,         //!< reports QS data receive status
    QS_QUERY_DATA,        //!< reports the data from "current object" query
    QS_PEEK_DATA,         //!< reports the data from the PEEK query
    QS_ASSERT_FAIL,       //!< assertion failed in the code
    QS_QF_RUN,            //!< QF_run() was entered

    // [71] Semaphore (SEM) records
    QS_SEM_TAKE,          //!< a semaphore was taken by a thread
    QS_SEM_BLOCK,         //!< a semaphore blocked a thread
    QS_SEM_SIGNAL,        //!< a semaphore was signaled
    QS_SEM_BLOCK_ATTEMPT, //!< a semaphore blocked was attempted

    // [75] Mutex (MTX) records
    QS_MTX_LOCK,          //!< a mutex was locked
    QS_MTX_BLOCK,         //!< a mutex blocked a thread
    QS_MTX_UNLOCK,        //!< a mutex was unlocked
    QS_MTX_LOCK_ATTEMPT,  //!< a mutex lock was attempted
    QS_MTX_BLOCK_ATTEMPT, //!< a mutex blocking was attempted
    QS_MTX_UNLOCK_ATTEMPT,//!< a mutex unlock was attempted

    // [81] Additional QF (AO) records
    QS_QF_ACTIVE_DEFER_ATTEMPT, //!< AO attempted to defer an event

    // [82] keep always last
    QS_PRE_MAX            //!< the # predefined signals
};

// QS record groups (for QS_GLB_FILTER())
#define QS_GRP_ALL  ((int_fast16_t)0xF0)
#define QS_GRP_SM   ((int_fast16_t)0xF1)
#define QS_GRP_AO   ((int_fast16_t)0xF2)
#define QS_GRP_EQ   ((int_fast16_t)0xF3)
#define QS_GRP_MP   ((int_fast16_t)0xF4)
#define QS_GRP_TE   ((int_fast16_t)0xF5)
#define QS_GRP_QF   ((int_fast16_t)0xF6)
#define QS_GRP_SC   ((int_fast16_t)0xF7)
#define QS_GRP_SEM  ((int_fast16_t)0xF8)
#define QS_GRP_MTX  ((int_fast16_t)0xF9)
#define QS_GRP_U0   ((int_fast16_t)0xFA)
#define QS_GRP_U1   ((int_fast16_t)0xFB)
#define QS_GRP_U2   ((int_fast16_t)0xFC)
#define QS_GRP_U3   ((int_fast16_t)0xFD)
#define QS_GRP_U4   ((int_fast16_t)0xFE)
#define QS_GRP_UA   ((int_fast16_t)0xFF)

// QS user record group offsets (for app-specific records)
#define QS_USER     ((enum_t)100)
#define QS_USER0    QS_USER
#define QS_USER1    ((enum_t)(QS_USER + 5))
#define QS_USER2    ((enum_t)(QS_USER + 10))
#define QS_USER3    ((enum_t)(QS_USER + 15))
#define QS_USER4    ((enum_t)(QS_USER + 20))

// QS ID group offsets (for QS_LOC_FILTER())
#define QS_ID_AO   ((uint_fast16_t)0)
#define QS_ID_EP   ((uint_fast16_t)64)
#define QS_ID_EQ   ((uint_fast16_t)80)
#define QS_ID_AP   ((uint_fast16_t)96)

// QS ID groups (for QS_LOC_FILTER())
#define QS_IDS_ALL ((int_fast16_t)0xF0)
#define QS_IDS_AO  ((int_fast16_t)0xF1)
#define QS_IDS_EP  ((int_fast16_t)0xF2)
#define QS_IDS_EQ  ((int_fast16_t)0xF3)
#define QS_IDS_AP  ((int_fast16_t)0xF4)

#define QS_EOD     ((uint16_t)0xFFFFU)
#define QS_CMD     ((uint8_t)0x07U)

//! @struct QSpyId
typedef struct {
    uint8_t prio;
} QSpyId;

#if (QS_OBJ_PTR_SIZE == 2U)
    typedef uint16_t QSObj;
#elif (QS_OBJ_PTR_SIZE == 4U)
    typedef uint32_t QSObj;
#elif (QS_OBJ_PTR_SIZE == 8U)
    typedef uint64_t QSObj;
#endif //  (QS_OBJ_PTR_SIZE == 8U)

#if (QS_FUN_PTR_SIZE == 2U)
    typedef uint16_t QSFun;
#elif (QS_FUN_PTR_SIZE == 4U)
    typedef uint32_t QSFun;
#elif (QS_FUN_PTR_SIZE == 8U)
    typedef uint64_t QSFun;
#endif //  (QS_FUN_PTR_SIZE == 8U)

//! @static @private @memberof QS
typedef void (* QSpyFunPtr )(void);

#if (QS_CTR_SIZE == 1U)
    typedef uint8_t QSCtr;
#elif (QS_CTR_SIZE == 2U)
    typedef uint16_t QSCtr;
#elif (QS_CTR_SIZE == 4U)
    typedef uint32_t QSCtr;
#endif //  (QS_CTR_SIZE == 4U)

#if (QS_TIME_SIZE == 2U)
    typedef uint16_t QSTimeCtr;
#elif (QS_TIME_SIZE == 4U)
    typedef uint32_t QSTimeCtr;
#endif //  (QS_TIME_SIZE == 4U)

//============================================================================
#define QS_INIT(arg_)        (QS_onStartup(arg_))
#define QS_EXIT()            (QS_onCleanup())
#define QS_OUTPUT()          (QS_output())
#define QS_RX_INPUT()        (QS_rx_input())
#define QS_GLB_FILTER(rec_)  (QS_glbFilter_((int_fast16_t)(rec_)))
#define QS_LOC_FILTER(qsId_) (QS_locFilter_((int_fast16_t)(qsId_)))
#define QS_FLUSH()           (QS_onFlush())

#define QS_BEGIN_ID(rec_, qsId_) \
if (QS_fltCheck_((uint32_t)(rec_) >> 5U, \
                 (uint32_t)1U << ((uint32_t)(rec_) & 0x1FU), \
                 (qsId_))) { \
    QS_CRIT_STAT \
    QS_CRIT_ENTRY(); \
    QS_beginRec_((uint_fast8_t)(rec_)); \
    QS_TIME_PRE(); {

#define QS_END() } \
    QS_endRec_(); \
    QS_CRIT_EXIT(); \
}

#define QS_BEGIN_INCRIT(rec_, qsId_) \
if (QS_fltCheck_((uint32_t)(rec_) >> 5U, \
                 (uint_fast32_t)1U << ((uint32_t)(rec_) & 0x1FU), (qsId_))) \
{ \
    QS_beginRec_((uint_fast8_t)(rec_)); \
    QS_TIME_PRE(); {

#define QS_END_INCRIT() } \
    QS_endRec_(); \
}

#define QS_GLB_CHECK_(rec_) \
    (QS_glbCheck_((uint32_t)(rec_) >> 5U, \
                  (uint32_t)1U << ((uint32_t)(rec_) & 0x1FU)))
#define QS_LOC_CHECK_(qsId_) (QS_locCheck_((qsId_)))

#ifndef QS_REC_DONE
    #define QS_REC_DONE() ((void)0)
#endif // ndef QS_REC_DONE

#define QS_I8(width_, data_) \
    (QS_u8_fmt_((uint8_t)(((width_) << 4U) & 0x7U) | QS_I8_ENUM_FMT, \
                (data_)))

#define QS_U8(width_, data_) \
    (QS_u8_fmt_((uint8_t)(((width_) << 4U)) | QS_U8_FMT, (data_)))

#define QS_I16(width_, data_) \
    (QS_u16_fmt_((uint8_t)(((width_) << 4U)) | QS_I16_FMT, (data_)))

#define QS_U16(width_, data_) \
    (QS_u16_fmt_((uint8_t)(((width_) << 4U)) | QS_U16_FMT, (data_)))

#define QS_I32(width_, data_) \
    (QS_u32_fmt_((uint8_t)(((width_) << 4U)) | QS_I32_FMT, (data_)))

#define QS_U32(width_, data_) \
    (QS_u32_fmt_((uint8_t)(((width_) << 4U)) | QS_U32_FMT, (data_)))

#define QS_I64(width_, data_) \
    (QS_u64_fmt_((uint8_t)(((width_) << 4U)) | QS_I64_FMT, (data_)))

#define QS_U64(width_, data_) \
    (QS_u64_fmt_((uint8_t)(((width_) << 4U)) | QS_U64_FMT, (data_)))

#define QS_F32(width_, data_) \
    (QS_f32_fmt_((uint8_t)(((width_) << 4U)) | QS_F32_FMT, (data_)))

#define QS_F64(width_, data_) \
    (QS_f64_fmt_((uint8_t)(((width_) << 4U)) | QS_F64_FMT, (data_)))

#define QS_STR(str_) (QS_str_fmt_((str_)))

#define QS_MEM(mem_, size_) (QS_mem_fmt_((mem_), (size_)))

#define QS_ENUM(group_, value_) \
    (QS_u8_fmt_((uint8_t)(0x80U | ((group_) << 4U)) | QS_I8_ENUM_FMT, \
                (uint8_t)(value_)))

#if (QS_TIME_SIZE == 2U)
    #define QS_TIME_PRE() (QS_u16_raw_(QS_onGetTime()))
#elif (QS_TIME_SIZE == 4U)
    #define QS_TIME_PRE() (QS_u32_raw_(QS_onGetTime()))
#endif //  (QS_TIME_SIZE == 4U)

#if (QS_OBJ_PTR_SIZE == 2U)
    #define QS_OBJ(obj_) (QS_u16_fmt_(QS_OBJ_FMT, (uint16_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4U)
    #define QS_OBJ(obj_) (QS_u32_fmt_(QS_OBJ_FMT, (uint32_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8U)
    #define QS_OBJ(obj_) (QS_u64_fmt_(QS_OBJ_FMT, (uint64_t)(obj_)))
#endif // (QS_OBJ_PTR_SIZE == 8U)

#if (QS_FUN_PTR_SIZE == 2U)
    #define QS_FUN(fun_) (QS_u16_fmt_(QS_FUN_FMT, (uint16_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 4U)
    #define QS_FUN(fun_) (QS_u32_fmt_(QS_FUN_FMT, (uint32_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 8U)
    #define QS_FUN(fun_) (QS_u64_fmt_(QS_FUN_FMT, (uint64_t)(fun_)))
#endif // (QS_FUN_PTR_SIZE == 8U)

#define QS_SIG(sig_, obj_) \
    QS_u16_fmt_(QS_SIG_FMT, (sig_)); \
    QS_obj_raw_(obj_)

#define QS_SIG_DICTIONARY(sig_, obj_) \
    (QS_sig_dict_pre_((QSignal)(sig_), (obj_), #sig_))

#define QS_OBJ_DICTIONARY(obj_) \
    (QS_obj_dict_pre_((obj_), #obj_))

#define QS_OBJ_ARR_DICTIONARY(obj_, idx_) \
    (QS_obj_arr_dict_pre_((obj_), (idx_), #obj_))

#define QS_FUN_DICTIONARY(fun_) \
    (QS_fun_dict_pre_((void (*)(void))(fun_), #fun_))

#define QS_USR_DICTIONARY(rec_) \
    (QS_usr_dict_pre_((rec_), #rec_))

#define QS_ENUM_DICTIONARY(value_, group_) \
    (QS_enum_dict_pre_((value_), (group_), #value_))

#define QS_RX_PUT(b_) (QS_rxPut((b_)))

#define QS_TR_CRIT_ENTRY()  (QS_crit_entry_pre_())
#define QS_TR_CRIT_EXIT()   (QS_crit_exit_pre_())

#define QS_TR_ISR_ENTRY(isrnest_, prio_) \
    (QS_isr_entry_pre_((isrnest_), (prio_)))
#define QS_TR_ISR_EXIT(isrnest_, prio_) \
    (QS_isr_exit_pre_((isrnest_), (prio_)))

#define QS_ONLY(code_) (code_)

#define QS_ASSERTION(module_, id_, delay_) \
    (QS_assertion_pre_((module_), (id_), (delay_)))

#ifndef QS_NFLOAT_T
    typedef float float32_t;
    typedef double float64_t;
#endif

//============================================================================
#ifndef QS_CRIT_STAT
    #define QS_CRIT_STAT QF_CRIT_STAT
#endif // ndef QS_CRIT_STAT

#ifndef QS_CRIT_ENTRY
    #define QS_CRIT_ENTRY() QF_CRIT_ENTRY()
#endif // ndef QS_CRIT_ENTRY

#ifndef QS_CRIT_EXIT
    #define QS_CRIT_EXIT() QF_CRIT_EXIT()
#endif // ndef QS_CRIT_EXIT

//============================================================================
//! @class QS
typedef struct {
    uint8_t *buf;      //!< @private @memberof QS
    QSCtr    end;      //!< @private @memberof QS
    QSCtr    head;     //!< @private @memberof QS
    QSCtr    tail;     //!< @private @memberof QS
    QSCtr    used;     //!< @private @memberof QS
    uint8_t  seq;      //!< @private @memberof QS
    uint8_t  chksum;   //!< @private @memberof QS
    uint8_t  critNest; //!< @private @memberof QS
    uint8_t  flags;    //!< @private @memberof QS
} QS;

//! @static @private @memberof QS
extern QS QS_priv_;

//----------------------------------------------------------------------------
//! @cond INTERNAL

//! @struct QS_Filter
typedef struct {
    uint32_t glb[4];
    uint32_t loc[4];
} QS_Filter;

//! @static @private @memberof QS
extern QS_Filter QS_filt_;

void QS_glbFilter_(int_fast16_t const filterSpec);
void QS_locFilter_(int_fast16_t const filterSpec);

bool QS_fltCheck_(uint_fast8_t const recIdx, uint_fast32_t const recBit,
                  uint_fast8_t const qsId);
bool QS_glbCheck_(uint_fast8_t const recIdx, uint_fast32_t const recBit);
bool QS_locCheck_(uint_fast8_t const qsId);

void QS_beginRec_(uint_fast8_t const rec);
void QS_endRec_(void);

void QS_u8_raw_(uint8_t const d);
void QS_2u8_raw_(uint8_t const d1, uint8_t const d2);
void QS_u16_raw_(uint16_t const d);
void QS_u32_raw_(uint32_t const d);
void QS_u64_raw_(uint64_t const d);
void QS_obj_raw_(void const * const obj);
void QS_str_raw_(char const * const str);

void QS_u8_fmt_(uint8_t const format, uint8_t const d);
void QS_u16_fmt_(uint8_t const format, uint16_t const d);
void QS_u32_fmt_(uint8_t const format, uint32_t const d);
void QS_u64_fmt_(uint8_t const format, uint64_t const d);
void QS_f32_fmt_(uint8_t const format, float32_t const f);
void QS_f64_fmt_(uint8_t const format, float64_t const d);
void QS_str_fmt_(char const * const str);
void QS_mem_fmt_(uint8_t const * const blk, uint8_t const size);

void QS_sig_dict_pre_(QSignal const sig, void const * const obj,
    char const * const name);
void QS_obj_dict_pre_(void const * const obj, char const * const name);
void QS_obj_arr_dict_pre_(void const * const obj, uint_fast16_t const idx,
    char const * const name);
void QS_fun_dict_pre_(QSpyFunPtr const fun, char const * const name);
void QS_usr_dict_pre_(enum_t const rec, char const * const name);
void QS_enum_dict_pre_(enum_t const value, uint8_t const group,
    char const * const name);

void QS_crit_entry_pre_(void);
void QS_crit_exit_pre_(void);
void QS_isr_entry_pre_(uint8_t const isrnest, uint8_t const prio);
void QS_isr_exit_pre_(uint8_t const isrnest, uint8_t const prio);

void QS_assertion_pre_(char const * const module, int_t const id,
    uint16_t const delay);

// Formats for data elements for app-specific trace records
#define QS_I8_ENUM_FMT ((uint8_t)0x0U)
#define QS_U8_FMT      ((uint8_t)0x1U)
#define QS_I16_FMT     ((uint8_t)0x2U)
#define QS_U16_FMT     ((uint8_t)0x3U)
#define QS_I32_FMT     ((uint8_t)0x4U)
#define QS_U32_FMT     ((uint8_t)0x5U)
#define QS_F32_FMT     ((uint8_t)0x6U)
#define QS_F64_FMT     ((uint8_t)0x7U)
#define QS_STR_FMT     ((uint8_t)0x8U)
#define QS_MEM_FMT     ((uint8_t)0x9U)
#define QS_SIG_FMT     ((uint8_t)0xAU)
#define QS_OBJ_FMT     ((uint8_t)0xBU)
#define QS_FUN_FMT     ((uint8_t)0xCU)
#define QS_I64_FMT     ((uint8_t)0xDU)
#define QS_U64_FMT     ((uint8_t)0xEU)
#define QS_HEX_FMT     ((uint8_t)0xFU)

//! @endcond
//----------------------------------------------------------------------------

struct QS_RxAttr; // forward declaration

//! @static @private @memberof QS
extern struct QS_RxAttr * const QS_rxPriv_;

//! @static @public @memberof QS
void QS_initBuf(
    uint8_t * const sto,
    uint_fast32_t const stoSize);

//! @static @public @memberof QS
uint16_t QS_getByte(void);

//! @static @public @memberof QS
uint8_t const * QS_getBlock(uint16_t * const pNbytes);

//! @static @public @memberof QS
void QS_doOutput(void);

//! @static @public @memberof QS
uint8_t QS_onStartup(void const * arg);

//! @static @public @memberof QS
void QS_onCleanup(void);

//! @static @public @memberof QS
void QS_onFlush(void);

//! @static @public @memberof QS
QSTimeCtr QS_onGetTime(void);

#define QUTEST_ON_POST 124

//----------------------------------------------------------------------------
// QS-RX (QS receive channel)

//! Object kinds used in QS-RX (NOTE: watch out for backwards compatibility!)
enum QS_ObjKind {
    QS_OBJ_SM,    //!< State Machine
    QS_OBJ_AO,    //!< Active Object
    QS_OBJ_MP,    //!< Event Pool
    QS_OBJ_EQ,    //!< Event Queue
    QS_OBJ_TE,    //!< Time Event
    QS_OBJ_AP,    //!< generic Application-specific object
    QS_OBJ_SM_AO, //!< state machine AND active object
    QS_OBJ_EP,    //!< Event Pool
};

//! @static @public @memberof QS
void QS_rxInitBuf(
    uint8_t * const sto,
    uint16_t const stoSize);

//! @static @public @memberof QS
bool QS_rxPut(uint8_t const b);

//! @static @public @memberof QS
void QS_rxParse(void);

//! @static @public @memberof QS
void QS_rxParseBuf(uint16_t const len);

//! @static @public @memberof QS
uint16_t QS_rxGetFree(void);

//! @static @public @memberof QS
bool QS_setCurrObj(enum QS_ObjKind const obj_kind, void * const obj_ptr);

//! @static @public @memberof QS
void *QS_getCurrObj(enum QS_ObjKind const obj_kind);

//! @static @public @memberof QS
bool QS_setCurrId(enum QS_ObjKind const obj_kind, uint8_t const obj_id);

//! @static @public @memberof QS
uint8_t QS_getCurrId(enum QS_ObjKind const obj_kind);

//! @static @public @memberof QS
void QS_doInput(void);

//! @static @public @memberof QS
void QS_onReset(void);

//! @static @public @memberof QS
void QS_onCommand(
    uint8_t cmdId,
    uint32_t param1,
    uint32_t param2,
    uint32_t param3);

typedef enum {
    QS_TARGET_NO_RESET,
    QS_TARGET_RESET
} QS_ResetAction;

void QS_target_info_pre_(QS_ResetAction const act);

//============================================================================
#ifdef Q_UTEST
//! @static @public @memberof QS
void QS_onTestSetup(void);

//! @static @public @memberof QS
void QS_onTestTeardown(void);

//! @static @public @memberof QS
void QS_onTestEvt(QEvt * e);

//! @static @public @memberof QS
void QS_onTestPost(
    void const * sender,
    QActive * recipient,
    QEvt const * e,
    bool status);

//! @static @public @memberof QS
void QS_onTestLoop(void);

//! @cond INTERNAL
struct QS_TProbe {
    QSFun    addr;
    uint32_t data;
    uint8_t  idx;
};

//! @static @private @memberof QS
uint32_t QS_getTestProbe_(QSpyFunPtr const api);

typedef struct {
    struct QS_TProbe tpBuf[16];
    QPSet     readySet;
    QPSet     readySet_dis;
    QSTimeCtr testTime;
    uint8_t   tpNum;
    uint8_t   intLock;
    uint8_t   lockCeil;
    uint8_t   memProt;
    bool      inTestLoop;
} QSTestAttr;

//! @static @private @memberof QS
extern QSTestAttr QS_tstPriv_;

//! @static @private @memberof QS
void QS_test_pause_(void);

#if (Q_UTEST != 0)
//! @static @private @memberof QS
void QS_processTestEvts_(void);
#endif // Q_UTEST != 0
//! @endcond

//============================================================================
// QP-stub for QUTest
// NOTE: The QP-stub is needed for unit testing QP applications,
// but might NOT be needed for testing QP itself.
#if (Q_UTEST != 0)

//----------------------------------------------------------------------------
//! @class QHsmDummy
//! @extends QAsm
typedef struct {
    QAsm super; //!< @protected @memberof QHsmDummy
} QHsmDummy;

//! @public @memberof QHsmDummy
void QHsmDummy_ctor(QHsmDummy * const me);

//! @cond INTERNAL
//! @private @memberof QHsmDummy
void QHsmDummy_init_(
    QAsm * const me,
    void const * const par,
    uint_fast8_t const qsId);

//! @private @memberof QHsmDummy
void QHsmDummy_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QHsmDummy
bool QHsmDummy_isIn_(
    QAsm * const me,
    QStateHandler const state);

//! @private @memberof QHsmDummy
QStateHandler QHsmDummy_getStateHandler_(QAsm const * const me);
//! @endcond

//----------------------------------------------------------------------------
//! @class QActiveDummy
//! @extends QActive
typedef struct {
    QActive super; //!< @protected @memberof QActiveDummy
} QActiveDummy;

//! @public @memberof QActiveDummy
void QActiveDummy_ctor(QActiveDummy * const me);

//! @cond INTERNAL
//! @private @memberof QActiveDummy
void QActiveDummy_init_(
    QAsm * const me,
    void const * const par,
    uint_fast8_t const qsId);

//! @private @memberof QActiveDummy
void QActiveDummy_dispatch_(
    QAsm * const me,
    QEvt const * const e,
    uint_fast8_t const qsId);

//! @private @memberof QActiveDummy
bool QActiveDummy_fakePost_(
    QActive * const me,
    QEvt const * const e,
    uint_fast16_t const margin,
    void const * const sender);

//! @private @memberof QActiveDummy
void QActiveDummy_fakePostLIFO_(
    QActive * const me,
    QEvt const * const e);
//! @endcond

//----------------------------------------------------------------------------
// critical section for QP-stub...
typedef uint_fast8_t QCritStatus;
QCritStatus QF_critEntry(void);
void QF_critExit(QCritStatus critStat);
//............................................................................
// scheduler locking for QP-stub...
typedef uint_fast8_t QSchedStatus;
QSchedStatus QF_schedLock(uint_fast8_t const ceiling);
void QF_schedUnlock(QSchedStatus const prevCeil);

#endif // Q_UTEST != 0

//----------------------------------------------------------------------------
#define QS_TEST_PROBE_DEF(fun_) \
    uint32_t const qs_tp_ = QS_getTestProbe_((void (*)(void))(fun_));
#define QS_TEST_PROBE(code_)         if (qs_tp_ != 0U) { code_ }
#define QS_TEST_PROBE_ID(id_, code_) if (qs_tp_ == (uint32_t)(id_)) { code_ }
#define QS_TEST_PAUSE()              (QS_test_pause_())

#else // Q_UTEST not defined

// dummy definitions when not building for QUTEST
#define QS_TEST_PROBE_DEF(fun_)
#define QS_TEST_PROBE(code_)
#define QS_TEST_PROBE_ID(id_, code_)
#define QS_TEST_PAUSE()  ((void)0)

//----------------------------------------------------------------------------
// memory protection facilities

#ifdef QF_MEM_ISOLATE
    #error Memory isolation not supported in this QP edition, need SafeQP
#endif

#endif // Q_UTEST

#endif // QS_H_
