#include "shape.hpp"
#include "qassert.h" // for Q_ASSERT

Q_DEFINE_THIS_FILE // file name for assertions

Shape s1(1, 2); // static allocation

int main(void) {
    Shape s2(3, 4); // automatic allocation
    Shape *ps3 = new Shape(5, 6); // dynamic allocation
    //Shape const *ps1 = &s1; // const pointer to s1

    s1.moveBy(7, 8);
    s2.moveBy(9, 10);
    ps3->moveBy(-1, -2);
    //ps1->moveBy(-3, -4);

    Q_ASSERT(s1.distanceFrom(&s1) == 0U);
    Q_ASSERT(s1.distanceFrom(&s2) ==
             s2.distanceFrom(&s1));
    Q_ASSERT(s1.distanceFrom(&s2) <=
             s1.distanceFrom(ps3)
           + ps3->distanceFrom(&s2));

    delete ps3;

    while (1) {
    }

    //return 0; // unreachable code
}

//............................................................................
extern "C" {
void Q_onAssert(char const *module, int loc) {
    // TBD: damage control
    (void)module; // avoid the "unused parameter" compiler warning
    (void)loc;    // avoid the "unused parameter" compiler warning
    while (1) {   // hang in an endless loop
    }
}

}  // extern "C"

