#include <type_traits>
#include <typeinfo>
#include <fmt/core.h>

// std::decay_t<decltype> is used to strip away any unnecessary qualifiers or references from a given type.
// std::common_type_t is used to compute the common type of a set of values.
template <typename T1, typename T2,
    // typename RT = std::decay_t<decltype(true ? T1() : T2())>>
    typename RT = std::common_type_t<T1, T2>>
RT max (T1 a, T2 b) {
    return b < a ? a : b;
}

int main() {
    auto x = ::max(1, 2.1); // x has type double
    auto y = ::max(1.0, 2); // y has type double
    auto z = ::max(1, 2); // z has type int

    static_assert(std::is_same_v<decltype(x), double>);
    static_assert(std::is_same_v<decltype(y), double>);
    static_assert(std::is_same_v<decltype(z), int>);

    fmt::print("x: {}, y: {}, z: {}\n", x, y, z);
    return 0;
}