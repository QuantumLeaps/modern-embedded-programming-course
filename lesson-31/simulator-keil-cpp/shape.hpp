#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <cstdint>  // standard integer types

class Shape {
private:    // Shape's attributes...
    std::int16_t x; // x-coordinate of Shape's position
    std::int16_t y; // y-coordinate of Shape's position

public:    // Shape's operations...
    Shape(std::int16_t x0, std::int16_t y0);
    void moveBy(std::int16_t dx, std::int16_t dy);
    uint16_t distanceFrom(Shape const * other) const;

    virtual void draw() const;
    virtual uint32_t area() const;
};

void drawGraph(Shape const *graph[]);

#endif // SHAPE_HPP_
