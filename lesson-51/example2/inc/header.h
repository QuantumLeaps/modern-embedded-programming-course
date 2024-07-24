#ifndef HEADER_H_
#define HEADER_H_

#include <stdint.h>
#include <stdbool.h>

/*!
@brief Free function brief description
@par Details
Free function longer description.
@param[in]  x  - the parameter x
@returns pointer to a static array
@par Backward Traceability
- @ref SRS_EXA_FF_00
*/
uint8_t const *free_fun(uint32_t x);

/**
@brief Class Foo brief description
@par Details
Class Foo longer description.
@par Backward Traceability
- @ref SRS_EXA_Foo_00
*/
typedef struct Foo {
    //! @brief Attribute x of class Foo brief description
    //! @par Details
    //! Attribute x of class Foo: longer description.
    //! @par Backward Traceability
    //! - @ref SRS_EXA_Foo_01
    uint32_t x;

    /// \brief Foo::x_dis, Duplicate Inverse Storage for attribute Foo::x
    /// \par Details
    /// Duplicate Inverse Storage (DIS) for attribute Foo::x: longer description.
    /// \par Backward Traceability
    /// - @ref Foo::x
    /// - @ref Foo_verify_()
    uint32_t x_dis;
} Foo;

/*!
@brief Constructor of class Foo brief description
@par Details
Constructor of class Foo longer description.
@param[in]  me - the instance pointer (OOP in C)
@param[in]  x  - the initial value for me->x
@par Backward Traceability
- @ref SRS_EXA_Foo_02
- @ref Foo
*/
void Foo_ctor(Foo * const me, uint32_t const x);

/*!
@brief Verify operation to check the class invariant
@par Details
Operation verify_() of class Foo longer description.
@param[in]  me - the instance pointer (OOP in C)
@returns 'true' when the Foo instance verification succeeds, 'false' otherwise.
@par Backward Traceability
- @ref SRS_EXA_Foo_03
- @ref Foo
*/
bool Foo_verify_(Foo const * const me);

/*!
@brief Update operation to update the class invariant
@par Details
Constructor of class Foo longer description.
@param[in]  me - the instance pointer (OOP in C)
@par Backward Traceability
- @ref SRS_EXA_Foo_04
- @ref Foo
*/
void Foo_update_(Foo* const me);

/*!
@brief Foo instance brief description (singleton)
@par Details
Foo instance longer description.
@par Backward Traceability
- @ref SRS_EXA_Foo_04
- @ref Foo
*/
extern Foo const Foo_inst;

#endif // HEADER_H_
