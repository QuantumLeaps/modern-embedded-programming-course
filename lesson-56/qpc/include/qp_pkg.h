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
#ifndef QP_PKG_H_
#define QP_PKG_H_

#ifdef QP_IMPL

// helper macros...
#define QACTIVE_CAST_(ptr_)     ((QActive *)(ptr_))

//! @static @private @memberof QActive
extern QActive * QActive_registry_[QF_MAX_ACTIVE + 1U];

//! @static @private @memberof QActive
extern QSubscrList * QActive_subscrList_;

//! @static @private @memberof QActive
extern QSignal QActive_maxPubSignal_;

#if (QF_MAX_TICK_RATE > 0U)
//! @static @private @memberof QTimeEvt
extern QTimeEvt QTimeEvt_timeEvtHead_[QF_MAX_TICK_RATE];


// Bitmasks are for the QTimeEvt::flags attribute
#define QTE_FLAG_IS_LINKED      (1U << 7U)
#define QTE_FLAG_WAS_DISARMED   (1U << 6U)

#endif // (QF_MAX_TICK_RATE > 0U)

//----------------------------------------------------------------------------
typedef struct {
#if (QF_MAX_EPOOL > 0U)
    QF_EPOOL_TYPE_ ePool_[QF_MAX_EPOOL]; //!< @private @memberof QF_Attr
    uint8_t maxPool_;                    //!< @private @memberof QF_Attr
#else
    uint8_t dummy;                       //!< @private @memberof QF_Attr
#endif // (QF_MAX_EPOOL == 0U)
} QF_Attr;

extern QF_Attr QF_priv_; //!< @static @private @memberof QF

//----------------------------------------------------------------------------
// Duplicate Inverse Storage (DIS) facilities

#ifndef Q_UNSAFE

#define QP_DIS_UPDATE(T_, org_)        ((T_)(~(org_)))
#define QP_DIS_VERIFY(T_, org_, dis_)  ((org_) == (T_)(~(dis_)))

#endif // Q_UNSAFE

#endif // QP_IMPL

#endif // QP_PKG_H_
