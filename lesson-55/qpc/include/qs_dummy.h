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
#ifndef QS_DUMMY_H_
#define QS_DUMMY_H_

#ifdef Q_SPY
    #error Q_SPY must NOT be defined to include qs_dummy.h
#endif

#ifdef Q_UTEST
    #error Q_UTEST must NOT be defined to include qs_dummy.h
#endif

#define QS_INIT(arg_)                   ((uint8_t)1U)
#define QS_EXIT()                       ((void)0)
#define QS_DUMP()                       ((void)0)
#define QS_GLB_FILTER(rec_)             ((void)0)
#define QS_LOC_FILTER(qsId_)            ((void)0)

#define QS_BEGIN_ID(rec_, qsId_)        if (false) {
#define QS_END()                        }
#define QS_BEGIN_INCRIT(rec_, qsId_)    if (false) {
#define QS_END_INCRIT()                 }

#define QS_I8(width_, data_)            ((void)0)
#define QS_U8(width_, data_)            ((void)0)
#define QS_I16(width_, data_)           ((void)0)
#define QS_U16(width_, data_)           ((void)0)
#define QS_I32(width_, data_)           ((void)0)
#define QS_U32(width_, data_)           ((void)0)
#define QS_F32(width_, data_)           ((void)0)
#define QS_F64(width_, data_)           ((void)0)
#define QS_I64(width_, data_)           ((void)0)
#define QS_U64(width_, data_)           ((void)0)
#define QS_ENUM(group_, value_)         ((void)0)
#define QS_STR(str_)                    ((void)0)
#define QS_MEM(mem_, size_)             ((void)0)
#define QS_SIG(sig_, obj_)              ((void)0)
#define QS_OBJ(obj_)                    ((void)0)
#define QS_FUN(fun_)                    ((void)0)

#define QS_SIG_DICTIONARY(sig_, obj_)   ((void)0)
#define QS_OBJ_DICTIONARY(obj_)         ((void)0)
#define QS_OBJ_ARR_DICTIONARY(obj_, idx_) ((void)0)
#define QS_FUN_DICTIONARY(fun_)         ((void)0)
#define QS_USR_DICTIONARY(rec_)         ((void)0)
#define QS_ENUM_DICTIONARY(value_, group_)  ((void)0)
#define QS_ASSERTION(module_, loc_, delay_) ((void)0)
#define QS_FLUSH()                      ((void)0)

#define QS_TEST_PROBE_DEF(fun_)
#define QS_TEST_PROBE(code_)
#define QS_TEST_PROBE_ID(id_, code_)
#define QS_TEST_PAUSE()                 ((void)0)

#define QS_OUTPUT()                     ((void)0)
#define QS_RX_INPUT()                   ((void)0)
#define QS_RX_PUT(b_)                   ((void)0)
#define QS_ONLY(code_)                  ((void)0)

//============================================================================
// QS API used inside applications
#ifndef QS_TIME_SIZE
    typedef uint32_t QSTimeCtr;
#elif (QS_TIME_SIZE == 2U)
    typedef uint16_t QSTimeCtr;
#else
    typedef uint32_t QSTimeCtr;
#endif

void QS_initBuf(uint8_t * const sto, uint_fast32_t const stoSize);
uint16_t QS_getByte(void);
uint8_t const *QS_getBlock(uint16_t* const pNbytes);
void QS_doOutput(void);
uint8_t QS_onStartup(void const* arg);
void QS_onCleanup(void);
void QS_onFlush(void);
QSTimeCtr QS_onGetTime(void);

void QS_rxInitBuf(uint8_t * const sto, uint16_t const stoSize);
void QS_rxParse(void);

//============================================================================
// QS API used inside test fixtures
#ifdef Q_UTEST

#if (QS_FUN_PTR_SIZE == 2U)
    typedef uint16_t QSFun;
#elif (QS_FUN_PTR_SIZE == 4U)
    typedef uint32_t QSFun;
#elif (QS_FUN_PTR_SIZE == 8U)
    typedef uint64_t QSFun;
#endif

struct QS_TProbe {
    QSFun    addr;
    uint32_t data;
    uint8_t  idx;
};

void QS_onTestSetup(void);
void QS_onTestTeardown(void);
void QS_onTestEvt(QEvt *e);
void QS_onTestPost(void const *sender, QActive *recipient,
                   QEvt const *e, bool status);
void QS_onTestLoop(void);

#endif // def Q_UTEST

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL
    // predefined QS trace records
    #define QS_BEGIN_PRE(rec_, qsId_)   if (false) {
    #define QS_END_PRE()                }
    #define QS_U8_PRE(data_)            ((void)0)
    #define QS_2U8_PRE(data1_, data2_)  ((void)0)
    #define QS_U16_PRE(data_)           ((void)0)
    #define QS_U32_PRE(data_)           ((void)0)
    #define QS_TIME_PRE()               ((void)0)
    #define QS_SIG_PRE(sig_)            ((void)0)
    #define QS_EVS_PRE(size_)           ((void)0)
    #define QS_OBJ_PRE(obj_)            ((void)0)
    #define QS_FUN_PRE(fun_)            ((void)0)
    #define QS_EQC_PRE(ctr_)            ((void)0)
    #define QS_MPC_PRE(ctr_)            ((void)0)
    #define QS_MPS_PRE(size_)           ((void)0)
    #define QS_TEC_PRE(ctr_)            ((void)0)

    #define QS_CRIT_STAT
    #define QS_CRIT_ENTRY()              ((void)0)
    #define QS_CRIT_EXIT()               ((void)0)

    #define QS_TR_CRIT_ENTRY()           ((void)0)
    #define QS_TR_CRIT_EXIT()            ((void)0)
    #define QS_TR_ISR_ENTRY(isrnest_, prio_) ((void)0)
    #define QS_TR_ISR_EXIT(isrnest_, prio_)  ((void)0)
#endif // QP_IMPL

#endif // QS_DUMMY_H_
