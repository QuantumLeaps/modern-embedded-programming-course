// basic ET example:
// very simple C++ code under test (CUT)
//
#ifndef SUM_HPP_
#define SUM_HPP_

namespace CUT {

class Sum {
public:
    Sum(int sum) : m_sum(sum) {} // ctor
    void clear(void) { m_sum = 0; }
    int get(void) noexcept { return m_sum; }
    int add(int y) noexcept;
private:
    int m_sum;
};

} // namespace CUT

#endif // SUM_HPP_
