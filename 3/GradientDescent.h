// GradientDescent.h
#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include "TFunction.h"

double FindRootByGradientDescent(const TFunction& func, double initialGuess, double learningRate, int iterations);

#endif // GRADIENTDESCENT_H