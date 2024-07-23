#include "header.h"

uint8_t const * free_fun(uint32_t x) {
    static uint8_t arr[10];
    a[0] = (uint8_t)x;
    a[1] = (uint8_t)(x >> 8U);
    a[2] = (uint8_t)(x >> 16U);
    a[3] = (uint8_t)(x >> 24U);
    return arr;
}

void Foo_ctor(Foo * const me, uint32_t const x) {
    me->x = x;
    Foo_update_(me);
}

void Foo_update_(Foo* const me) {
    me->x_dis = ~me->x;
}

bool Foo_verify_(Foo const* const me) {
    return me->x_dis == ~me->x;
}
