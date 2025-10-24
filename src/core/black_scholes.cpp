#include "black_scholes.h"

#include <cmath>
#include <stdexcept>

namespace iv_calculator {
    namespace core {
        // Standard normal cumulative distribution function
        double norm_cdf(double x) { return 0.5 * (1 + std::erf(x / std::sqrt(2))); }

        double black_scholes_price(bool is_call, double S, double K, double T, double r,
                                   double sigma) {
            // Input validation
            if (S <= 0 || K <= 0 || T <= 0 || sigma < 0) {
                throw std::invalid_argument("Invalid input parameters");
            }

            // Black-Scholes formula implementation
            double d1 = (std::log(S / K) + (r + sigma * sigma / 2) * T) / (sigma * std::sqrt(T));
            double d2 = d1 - sigma * std::sqrt(T);

            if (is_call) {
                return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
            } else {
                return K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
            }
        }

        double calculate_implied_volatility(bool is_call, double S, double K, double T, double r,
                                            double option_price) {
            // Simple bisection method to find implied volatility
            if (option_price <= 0) {
                throw std::invalid_argument("Option price must be positive");
            }

            // Lower and upper bounds for volatility
            double sigma_low = 0.001;
            double sigma_high = 10.0;

            // Target precision
            double epsilon = 1e-6;

            // Maximum iterations
            int max_iterations = 1000;

            for (int i = 0; i < max_iterations; ++i) {
                double sigma_mid = (sigma_low + sigma_high) / 2;

                double price = black_scholes_price(is_call, S, K, T, r, sigma_mid);

                if (std::abs(price - option_price) < epsilon) {
                    return sigma_mid;
                }

                if (price < option_price) {
                    sigma_low = sigma_mid;
                } else {
                    sigma_high = sigma_mid;
                }
            }

            throw std::runtime_error("Implied volatility calculation did not converge");
        }
    }  // namespace core
}  // namespace iv_calculator
