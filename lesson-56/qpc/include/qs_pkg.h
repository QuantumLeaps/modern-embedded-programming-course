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
#ifndef QS_PKG_H_
#define QS_PKG_H_

//============================================================================
#define QS_BEGIN_PRE(rec_, qsId_) \
if (QS_fltCheck_((uint32_t)(rec_)>>5U, \
                 (uint32_t)1U << ((uint32_t)(rec_) & 0x1FU), (qsId_))) \
{ \
    QS_beginRec_((uint_fast8_t)(rec_));

#define QS_END_PRE()           QS_endRec_(); }

#define QS_U8_PRE(data_)       (QS_u8_raw_((uint8_t)(data_)))
#define QS_2U8_PRE(data1_, data2_) \
    (QS_2u8_raw_((uint8_t)(data1_), (uint8_t)(data2_)))
#define QS_U16_PRE(data_)      (QS_u16_raw_((uint16_t)(data_)))
#define QS_U32_PRE(data_)      (QS_u32_raw_((uint32_t)(data_)))
#define QS_STR_PRE(msg_)       (QS_str_raw_((msg_)))
#define QS_OBJ_PRE(obj_)       (QS_obj_raw_(obj_))
#define QS_SIG_PRE(sig_)       (QS_u16_raw_((uint16_t)(sig_)))

#if (!defined QS_FUN_PTR_SIZE || (QS_FUN_PTR_SIZE == 2U))
    #define QS_FUN_PRE(fun_)   (QS_u16_raw_((uint16_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 4U)
    #define QS_FUN_PRE(fun_)   (QS_u32_raw_((uint32_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 8U)
    #define QS_FUN_PRE(fun_)   (QS_u64_raw_((uint64_t)(fun_)))
#else
    #define QS_FUN_PRE(fun_)   (QS_u32_raw_((uint32_t)(fun_)))
#endif

//----------------------------------------------------------------------------
#if (!defined QF_EQUEUE_CTR_SIZE || (QF_EQUEUE_CTR_SIZE == 1U))
    #define QS_EQC_PRE(ctr_)   QS_u8_raw_((uint8_t)(ctr_))
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    #define QS_EQC_PRE(ctr_)   QS_u16_raw_((uint16_t)(ctr_))
#elif (QF_EQUEUE_CTR_SIZE == 4U)
    #define QS_EQC_PRE(ctr_)   QS_u32_raw_((uint32_t)(ctr_))
#endif

#if (!defined QF_EVENT_SIZ_SIZE || (QF_EVENT_SIZ_SIZE == 1U))
    #define QS_EVS_PRE(size_)  QS_u8_raw_((uint8_t)(size_))
#elif (QF_EVENT_SIZ_SIZE == 2U)
    #define QS_EVS_PRE(size_)  QS_u16_raw_((uint16_t)(size_))
#elif (QF_EVENT_SIZ_SIZE == 4U)
    #define QS_EVS_PRE(size_)  QS_u32_raw_((uint32_t)(size_))
#endif

#if (!defined QF_MPOOL_SIZ_SIZE || (QF_MPOOL_SIZ_SIZE == 1U))
    #define QS_MPS_PRE(size_)  QS_u8_raw_((uint8_t)(size_))
#elif (QF_MPOOL_SIZ_SIZE == 2U)
    #define QS_MPS_PRE(size_)  QS_u16_raw_((uint16_t)(size_))
#elif (QF_MPOOL_SIZ_SIZE == 4U)
    #define QS_MPS_PRE(size_)  QS_u32_raw_((uint32_t)(size_))
#endif

#if (!defined QF_MPOOL_CTR_SIZE || (QF_MPOOL_CTR_SIZE == 1U))
    #define QS_MPC_PRE(ctr_)   QS_u8_raw_((uint8_t)(ctr_))
#elif (QF_MPOOL_CTR_SIZE == 2U)
    #define QS_MPC_PRE(ctr_)   QS_u16_raw_((uint16_t)(ctr_))
#elif (QF_MPOOL_CTR_SIZE == 4U)
    #define QS_MPC_PRE(ctr_)   QS_u32_raw_((uint16_t)(ctr_))
#endif

#if (!defined QF_TIMEEVT_CTR_SIZE || (QF_TIMEEVT_CTR_SIZE == 1U))
    #define QS_TEC_PRE(ctr_)   QS_u8_raw_((uint8_t)(ctr_))
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    #define QS_TEC_PRE(ctr_)   QS_u16_raw_((uint16_t)(ctr_))
#elif (QF_TIMEEVT_CTR_SIZE == 4U)
    #define QS_TEC_PRE(ctr_)   QS_u32_raw_((uint32_t)(ctr_))
#endif

//----------------------------------------------------------------------------
//! QS received record types (RX channel)
typedef enum {
    QS_RX_INFO,           //!< query Target info (ver, config, tstamp)
    QS_RX_COMMAND,        //!< execute a user-defined command in the Target
    QS_RX_RESET,          //!< reset the Target
    QS_RX_TICK,           //!< call system clock tick in the Target
    QS_RX_PEEK,           //!< peek Target memory
    QS_RX_POKE,           //!< poke Target memory
    QS_RX_FILL,           //!< fill Target memory
    QS_RX_TEST_SETUP,     //!< test setup
    QS_RX_TEST_TEARDOWN,  //!< test teardown
    QS_RX_TEST_PROBE,     //!< set a Test-Probe in the Target
    QS_RX_GLB_FILTER,     //!< set global filters in the Target
    QS_RX_LOC_FILTER,     //!< set local  filters in the Target
    QS_RX_AO_FILTER,      //!< set local AO filter in the Target
    QS_RX_CURR_OBJ,       //!< set the "current-object" in the Target
    QS_RX_TEST_CONTINUE,  //!< continue a test after QS_TEST_PAUSE()
    QS_RX_QUERY_CURR,     //!< query the "current object" in the Target
    QS_RX_EVENT,          //!< inject an event to the Target
} QS_RxRecords;

#define QS_FRAME          ((uint8_t)0x7EU)
#define QS_ESC            ((uint8_t)0x7DU)
#define QS_ESC_XOR        ((uint8_t)0x20U)
#define QS_GOOD_CHKSUM    ((uint8_t)0xFFU)

//----------------------------------------------------------------------------
#define QS_INSERT_BYTE_(b_) \
    buf[head] = (b_);       \
    ++head;                 \
    if (head == end) {      \
        head = 0U;          \
    }

#define QS_INSERT_ESC_BYTE_(b_)                       \
    chksum = (uint8_t)(chksum + (b_));                \
    if (((b_) != QS_FRAME) && ((b_) != QS_ESC)) {     \
        QS_INSERT_BYTE_(b_)                           \
    }                                                 \
    else {                                            \
        QS_INSERT_BYTE_(QS_ESC)                       \
        QS_INSERT_BYTE_((uint8_t)((b_) ^ QS_ESC_XOR)) \
        ++QS_priv_.used;                              \
    }

#define QS_PTR2UNIT_CAST(T_, ptr_) ((T_)(ptr_))

#endif // QS_PKG_H_
