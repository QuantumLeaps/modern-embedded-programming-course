#include "shape.hpp" // Shape class interface

Shape::Shape(std::int16_t x0, std::int16_t y0)
: x(x0),
  y(y0)
{
}

void Shape::moveBy(std::int16_t dx, std::int16_t dy) {
    this->x += dx;
    this->y += dy;
}

uint16_t Shape::distanceFrom(Shape const * other) const
{
    std::int16_t dx = x - other->x;
    std::int16_t dy = y - other->y;
    if (dx < 0) {
       dx = -dx;
    }
    if (dy < 0) {
       dy = -dy;
    }
    return dx + dy;
}
