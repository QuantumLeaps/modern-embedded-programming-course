#include "rectangle.hpp" // Rectangle class interface

Rectangle::Rectangle(std::int16_t x0, std::int16_t y0,
                     std::uint16_t w0, std::uint16_t h0)
: Shape(x0, y0), // base class ctor
  // init attributes added in this class
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
    return static_cast<std::uint32_t>(width)
           * static_cast<std::uint32_t>(height);
}
