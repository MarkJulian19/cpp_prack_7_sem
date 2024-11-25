#include <gtest/gtest.h>
#include "FunctionFactory.h"
#include "Operators.h"
#include "GradientDescent.h"

TEST(FunctionCreationTest, CreateBasicFunctions) {
    FunctionFactory factory;
    auto ident = factory.Create("ident");
    EXPECT_EQ((*ident)(5), 5);
    EXPECT_EQ(ident->GetDeriv(5), 1);

    auto constant = factory.Create("const", 10);
    EXPECT_EQ((*constant)(5), 10);
    EXPECT_EQ(constant->GetDeriv(5), 0);

    auto power = factory.Create("power", 2);
    EXPECT_EQ((*power)(3), 9);
    EXPECT_EQ(power->GetDeriv(3), 6);

    auto expFunc = factory.Create("exp");
    EXPECT_DOUBLE_EQ((*expFunc)(0), 1);
    EXPECT_DOUBLE_EQ(expFunc->GetDeriv(0), 1);

    auto poly = factory.Create("polynomial", std::vector<double>{1, 2, 3}); // 1 + 2x + 3x^2
    EXPECT_EQ((*poly)(1), 6);
    EXPECT_EQ(poly->GetDeriv(1), 8);
}

TEST(FunctionOperatorTest, ArithmeticOperations) {
    FunctionFactory factory;
    auto f = factory.Create("power", 2); // x^2
    auto g = factory.Create("const", 3); // 3

    auto sum = *f + *g;
    EXPECT_EQ(sum(2), 7);
    EXPECT_EQ(sum.GetDeriv(2), 4);

    auto product = *f * *g;
    EXPECT_EQ(product(2), 12);
    EXPECT_EQ(product.GetDeriv(2), 12);

    EXPECT_THROW(*f + std::string("abc"), std::logic_error);
}

TEST(FunctionDerivativeTest, DerivativeCalculation) {
    FunctionFactory factory;
    auto poly = factory.Create("polynomial", std::vector<double>{0, 0, 1}); // x^2
    EXPECT_EQ(poly->GetDeriv(3), 6);
}

TEST(GradientDescentTest, FindRoot) {
    FunctionFactory factory;
    auto poly = factory.Create("polynomial", std::vector<double>{-4, 0, 1}); // x^2 - 4
    double root = FindRootByGradientDescent(*poly, 1.0, 0.1, 100);
    EXPECT_NEAR(root, 2.0, 0.01);
}
