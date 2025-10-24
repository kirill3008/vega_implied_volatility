#pragma once

namespace iv_calculator::core {
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
     * @brief Calculate implied volatility using numerical methods
     *
     * @param is_call True for Call option, False for Put option
     * @param S Current price of the underlying asset
     * @param K Strike price
     * @param T Time to expiration in years
     * @param r Risk-free interest rate
     * @param option_price Market price of the option
     * @return double Implied volatility
     */
    double calculate_implied_volatility(bool is_call, double S, double K, double T, double r,
                                        double option_price);
}  // namespace iv_calculator::core
