#pragma once

#include <cstdint>

namespace iv_calculator::core {
    /**
     * @brief Enumeration of numerical methods for implied volatility calculation
     */
    enum class ImpliedVolatilityMethod : std::uint8_t {
        BISECTION,      ///< Bisection method (robust but slower)
        NEWTON_RAPHSON  ///< Newton-Raphson method (faster but less robust)
    };

    /**
     * @brief Calculate option price using Black-Scholes model
     *
     * @param is_call True for Call option, False for Put option
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param sigma Volatility of the underlying asset
     * @return double Option price
     */
    double black_scholes_price(bool is_call, double S, double K, double T, double r, double sigma);

    /**
     * @brief Calculate option's vega (sensitivity to volatility)
     *
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param sigma Volatility of the underlying asset
     * @return double Option's vega value
     */
    double black_scholes_vega(double S, double K, double T, double r, double sigma);

    /**
     * @brief Calculate implied volatility using numerical methods
     *
     * @param is_call True for Call option, False for Put option
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param option_price Market price of the option
     * @param method Numerical method to use (default: BISECTION)
     * @return double Implied volatility
     */
    double calculate_implied_volatility(
        bool is_call, double S, double K, double T, double r, double option_price,
        ImpliedVolatilityMethod method = ImpliedVolatilityMethod::BISECTION);

    /**
     * @brief Calculate implied volatility using bisection method
     *
     * @param is_call True for Call option, False for Put option
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param option_price Market price of the option
     * @return double Implied volatility
     */
    double bisection_implied_volatility(bool is_call, double S, double K, double T, double r,
                                        double option_price);

    /**
     * @brief Calculate implied volatility using Newton-Raphson method
     *
     * @param is_call True for Call option, False for Put option
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param option_price Market price of the option
     * @return double Implied volatility
     */
    double newton_raphson_implied_volatility(bool is_call, double S, double K, double T, double r,
                                             double option_price);
}  // namespace iv_calculator::core
