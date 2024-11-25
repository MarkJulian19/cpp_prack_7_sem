// ConstFunc.cpp
#include "ConstFunc.h"
#include <sstream>

ConstFunc::ConstFunc(double value)
    : TFunction(
        [value](double) { return value; },
        [](double) { return 0.0; },
        ""
    ),
    value_(value)
{
    std::ostringstream oss;
    oss << value_;
    str_ = oss.str();
}