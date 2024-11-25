// FunctionFactory.h
#ifndef FUNCTIONFACTORY_H
#define FUNCTIONFACTORY_H

#include "TFunction.h"
#include <vector>

class FunctionFactory {
public:
    TFunctionPtr Create(const std::string& type);
    TFunctionPtr Create(const std::string& type, double param);
    TFunctionPtr Create(const std::string& type, const std::vector<double>& params);
};

#endif // FUNCTIONFACTORY_H