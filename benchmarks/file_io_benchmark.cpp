#include "src/core/black_scholes.h"
#include "src/io/file_io.h"
#include <benchmark/benchmark.h>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace iv_calculator::core;
using namespace iv_calculator::io;

// Helper function to generate test CSV data
std::string generate_test_csv_data(int num_options) {
    std::stringstream ss;
    ss << "Type,Asset,Strike,Time,Rate,Price,Volatility\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Generate more realistic option parameters to avoid convergence issues
    std::uniform_real_distribution<> asset_dist(90.0, 110.0);  // Narrower range
    std::uniform_real_distribution<> moneyness_dist(0.85, 1.15); // Strike/Asset ratio
    std::uniform_real_distribution<> time_dist(0.25, 1.0);     // Reasonable expiry
    std::uniform_real_distribution<> rate_dist(0.02, 0.06);    // Typical rates
    std::uniform_real_distribution<> vol_dist(0.15, 0.35);     // Realistic vol range
    
    for (int i = 0; i < num_options; ++i) {
        bool is_call = i % 2 == 0;
        double asset = asset_dist(gen);
        double time = time_dist(gen);
        double rate = rate_dist(gen);
        double vol = vol_dist(gen);
        
        // Ensure strike price is reasonable relative to asset price
        // This prevents deep ITM/OTM options that might cause convergence issues
        double moneyness = moneyness_dist(gen);
        double strike = is_call ? 
            asset * std::min(moneyness, 1.1) : // Call: avoid deep OTM
            asset * std::max(moneyness, 0.9);   // Put: avoid deep OTM
        
        // Calculate option price using current volatility
        double price = black_scholes_price(is_call, asset, strike, time, rate, vol);
        
        ss << (is_call ? "Call" : "Put") << ","
           << asset << ","
           << strike << ","
           << time << ","
           << rate << ","
           << price << ","
           << vol << "\n";
    }
    
    return ss.str();
}

