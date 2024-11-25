#ifndef TFUNCTION_H
#define TFUNCTION_H

#include <functional>
#include <memory>
#include <string>

class TFunction {
public:
    using FuncType = std::function<double(double)>;

    TFunction();
    TFunction(FuncType func, FuncType deriv, std::string str);

    virtual double operator()(double x) const;
    virtual double GetDeriv(double x) const;
    virtual std::string ToString() const;

protected:
    FuncType func_;
    FuncType deriv_;
    std::string str_;
};

using TFunctionPtr = std::shared_ptr<TFunction>;

#endif // TFUNCTION_H
