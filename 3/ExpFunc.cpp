// ExpFunc.cpp
#include "ExpFunc.h"
#include <cmath>

ExpFunc::ExpFunc()
    : TFunction(
        [](double x) { return std::exp(x); },
        [](double x) { return std::exp(x); },
        "exp(x)"
    ) {}