#include "shape.h" /* Shape class interface */

Shape::Shape(int16_t x0, int16_t y0)
: x(x0),
  y(y0)
{
}

void Shape::moveBy(int16_t dx, int16_t dy) {
    this->x += dx;
    this->y += dy;
}

uint16_t Shape::distanceFrom(Shape const * other) const
{
    int16_t dx = x - other->x;
    int16_t dy = y - other->y;
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
    return (uint32_t)0;
}

/* draw all shapes in a given array graph[] */
void drawGraph(Shape const *graph[]) {
    uint8_t i;
    for (i = 0; graph[i] != (Shape *)0; ++i) {
        graph[i]->draw(); /* polymorphism! */
    }
}
