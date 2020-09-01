#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "shape.h"


class Rectangle : public Shape { /* inherited Shape */
private: /* Rectangle's attributes... */
    /* attributes specific to this class */
    uint16_t width;
    uint16_t height;

public:  /* Rectangle's operations... */
    Rectangle(int16_t x0, int16_t y0,
              uint16_t w0, uint16_t h0);
    virtual void draw() const;
    virtual uint32_t area() const;
};

#endif /* RECTANGLE_H */

