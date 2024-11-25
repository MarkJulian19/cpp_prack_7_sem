// PowerFunc.cpp
#include "PowerFunc.h"
#include <cmath>
#include <sstream>

PowerFunc::PowerFunc(double exponent)
    : TFunction(
        [exponent](double x) { return std::pow(x, exponent); },
        [exponent](double x) { return exponent * std::pow(x, exponent - 1); },
        ""
    ),
    exponent_(exponent)
{
    std::ostringstream oss;
    if (exponent_ == 1.0) {
        str_ = "x";
    } else {
        oss << "x^" << exponent_;
        str_ = oss.str();
    }
}