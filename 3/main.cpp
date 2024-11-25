#include <iostream>
#include "TFunction.h"
#include "FunctionFactory.h"
#include "Operators.h"

int main() {
    FunctionFactory funcFactory;
    std::vector<TFunctionPtr> cont;

    auto f = funcFactory.Create("power", 2); // PowerFunc x^2
    cont.push_back(f);

    auto g = funcFactory.Create("polynomial", std::vector<double>{7, 0, 3, 15}); // 7 + 3*x^2 + 15*x^3
    cont.push_back(g);

    for (const auto& ptr : cont) {
        std::cout << ptr->ToString() << " for x = 10 is " << (*ptr)(10) << std::endl;
    }

    auto p = *f + *g;
    std::cout << p.ToString() << " derivative at x=1: " << p.GetDeriv(1) << std::endl; // 53

    try {
        auto h = *f + std::string("abc"); // std::logic_error
    } catch (const std::logic_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "Derivative of f at x=3: " << f->GetDeriv(3) << std::endl; // 6

    return 0;
}
