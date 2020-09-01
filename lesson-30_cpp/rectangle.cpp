#include "rectangle.h" /* Rectangle class interface */

Rectangle::Rectangle(int16_t x0, int16_t y0,
                     uint16_t w0, uint16_t h0)
: Shape(x0, y0), /* base class ctor */

  /* init attributes added in this class */
  width(w0),
  height(h0)
{
}

void Rectangle::draw() const {
    //drawHorLine(x, y, width);
    //drawVerLine(x + width,
    //            y, height);
    //drawHorLine(x + width,
    //            y + height, width);
    //drawVerLine(x, y, height);
}

uint32_t Rectangle::area() const {
    return (uint32_t)width * (uint32_t)height;
}
