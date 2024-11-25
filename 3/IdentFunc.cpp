// IdentFunc.cpp
#include "IdentFunc.h"

IdentFunc::IdentFunc()
    : TFunction(
        [](double x) { return x; },
        [](double x) { return 1.0; },
        "x"
    ) {}