#ifndef SHAPE_H
#define SHAPE_H

#include <stdint.h>

class Shape {
protected:    /* Shape's attributes... */
    int16_t x; /* x-coordinate of Shape's position */
    int16_t y; /* y-coordinate of Shape's position */

public:    /* Shape's operations... */
    Shape(int16_t x0, int16_t y0);
    void moveBy(int16_t dx, int16_t dy);
    uint16_t distanceFrom(Shape const * other) const;
};

#endif /* SHAPE_H */