// Helper function to generate test JSON data
std::string generate_test_json_data(int num_options) {
    std::stringstream ss;
    ss << "[\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Generate more realistic option parameters to avoid convergence issues
    std::uniform_real_distribution<> asset_dist(90.0, 110.0);  // Narrower range
    std::uniform_real_distribution<> moneyness_dist(0.85, 1.15); // Strike/Asset ratio
    std::uniform_real_distribution<> time_dist(0.25, 1.0);     // Reasonable expiry
    std::uniform_real_distribution<> rate_dist(0.02, 0.06);    // Typical rates
    std::uniform_real_distribution<> vol_dist(0.15, 0.35);     // Realistic vol range
    
    for (int i = 0; i < num_options; ++i) {
        bool is_call = i % 2 == 0;
        double asset = asset_dist(gen);
        double time = time_dist(gen);
        double rate = rate_dist(gen);
        double vol = vol_dist(gen);
        
        // Ensure strike price is reasonable relative to asset price
        // This prevents deep ITM/OTM options that might cause convergence issues
        double moneyness = moneyness_dist(gen);
        double strike = is_call ? 
            asset * std::min(moneyness, 1.1) : // Call: avoid deep OTM
            asset * std::max(moneyness, 0.9);   // Put: avoid deep OTM
        
        // Calculate option price using current volatility
        double price = black_scholes_price(is_call, asset, strike, time, rate, vol);
        
        ss << "    {\n"
           << "        \"type\": \"" << (is_call ? "Call" : "Put") << "\",\n"
           << "        \"asset_price\": " << asset << ",\n"
           << "        \"strike_price\": " << strike << ",\n"
           << "        \"time_to_expiry\": " << time << ",\n"
           << "        \"risk_free_rate\": " << rate << ",\n"
           << "        \"option_price\": " << price << ",\n"
           << "        \"volatility\": " << vol << "\n"
           << "    }";
        
        if (i < num_options - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "]";
    return ss.str();
}

// Benchmark for CSV file reading and processing
static void BM_CSVFileProcessing(benchmark::State& state) {
    int num_options = state.range(0);
    std::string temp_file = "temp_benchmark.csv";
    
    // Generate test data and write to file
    std::string csv_data = generate_test_csv_data(num_options);
    {
        std::ofstream file(temp_file);
        file << csv_data;
    }
    
    for (auto _ : state) {
        // Time the file reading and processing
        auto options = read_csv(temp_file);
        
        // Calculate implied volatility for each option
        for (auto& option : options) {
            try {
                option.volatility = calculate_implied_volatility(
                    option.is_call, option.asset_price, option.strike_price,
                    option.time_to_expiry, option.risk_free_rate, option.option_price
                );
            } catch (const std::exception& e) {
                // Mark as failed but continue to measure overall throughput
                option.volatility = -1.0; // Error indicator
                state.counters["FailedCalculations"]++;
            }
        }
    }
    
    // Cleanup
    std::remove(temp_file.c_str());
    
    // Report batch size only - performance visible in timing columns
    state.counters["Batch"] = num_options;
}

// Benchmark for JSON file reading and processing
static void BM_JSONFileProcessing(benchmark::State& state) {
    int num_options = state.range(0);
    std::string temp_file = "temp_benchmark.json";
    
    // Generate test data and write to file
    std::string json_data = generate_test_json_data(num_options);
    {
        std::ofstream file(temp_file);
        file << json_data;
    }
    
    for (auto _ : state) {
        // Time the file reading and processing
        auto options = read_json(temp_file);
        
        // Calculate implied volatility for each option
        for (auto& option : options) {
            try {
                option.volatility = calculate_implied_volatility(
                    option.is_call, option.asset_price, option.strike_price,
                    option.time_to_expiry, option.risk_free_rate, option.option_price
                );
            } catch (const std::exception& e) {
                // Mark as failed but continue to measure overall throughput
                option.volatility = -1.0; // Error indicator
                state.counters["FailedCalculations"]++;
            }
        }
    }
    
    // Cleanup
    std::remove(temp_file.c_str());
    
    // Report batch size only
    state.counters["Batch"] = num_options;
}

// Benchmark for batch processing with just the core calculation (no file I/O)
static void BM_MemoryBatchProcessing(benchmark::State& state) {
    int num_options = state.range(0);
    
    std::vector<OptionData> options;
    
    // Generate test data in memory
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Generate more realistic option parameters to avoid convergence issues
    std::uniform_real_distribution<> asset_dist(90.0, 110.0);  // Narrower range
    std::uniform_real_distribution<> moneyness_dist(0.85, 1.15); // Strike/Asset ratio
    std::uniform_real_distribution<> time_dist(0.25, 1.0);     // Reasonable expiry
    std::uniform_real_distribution<> rate_dist(0.02, 0.06);    // Typical rates
    std::uniform_real_distribution<> vol_dist(0.15, 0.35);     // Realistic vol range
    
    for (int i = 0; i < num_options; ++i) {
        OptionData option;
        option.is_call = i % 2 == 0;
        option.asset_price = asset_dist(gen);
        option.time_to_expiry = time_dist(gen);
        option.risk_free_rate = rate_dist(gen);
        double vol = vol_dist(gen);
        
        // Ensure strike price is reasonable relative to asset price
        // This prevents deep ITM/OTM options that might cause convergence issues
        double moneyness = moneyness_dist(gen);
        option.strike_price = option.is_call ? 
            option.asset_price * std::min(moneyness, 1.1) : // Call: avoid deep OTM
            option.asset_price * std::max(moneyness, 0.9);   // Put: avoid deep OTM
        
        // Calculate option price using current volatility
        option.option_price = black_scholes_price(
            option.is_call, option.asset_price, option.strike_price,
            option.time_to_expiry, option.risk_free_rate, vol
        );
        
        options.push_back(option);
    }
    
    for (auto _ : state) {
        // Calculate implied volatility for each option
        for (auto& option : options) {
            try {
                option.volatility = calculate_implied_volatility(
                    option.is_call, option.asset_price, option.strike_price,
                    option.time_to_expiry, option.risk_free_rate, option.option_price
                );
            } catch (const std::exception& e) {
                // Mark as failed but continue to measure overall throughput
                option.volatility = -1.0; // Error indicator
                state.counters["FailedCalculations"]++;
            }
        }
    }
    
    // Report batch size only
    state.counters["Batch"] = num_options;
}

// Benchmark for the CLI batch processor
static void BM_CLIBatchProcessing(benchmark::State& state) {
    int num_options = state.range(0);
    std::string temp_input = "temp_cli_input.csv";
    std::string temp_output = "temp_cli_output.csv";
    
    // Generate test data
    std::string csv_data = generate_test_csv_data(num_options);
    {
        std::ofstream file(temp_input);
        file << csv_data;
    }
    
    for (auto _ : state) {
        // Simulate CLI call (would need to create a separate executable for this)
        // For now, we'll just measure the processing part
        auto options = read_csv(temp_input);
        
        // Calculate implied volatility for each option
        for (auto& option : options) {
            if (option.option_price > 0 && option.volatility <= 0) {
                try {
                    option.volatility = calculate_implied_volatility(
                        option.is_call, option.asset_price, option.strike_price,
                        option.time_to_expiry, option.risk_free_rate, option.option_price
                    );
                } catch (const std::exception& e) {
                    // Mark as failed but continue to measure overall throughput
                    option.volatility = -1.0; // Error indicator
                    state.counters["FailedCalculations"]++;
                }
            }
        }
        
        // Write results back
        write_csv(temp_output, options);
    }
    
    // Cleanup
    std::remove(temp_input.c_str());
    std::remove(temp_output.c_str());
    
    // Report batch size only
    state.counters["Batch"] = num_options;
}

// Register benchmarks with different batch sizes
// Args: [num_options]
BENCHMARK(BM_CSVFileProcessing)
    ->Args({10})      // Small batch
    ->Args({100})     // Medium batch  
    ->Args({1000})    // Target requirement
    ->Args({10000})   // Large batch
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_JSONFileProcessing)
    ->Args({10})      // Small batch
    ->Args({100})     // Medium batch
    ->Args({1000})    // Target requirement
    ->Args({10000})   // Large batch
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_MemoryBatchProcessing)
    ->Args({10})      // Small batch
    ->Args({100})     // Medium batch
    ->Args({1000})    // Target requirement
    ->Args({10000})   // Large batch
    ->Args({100000})  // Very large batch
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_CLIBatchProcessing)
    ->Args({10})      // Small batch
    ->Args({100})     // Medium batch
    ->Args({1000})    // Target requirement
    ->Args({10000})   // Large batch
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

