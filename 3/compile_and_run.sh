#! /bin/bash
g++ main.cpp TFunction.cpp IdentFunc.cpp ConstFunc.cpp PowerFunc.cpp ExpFunc.cpp PolynomialFunc.cpp FunctionFactory.cpp Operators.cpp GradientDescent.cpp --std=c++17 -O2 -o main
./main