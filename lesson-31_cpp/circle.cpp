#include "circle.h" /* Circle class interface */

Circle::Circle(int16_t x0, int16_t y0,
                     uint16_t r0)
: Shape(x0, y0), /* base class ctor */

  /* init attributes added in this class */
  radius(r0)
{
}

void Circle::draw() const {
    //drawEllipse(x, y, radius, radius);
}

uint32_t Circle::area() const {
    return 3U * (uint32_t)radius * (uint32_t)radius;
}
