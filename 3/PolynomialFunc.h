// PolynomialFunc.h
#ifndef POLYNOMIALFUNC_H
#define POLYNOMIALFUNC_H

#include "TFunction.h"
#include <vector>

class PolynomialFunc : public TFunction {
public:
    explicit PolynomialFunc(const std::vector<double>& coefficients);
private:
    std::vector<double> coefficients_;
};

#endif // POLYNOMIALFUNC_H