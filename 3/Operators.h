#ifndef OPERATORS_H
#define OPERATORS_H

#include "TFunction.h"
#include <stdexcept>
#include <memory>
#include <type_traits>

// Операторы для TFunction
TFunction operator+(const TFunction& lhs, const TFunction& rhs);
TFunction operator-(const TFunction& lhs, const TFunction& rhs);
TFunction operator*(const TFunction& lhs, const TFunction& rhs);
TFunction operator/(const TFunction& lhs, const TFunction& rhs);

// Шаблонные операторы для проверки типов
template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator+(const TFunction& lhs, const T& rhs) {
    throw std::logic_error("Invalid operand type for operator+");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator+(const T& lhs, const TFunction& rhs) {
    throw std::logic_error("Invalid operand type for operator+");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator-(const TFunction& lhs, const T& rhs) {
    throw std::logic_error("Invalid operand type for operator-");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator-(const T& lhs, const TFunction& rhs) {
    throw std::logic_error("Invalid operand type for operator-");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator*(const TFunction& lhs, const T& rhs) {
    throw std::logic_error("Invalid operand type for operator*");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator*(const T& lhs, const TFunction& rhs) {
    throw std::logic_error("Invalid operand type for operator*");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator/(const TFunction& lhs, const T& rhs) {
    throw std::logic_error("Invalid operand type for operator/");
}

template<typename T>
typename std::enable_if<!std::is_base_of<TFunction, T>::value, TFunction>::type
operator/(const T& lhs, const TFunction& rhs) {
    throw std::logic_error("Invalid operand type for operator/");
}

#endif // OPERATORS_H
