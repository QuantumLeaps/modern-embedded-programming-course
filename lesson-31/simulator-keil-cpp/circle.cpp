#include "circle.hpp" // Circle class interface

Circle::Circle(std::int16_t x0, std::int16_t y0,
               std::uint16_t r0)
: Shape(x0, y0), // base class ctor

  // init attributes added in this class
  radius(r0)
{
}

void Circle::draw() const {
    //drawEllipse(x, y, radius, radius);
}

uint32_t Circle::area() const {
    return 3U * static_cast<std::uint32_t>(radius)
              * static_cast<std::uint32_t>(radius);
}
