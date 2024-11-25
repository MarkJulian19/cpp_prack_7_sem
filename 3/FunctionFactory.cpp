// FunctionFactory.cpp
#include "FunctionFactory.h"
#include "IdentFunc.h"
#include "ConstFunc.h"
#include "PowerFunc.h"
#include "ExpFunc.h"
#include "PolynomialFunc.h"
#include <stdexcept>

TFunctionPtr FunctionFactory::Create(const std::string& type) {
    if (type == "ident") {
        return std::make_shared<IdentFunc>();
    } else if (type == "exp") {
        return std::make_shared<ExpFunc>();
    }
    throw std::invalid_argument("Invalid function type");
}

TFunctionPtr FunctionFactory::Create(const std::string& type, double param) {
    if (type == "const") {
        return std::make_shared<ConstFunc>(param);
    } else if (type == "power") {
        return std::make_shared<PowerFunc>(param);
    }
    throw std::invalid_argument("Invalid function type or parameters");
}

TFunctionPtr FunctionFactory::Create(const std::string& type, const std::vector<double>& params) {
    if (type == "polynomial") {
        return std::make_shared<PolynomialFunc>(params);
    }
    throw std::invalid_argument("Invalid function type or parameters");
}