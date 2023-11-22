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

void Shape::draw() const {
}

uint32_t Shape::area() const {
    return 0U;
}

/* draw all shapes in a given array graph[] */
void drawGraph(Shape const *graph[]) {
    for (std::uint8_t i = 0; graph[i] != (Shape *)0; ++i) {
        graph[i]->draw(); /* polymorphism! */
    }
}

