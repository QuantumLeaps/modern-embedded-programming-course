#ifndef CIRCLE_HPP_
#define CIRCLE_HPP_

#include "shape.hpp"

class Circle : public Shape { // inherit Shape
private: // Circle's attributes...
    // attributes specific to this class
    std::uint16_t radius;

public:  // Circle's operations...
    Circle(std::int16_t x0, std::int16_t y0,
           std::uint16_t r0);

    void draw() const override;
    uint32_t area() const override;
};

#endif // CIRCLE_HPP_

