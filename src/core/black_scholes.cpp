#include "black_scholes.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace iv_calculator {
    namespace core {
        // Standard normal cumulative distribution function
        double norm_cdf(double x) { return 0.5 * (1 + std::erf(x / std::sqrt(2))); }

        // Standard normal probability density function
        double norm_pdf(double x) { return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x); }

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

        double black_scholes_vega(double S, double K, double T, double r, double sigma) {
            // Input validation
            if (S <= 0 || K <= 0 || T <= 0 || sigma <= 0) {
                throw std::invalid_argument("Invalid input parameters");
            }

            // Calculate d1 from Black-Scholes formula
            double d1 = (std::log(S / K) + (r + sigma * sigma / 2) * T) / (sigma * std::sqrt(T));

            // Vega formula (same for both call and put options)
            // Vega = S * sqrt(T) * norm_pdf(d1)
            return S * std::sqrt(T) * norm_pdf(d1) /
                   100.0;  // Divided by 100 to convert to percentage points
        }

        double calculate_implied_volatility(bool is_call, double S, double K, double T, double r,
                                            double option_price, ImpliedVolatilityMethod method) {
            // Choose the appropriate method based on the parameter
            switch (method) {
                case ImpliedVolatilityMethod::NEWTON_RAPHSON:
                    try {
                        return newton_raphson_implied_volatility(is_call, S, K, T, r, option_price);
                    } catch (const std::runtime_error&) {
                        // If Newton-Raphson fails, fall back to bisection method
                        return bisection_implied_volatility(is_call, S, K, T, r, option_price);
                    }
                case ImpliedVolatilityMethod::BISECTION:
                default:
                    return bisection_implied_volatility(is_call, S, K, T, r, option_price);
            }
        }

        double bisection_implied_volatility(bool is_call, double S, double K, double T, double r,
                                            double option_price) {
            // Simple bisection method to find implied volatility
            if (option_price <= 0) {
                throw std::invalid_argument("Option price must be positive");
            }

            // Lower and upper bounds for volatility
            double sigma_low = 0.001;
            double sigma_high = 10.0;

            // Target precision
            double epsilon = 1e-8;  // Increased precision

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

        double newton_raphson_implied_volatility(bool is_call, double S, double K, double T,
                                                 double r, double option_price) {
            // Newton-Raphson method to find implied volatility
            if (option_price <= 0) {
                throw std::invalid_argument("Option price must be positive");
            }

            // Initial volatility guess - use better approximation for starting point
            // For ATM options, use Brenner and Subrahmanyam (1988) approximation
            // For others, use a standard starting point
            double sigma = NAN;
            // For short expiry options, start with higher initial volatility
            if (T < 0.1) {
                sigma = 0.5;  // Higher starting point for short expiry
            }
            // For ATM options, use Brenner-Subrahmanyam approximation
            else if (std::abs(S / K - 1.0) < 0.1) {
                // Brenner-Subrahmanyam approximation: sigma ≈ sqrt(2π/T) * (C/S)
                sigma = std::sqrt(2.0 * M_PI / T) * option_price / S;
                // Clamp to reasonable range
                sigma = std::max(0.1, std::min(sigma, 1.0));
            }
            // For ITM options, use lower initial volatility
            else if ((is_call && S > K) || (!is_call && S < K)) {
                sigma = 0.2;  // ITM
            }
            // For OTM options, use higher initial volatility
            else {
                sigma = 0.4;  // OTM
            }

            // Target precision
            double epsilon = 1e-6;

            // Maximum iterations
            int max_iterations = 100;  // Increased max iterations

            // Last valid sigma (in case we need to revert)
            double last_valid_sigma = sigma;
            double best_price_diff = std::numeric_limits<double>::max();
            double best_sigma = sigma;

            for (int i = 0; i < max_iterations; ++i) {
                // Calculate price and vega at current sigma
                double price = black_scholes_price(is_call, S, K, T, r, sigma);
                double vega = black_scholes_vega(S, K, T, r, sigma);

                // Track the best approximation so far
                double price_diff = std::abs(price - option_price);
                if (price_diff < best_price_diff) {
                    best_price_diff = price_diff;
                    best_sigma = sigma;
                }

                // Check for convergence
                if (std::abs(price - option_price) < epsilon) {
                    return sigma;
                }

                // Check for very small vega to avoid division by near-zero
                if (std::abs(vega) < 1e-10) {
                    // Fall back to last valid sigma or throw if none
                    if (last_valid_sigma > 0) {
                        sigma = last_valid_sigma;
                        break;
                    } else {
                        throw std::runtime_error("Newton-Raphson method failed: vega too small");
                    }
                }

                // Update sigma using Newton-Raphson formula
                // Calculate raw adjustment
                double adjustment = (price - option_price) / vega;

                // Limit the size of adjustment to prevent overshooting
                // Make smaller adjustments for short expiry options
                double max_adjustment = (T < 0.1) ? 0.1 * sigma : 0.3 * sigma;
                if (std::abs(adjustment) > max_adjustment) {
                    adjustment = (adjustment > 0) ? max_adjustment : -max_adjustment;
                }

                double new_sigma = sigma - adjustment;

                // Ensure volatility stays positive and within reasonable bounds
                if (new_sigma <= 0.0001) {
                    new_sigma = 0.0001;
                } else if (new_sigma > 5.0) {
                    new_sigma = 5.0;
                }

                // Update sigma
                last_valid_sigma = sigma;
                sigma = new_sigma;
            }

            // If we've reached max iterations but have a reasonable value, return it
            // Return either the best approximation or fall back to bisection
            if (best_price_diff < epsilon * 100) {
                return best_sigma;  // Return best approximation found
            } else {
                // Fall back to bisection method
                return bisection_implied_volatility(is_call, S, K, T, r, option_price);
            }
        }
    }  // namespace core
}  // namespace iv_calculator
