#ifndef RECTANGLE_HPP_
#define RECTANGLE_HPP_

#include "shape.hpp"

class Rectangle : public Shape { // inherited Shape
private: // Rectangle's attributes...
    // attributes specific to this class
    std::uint16_t width;
    std::uint16_t height;

public:  // Rectangle's operations...
    Rectangle(std::int16_t x0, std::int16_t y0,
              std::uint16_t w0, std::uint16_t h0);
    void draw() const;
    uint32_t area() const;
};

#endif // RECTANGLE_HPP_

