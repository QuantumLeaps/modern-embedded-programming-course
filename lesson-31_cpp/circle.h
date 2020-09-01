#ifndef CIRCLE_H
#define CIRCLE_H

#include "shape.h"


class Circle : public Shape { /* inherited Shape */
private: /* Circle's attributes... */
    /* attributes specific to this class */
    uint16_t radius;

public:  /* Circle's operations... */
    Circle(int16_t x0, int16_t y0,
              uint16_t r0);
    virtual void draw() const;
    virtual uint32_t area() const;
};

#endif /* CIRCLE_H */

