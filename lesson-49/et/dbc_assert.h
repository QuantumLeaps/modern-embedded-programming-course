/*============================================================================
* Design By Contract (DBC) for embedded C and C++
* GitHub: https://github.com/QuantumLeaps/DBC-for-embedded-C
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005 Quantum Leaps, <state-machine.com>.
*
* SPDX-License-Identifier: MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
============================================================================*/
#ifndef DBC_ASSERT_H_
#define DBC_ASSERT_H_

/*! @file
* @brief Memory-efficient Design by Contract (DBC) for embedded C and C++.
*
* @note
* The runtime checking of the DBC assertions can be disabled by defining
* the macro #DBC_DISABLE. However, it is generally **not** advisable to
* disable assertions, *especially* in the production code. Instead, the
* assertion fault handler DBC_fault_handler() should be very carefully
* designed and tested under all fault conditions.
*/

/* Active DbC macros -------------------------------------------------------*/
#ifndef DBC_DISABLE

/*! Define the user-specified module name for assertions in this file.
*
* @details
* Macro to be placed at the top of each C/C++ module to define the
* single instance of the module name string to be used in reporting
* assertions in this module. This macro takes the user-supplied parameter
* `name_`.
*
* @param[in] name_ string constant representing the module name
*
* @note
* This macro should **not** be terminated by a semicolon.
*/
#define DBC_MODULE_NAME(name_) \
    static char const DBC_module_name_[] = name_;

/*! General purpose assertion with user-specified ID number.
*
* @details
* Makes sure the `expr_` parameter is TRUE. Calls the DBC_fault_handler()
* callback if the `expr_` evaluates to FALSE. This assertion takes the
* user-supplied parameter `label_` to identify the location of this
* assertion within the module. This avoids the volatility of using line
* numbers, which change whenever a line of code is added or removed
* upstream from the assertion.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
* @param[in] expr_  Boolean expression to check
*
* @note
* The `expr_` expression is **not** evaluated if assertions are
* disabled with the ::DBC_DISABLE switch.
*/
#define DBC_ASSERT(label_, expr_) ((expr_) \
    ? ((void)0) : DBC_fault_handler(&DBC_module_name_[0], (label_)))

/*! General purpose assertion with user-specified ID number that
* evaluates the `expr_` expression even when assertions are disabled.
*
* @details
* Like the DBC_ASSERT() macro, except it **always** evaluates the
* `expr_` expression even when DBC assertions are disabled with the
* #DBC_DISABLE macro.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
* @param[in] expr_  Boolean expression to check
*/
#define DBC_ALLEGE(label_, expr_) DBC_ASSERT(label_, expr_)

/*! Assertion for a wrong path through the code
*
* @details
* Calls the DBC_fault_handler() callback if ever executed. This assertion
* takes the user-supplied parameter `id_` to identify the location of
* this assertion within the file. This avoids the volatility of using
* line numbers, which change whenever a line of code is added or removed
* upstream from the assertion.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
*/
#define DBC_ERROR(label_) DBC_fault_handler(&DBC_module_name_[0], (label_))

/*! Assertion for checking preconditions.
*
* @details
* Equivalent to DBC_ASSERT(), except the name provides a better
* documentation of the intention of this assertion.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
* @param[in] expr_  Boolean expression to check
*
* @note
* The `expr_` expression is **not** evaluated if assertions are
* disabled with the ::DBC_DISABLE switch.
*/
#define DBC_REQUIRE(label_, expr_)  DBC_ASSERT((label_), (expr_))

/*! Assertion for checking postconditions.
*
* @details
* Equivalent to DBC_ASSERT(), except the name provides a better
* documentation of the intention of this assertion.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
* @param[in] expr_  Boolean expression to check
*
* @note
* The `expr_` expression is **not** evaluated if assertions are
* disabled with the ::DBC_DISABLE switch.
*/
#define DBC_ENSURE(label_, expr_) DBC_ASSERT((label_), (expr_))

/*! Assertion for checking invariants.
*
* @details
* Equivalent to DBC_ASSERT(), except the name provides a better
* documentation of the intention of this assertion.
*
* @param[in] label_ numeric label of the assertion (unique within the module)
* @param[in] expr_  Boolean expression to check
*
* @note
* The `expr_` expression is **not** evaluated if assertions are
* disabled with the ::DBC_DISABLE switch.
*/
#define DBC_INVARIANT(label_, expr_)  DBC_ASSERT((label_), (expr_))

#ifndef DBC_NORETURN
#define DBC_NORETURN
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! DBC assertion fault handler.
*
* @details
* This is an application-specific callback function needs to be defined in
* the application to perform the clean system shutdown and perhaps a reset.
* The DBC_fault_handler() function is the last line of defense after the
* system failure and its implementation should be very **carefully**
* designed and **tested** under various fault conditions, including but
* not limited to: stack overflow, stack corruption, or calling
* DBC_fault_handler() from ISRs.

* @param[in] module name of the file/module in which the assertion failed
*                   (constant, zero-terminated C string)
* @param[in] label  unique label of the assertion within the module.
*                   This could be a line number or a user-defined label.
*
* @returns
* This callback function should **not return** (see #NORETURN),
* as continuation after an assertion failure does not make sense.
*
* @note
* It is typically a **bad idea** to implement DBC_fault_handler() as an
* endless loop that ties up the CPU. During debugging, DBC_fault_handler()
* is an ideal place to put a breakpoint.
*/
DBC_NORETURN void DBC_fault_handler(char const * module, int label);

#ifdef __cplusplus
}
#endif

/* Inactive DbC macros -----------------------------------------------------*/
#else

#define DBC_MODULE_NAME(dummy_)
#define DBC_ASSERT(label_, expr_)      ((void)0)
#define DBC_ERROR(label_)              ((void)0)
#define DBC_REQUIRE(label_, expr_)  ((void)0)
#define DBC_ENSURE(label_, expr_) ((void)0)
#define DBC_INVARIANT(label_, expr_)  ((void)0)
#define DBC_ALLEGE(label_, expr_)      ((void)(expr_))

#endif /* Inactive DBC macros */

#endif /* DBC_ASSERT_ */
