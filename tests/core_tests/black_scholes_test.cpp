#include "src/core/black_scholes.h"

#include <gtest/gtest.h>

using namespace iv_calculator::core;

TEST(BlackScholesTest, CallOptionPricing) {
    // Test case with known values
    double price = black_scholes_price(true, 100.0, 100.0, 1.0, 0.05, 0.2);
    EXPECT_NEAR(price, 10.45, 0.01);
}

TEST(BlackScholesTest, PutOptionPricing) {
    // Test case with known values
    double price = black_scholes_price(false, 100.0, 100.0, 1.0, 0.05, 0.2);
    EXPECT_NEAR(price, 5.57, 0.01);
}

TEST(BlackScholesTest, ImpliedVolatility) {
    // Calculate implied volatility from a price
    double sigma = calculate_implied_volatility(true, 100.0, 100.0, 1.0, 0.05, 10.45);
    EXPECT_NEAR(sigma, 0.2, 0.001);
}

TEST(BlackScholesTest, InvalidInputs) {
    // Test that invalid inputs throw exceptions
    EXPECT_THROW(black_scholes_price(true, -100.0, 100.0, 1.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(black_scholes_price(true, 100.0, 0.0, 1.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(black_scholes_price(true, 100.0, 100.0, 0.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(black_scholes_price(true, 100.0, 100.0, 1.0, 0.05, -0.2), std::invalid_argument);
}
