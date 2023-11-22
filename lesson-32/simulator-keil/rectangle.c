#include "rectangle.h" /* Rectangle class interface */

void Rectangle_ctor(Rectangle * const me,
                    int16_t x0, int16_t y0,
                    uint16_t w0, uint16_t h0)
{
    static const struct ShapeVtable vtable = {
        (void (*)(Shape const * const me))&Rectangle_draw,
        (uint32_t (*)(Shape const * const me))&Rectangle_area
    };

    Shape_ctor(&me->super, x0, y0); /* base class ctor */
    me->super.vptr = &vtable;

    /* init attributes added in this class */
    me->width = w0;
    me->height = h0;
}

void Rectangle_draw(Rectangle const * const me) {
    //drawHorLine(me->super.x, me->super.y, me->width);
    //drawVerLine(me->super.x + me->width,
    //            me->super.y, me->height);
    //drawHorLine(me->super.x + me->width,
    //            me->super.y + me->height, me->width);
    //drawVerLine(me->super.x, me->super.y, me->height);
}

uint32_t Rectangle_area(Rectangle const * const me) {
    return (uint32_t)me->width * (uint32_t)me->height;
}
