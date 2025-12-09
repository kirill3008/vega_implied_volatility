#include "src/core/black_scholes.h"
#include <benchmark/benchmark.h>

using namespace iv_calculator::core;

// Benchmark for Black-Scholes price calculation
static void BM_BlackScholesPriceCall(benchmark::State& state) {
    double S = 100.0;      // Asset price
    double K = 100.0;      // Strike price
    double T = 1.0;        // Time to expiry (1 year)
    double r = 0.05;       // Risk-free rate
    double sigma = 0.2;    // Volatility
    
    for (auto _ : state) {
        double price = black_scholes_price(true, S, K, T, r, sigma);
        benchmark::DoNotOptimize(price);
    }
}

static void BM_BlackScholesPricePut(benchmark::State& state) {
    double S = 100.0;      // Asset price
    double K = 100.0;      // Strike price
    double T = 1.0;        // Time to expiry (1 year)
    double r = 0.05;       // Risk-free rate
    double sigma = 0.2;    // Volatility
    
    for (auto _ : state) {
        double price = black_scholes_price(false, S, K, T, r, sigma);
        benchmark::DoNotOptimize(price);
    }
}

// Benchmark for vega calculation
static void BM_BlackScholesVega(benchmark::State& state) {
    double S = 100.0;      // Asset price
    double K = 100.0;      // Strike price
    double T = 1.0;        // Time to expiry (1 year)
    double r = 0.05;       // Risk-free rate
    double sigma = 0.2;    // Volatility
    
    for (auto _ : state) {
        double vega = black_scholes_vega(S, K, T, r, sigma);
        benchmark::DoNotOptimize(vega);
    }
}

// Benchmark for implied volatility calculation with bisection method
static void BM_ImpliedVolatilityBisection(benchmark::State& state) {
    bool is_call = state.range(0) == 1;  // Parameterize call/put
    double S = 100.0;       // Asset price
    double K = state.range(1);           // Parameterize strike price
    double T = 1.0;         // Time to expiry (1 year)
    double r = 0.05;        // Risk-free rate
    double original_vol = 0.2;  // Original volatility
    
    // Calculate the option price first
    double option_price = black_scholes_price(is_call, S, K, T, r, original_vol);
    
    for (auto _ : state) {
        double implied_vol = calculate_implied_volatility(
            is_call, S, K, T, r, option_price, 
            ImpliedVolatilityMethod::BISECTION
        );
        benchmark::DoNotOptimize(implied_vol);
    }
}

// Benchmark for implied volatility calculation with Newton-Raphson method
static void BM_ImpliedVolatilityNewtonRaphson(benchmark::State& state) {
    bool is_call = state.range(0) == 1;  // Parameterize call/put
    double S = 100.0;       // Asset price
    double K = state.range(1);           // Parameterize strike price
    double T = 1.0;         // Time to expiry (1 year)
    double r = 0.05;        // Risk-free rate
    double original_vol = 0.2;  // Original volatility
    
    // Calculate the option price first
    double option_price = black_scholes_price(is_call, S, K, T, r, original_vol);
    
    for (auto _ : state) {
        double implied_vol = calculate_implied_volatility(
            is_call, S, K, T, r, option_price,
            ImpliedVolatilityMethod::NEWTON_RAPHSON
        );
        benchmark::DoNotOptimize(implied_vol);
    }
}

// At-the-money benchmark
BENCHMARK(BM_BlackScholesPriceCall);
BENCHMARK(BM_BlackScholesPricePut);
BENCHMARK(BM_BlackScholesVega);

// Parameterized benchmarks for different moneyness scenarios
// Args: [is_call, strike_price]
BENCHMARK(BM_ImpliedVolatilityBisection)
    ->Args({1, 100})   // ATM Call
    ->Args({0, 100})   // ATM Put  
    ->Args({1, 90})    // In-the-money Call
    ->Args({1, 110})   // Out-of-the-money Call
    ->Args({0, 90})    // Out-of-the-money Put
    ->Args({0, 110})   // In-the-money Put
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ImpliedVolatilityNewtonRaphson)
    ->Args({1, 100})   // ATM Call
    ->Args({0, 100})   // ATM Put
    ->Args({1, 90})    // In-the-money Call
    ->Args({1, 110})   // Out-of-the-money Call
    ->Args({0, 90})    // Out-of-the-money Put
    ->Args({0, 110})   // In-the-money Put
    ->Unit(benchmark::kMicrosecond);

// Additional benchmarks for different time scenarios
static void BM_ImpliedVolatilityTimeScenarios(benchmark::State& state) {
    bool is_call = true;
    double S = 100.0;
    double K = 100.0;
    double T = state.range(0) / 365.0;  // Convert days to years
    double r = 0.05;
    double original_vol = 0.2;
    
    double option_price = black_scholes_price(is_call, S, K, T, r, original_vol);
    
    for (auto _ : state) {
        double implied_vol = calculate_implied_volatility(
            is_call, S, K, T, r, option_price,
            ImpliedVolatilityMethod::NEWTON_RAPHSON
        );
        benchmark::DoNotOptimize(implied_vol);
    }
}

// Args: [days_to_expiry]
BENCHMARK(BM_ImpliedVolatilityTimeScenarios)
    ->Args({1})      // 1 day
    ->Args({7})      // 1 week
    ->Args({30})     // 1 month
    ->Args({90})     // 3 months
    ->Args({365})    // 1 year
    ->Unit(benchmark::kMicrosecond);

// Performance verification against README requirements
static void BM_SingleCalculationRequirement(benchmark::State& state) {
    // Verify the 10ms requirement for single calculation
    bool is_call = true;
    double S = 100.0;
    double K = 100.0;
    double T = 1.0;
    double r = 0.05;
    double original_vol = 0.2;
    
    double option_price = black_scholes_price(is_call, S, K, T, r, original_vol);
    
    for (auto _ : state) {
        double implied_vol = calculate_implied_volatility(
            is_call, S, K, T, r, option_price,
            ImpliedVolatilityMethod::BISECTION
        );
        benchmark::DoNotOptimize(implied_vol);
    }
}

BENCHMARK(BM_SingleCalculationRequirement)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
