//============================================================================
// QP/C/C++ Real-Time Event Framework (RTEF)
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
#ifndef QSAFE_H_
#define QSAFE_H_

// QF-FuSa enabled ===========================================================
#ifndef Q_UNSAFE

#ifndef QF_CRIT_STAT
    #define QF_CRIT_STAT
#endif

#ifndef QF_CRIT_ENTRY
    #define QF_CRIT_ENTRY() ((void)0)
#endif

#ifndef QF_CRIT_EXIT
    #define QF_CRIT_EXIT() ((void)0)
#endif

#ifndef QF_CRIT_EST
    #define QF_CRIT_EST() ((void)0)
#endif

#define Q_DEFINE_THIS_MODULE(name_) \
    static char const Q_this_module_[] = name_;

#define Q_ASSERT_INCRIT(id_, expr_) \
    ((expr_) ? ((void)0) : Q_onError(&Q_this_module_[0], (id_)))

#define Q_ERROR_INCRIT(id_) \
    (Q_onError(&Q_this_module_[0], (id_)))

#define Q_ASSERT_ID(id_, expr_) do { \
    QF_CRIT_STAT \
    QF_CRIT_ENTRY(); \
    if (!(expr_)) Q_onError(&Q_this_module_[0], (id_)); \
    QF_CRIT_EXIT(); \
} while (false)

#define Q_ERROR_ID(id_) do { \
    QF_CRIT_EST(); \
    Q_onError(&Q_this_module_[0], (id_)); \
} while (false)

#define Q_ASSERT_LOCAL(id_, expr_) do { \
    if (!(expr_)) { \
        QF_CRIT_EST(); \
        Q_onError(&Q_this_module_[0], (id_)); \
    } \
} while (false)

// QF-FuSa disabled ==========================================================
#else

#define Q_DEFINE_THIS_MODULE(name_)
#define Q_ASSERT_INCRIT(id_, expr_)    ((void)0)
#define Q_ERROR_INCRIT(id_)            ((void)0)
#define Q_ASSERT_ID(id_, expr_)        ((void)0)
#define Q_ERROR_ID(id_)                ((void)0)
#define Q_ASSERT_LOCAL(id_, expr_)     ((void)0)

#endif // QF-FuSa disabled

//============================================================================
#define Q_DEFINE_THIS_FILE             Q_DEFINE_THIS_MODULE(__FILE__)
#define Q_ASSERT(expr_)                Q_ASSERT_ID(__LINE__, (expr_))
#define Q_ERROR()                      Q_ERROR_ID(__LINE__)

#define Q_REQUIRE(expr_)               Q_ASSERT(expr_)
#define Q_REQUIRE_ID(id_, expr_)       Q_ASSERT_ID((id_), (expr_))
#define Q_REQUIRE_INCRIT(id_, expr_)   Q_ASSERT_INCRIT((id_), (expr_))
#define Q_REQUIRE_LOCAL(id_, expr_)    Q_ASSERT_LOCAL((id_), (expr_))

#define Q_ENSURE(expr_)                Q_ASSERT(expr_)
#define Q_ENSURE_ID(id_, expr_)        Q_ASSERT_ID((id_), (expr_))
#define Q_ENSURE_INCRIT(id_, expr_)    Q_ASSERT_INCRIT((id_), (expr_))
#define Q_ENSURE_LOCAL(id_, expr_)     Q_ASSERT_LOCAL((id_), (expr_))

#define Q_INVARIANT(expr_)             Q_ASSERT(expr_)
#define Q_INVARIANT_ID(id_, expr_)     Q_ASSERT_ID((id_), (expr_))
#define Q_INVARIANT_INCRIT(id_, expr_) Q_ASSERT_INCRIT((id_), (expr_))
#define Q_INVARIANT_LOCAL(id_, expr_)  Q_ASSERT_LOCAL((id_), (expr_))

#define Q_ERROR_LOCAL(id_)             Q_ERROR_ID(id_)

#ifndef Q_ASSERT_STATIC
    #define Q_ASSERT_STATIC(expr_) extern char Q_static_assert_[(expr_) ? 1 : -1]
#endif // ndef Q_ASSERT_STATIC

#ifndef Q_NORETURN
    #define Q_NORETURN _Noreturn void
#endif // ndef Q_NORETURN

// Is this header file used outside QP?
#ifndef QP_VERSION
    #define Q_DIM(array_) (sizeof(array_) / sizeof((array_)[0U]))
#endif // ndef QP_VERSION

//============================================================================
#ifdef __cplusplus
extern "C" {
#endif

#ifndef QP_VERSION
    typedef int int_t;
#endif // ndef QP_VERSION

Q_NORETURN Q_onError(
    char const * const module,
    int_t const id);

#ifdef __cplusplus
}
#endif

#endif // QSAFE_H_
