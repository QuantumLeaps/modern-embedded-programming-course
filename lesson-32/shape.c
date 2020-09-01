#include "shape.h" /* Shape class interface */

static void Shape_draw(Shape const * const me);
static uint32_t Shape_area(Shape const * const me);

void Shape_ctor(Shape * const me, int16_t x0, int16_t y0) {
    static const struct ShapeVtable vtable = {
        &Shape_draw,
        &Shape_area
    };
    me->vptr = &vtable;
    me->x = x0;
    me->y = y0;
}

void Shape_moveBy(Shape * const me, int16_t dx, int16_t dy) {
    me->x += dx;
    me->y += dy;
}

uint16_t Shape_distanceFrom(Shape const * const me,
                            Shape const * other)
{
    int16_t dx = me->x - other->x;
    int16_t dy = me->y - other->y;
    if (dx < 0) {
       dx = -dx;
    }
    if (dy < 0) {
       dy = -dy;
    }
    return dx + dy;
}

static void Shape_draw(Shape const * const me) {
    (void)me; /* unused parameter */
}

static uint32_t Shape_area(Shape const * const me) {
    (void)me; /* unused parameter */
    return 0U;
}

/* draw all shapes in a given array graph[] */
void drawGraph(Shape const *graph[]) {
    uint8_t i;
    for (i = 0; graph[i] != (Shape *)0; ++i) {
        Shape_draw_vcall(graph[i]); /* polymorphism! */
    }
}

