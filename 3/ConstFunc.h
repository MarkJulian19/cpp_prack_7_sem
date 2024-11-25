// ConstFunc.h
#ifndef CONSTFUNC_H
#define CONSTFUNC_H

#include "TFunction.h"

class ConstFunc : public TFunction {
public:
    explicit ConstFunc(double value);
private:
    double value_;
};

#endif // CONSTFUNC_H