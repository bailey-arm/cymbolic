#include <iostream>
#include "cymbolic/expr.hpp"
using namespace cymbolic;

int main() {
    auto x = var("x");
    auto expr = sin(x * x);

    Env env{{"x", 1.5}};
    std::cout << expr->eval(env) << std::endl;
    std::cout << expr->diff("x")->eval(env) << std::endl;

    return 0;
}
