// PolynomialFunc.cpp
#include "PolynomialFunc.h"
#include <sstream>
#include <cmath>

PolynomialFunc::PolynomialFunc(const std::vector<double>& coefficients)
    : TFunction(
        [coefficients](double x) {
            double result = 0.0;
            double x_pow = 1.0;
            for (double coef : coefficients) {
                result += coef * x_pow;
                x_pow *= x;
            }
            return result;
        },
        [coefficients](double x) {
            double result = 0.0;
            double x_pow = 1.0;
            for (size_t i = 1; i < coefficients.size(); ++i) {
                result += i * coefficients[i] * x_pow;
                x_pow *= x;
            }
            return result;
        },
        ""
    ),
    coefficients_(coefficients)
{
    std::ostringstream oss;
    bool first = true;
    for (size_t i = 0; i < coefficients_.size(); ++i) {
        double coef = coefficients_[i];
        if (coef != 0) {
            if (!first) {
                oss << " + ";
            }
            first = false;
            if (i == 0) {
                oss << coef;
            } else if (i == 1) {
                oss << coef << "*x";
            } else {
                oss << coef << "*x^" << i;
            }
        }
    }
    if (first) {
        oss << "0";
    }
    str_ = oss.str();
}