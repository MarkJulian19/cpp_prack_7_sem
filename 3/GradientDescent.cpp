// GradientDescent.cpp
#include "GradientDescent.h"
#include <stdexcept>

double FindRootByGradientDescent(const TFunction& func, double initialGuess, double learningRate, int iterations) {
    double x = initialGuess;
    for (int i = 0; i < iterations; ++i) {
        double y = func(x);
        double dydx = func.GetDeriv(x);
        if (dydx == 0.0) {
            throw std::runtime_error("Zero derivative encountered during gradient descent");
        }
        x = x - learningRate * y / dydx;
    }
    return x;
}