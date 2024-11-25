#include "TFunction.h"
#include <stdexcept>

TFunction::TFunction()
    : func_(nullptr), deriv_(nullptr), str_("") {}

TFunction::TFunction(FuncType func, FuncType deriv, std::string str)
    : func_(func), deriv_(deriv), str_(str) {}

double TFunction::operator()(double x) const {
    if (func_) {
        return func_(x);
    }
    throw std::logic_error("Function not defined");
}

double TFunction::GetDeriv(double x) const {
    if (deriv_) {
        return deriv_(x);
    }
    throw std::logic_error("Derivative not defined");
}

std::string TFunction::ToString() const {
    return str_;
}
