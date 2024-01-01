// basic ET example:
// very simple C++ code under test (CUT)
//
#include "sum.hpp"

namespace CUT {

int Sum::add(int y) noexcept {
    m_sum += y;
    return m_sum;
}

} // namespace CUT
