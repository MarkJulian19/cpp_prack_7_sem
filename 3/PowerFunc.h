// PowerFunc.h
#ifndef POWERFUNC_H
#define POWERFUNC_H

#include "TFunction.h"

class PowerFunc : public TFunction {
public:
    explicit PowerFunc(double exponent);
private:
    double exponent_;
};

#endif // POWERFUNC_H