#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "shape.h"

/* Rectangle's attributes... */
typedef struct {
    Shape super; /* inherited Shape */

    /* attributes specific to this class */
    uint16_t width;
    uint16_t height;
} Rectangle;

/* Rectangle's operations... */
void Rectangle_ctor(Rectangle * const me,
                    int16_t x0, int16_t y0,
                    uint16_t w0, uint16_t h0);
void Rectangle_draw(Rectangle const * const me);
uint32_t Rectangle_area(Rectangle const * const me);

#endif /* RECTANGLE_H */

