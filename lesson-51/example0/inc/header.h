#ifndef HEADER_H_
#define HEADER_H_

#include <stdint.h>
#include <stdbool.h>

uint8_t const *free_fun(uint32_t x);

typedef struct Foo {
    uint32_t x;
    uint32_t x_dis;
} Foo;

void Foo_ctor(Foo * const me, uint32_t const x);
bool Foo_verify_(Foo const * const me);
void Foo_update_(Foo* const me);

extern Foo const Foo_inst;

#endif // HEADER_H_
