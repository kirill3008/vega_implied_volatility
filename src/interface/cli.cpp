#include "src/core/black_scholes.h"
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace iv_calculator::core;

// Prints usage instructions
void print_usage() {
    std::cout << "Usage: iv_calculator [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help                 Show this help message" << std::endl;
    std::cout << "  --call                 Calculate for Call option (default)" << std::endl;
    std::cout << "  --put                  Calculate for Put option" << std::endl;
    std::cout << "  --price PRICE          Calculate implied volatility from option price"
              << std::endl;
    std::cout << "  --volatility VOL       Calculate option price from volatility" << std::endl;
    std::cout << "  --asset PRICE          Current price of the underlying asset" << std::endl;
    std::cout << "  --strike PRICE         Strike price of the option" << std::endl;
    std::cout << "  --time YEARS           Time to expiration in years" << std::endl;
    std::cout << "  --rate RATE            Risk-free interest rate (as decimal)" << std::endl;
    std::cout << "  --batch FILE           Process batch data from CSV file" << std::endl;
    std::cout << "  --output FILE          Write results to CSV file" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout
        << "  iv_calculator --call --asset 100 --strike 100 --time 1 --rate 0.05 --volatility 0.2"
        << std::endl;
    std::cout << "  iv_calculator --put --asset 100 --strike 100 --time 1 --rate 0.05 --price 5.57"
              << std::endl;
}

// Structure to hold command line arguments
struct Arguments {
    bool is_call = true;
    double asset_price = 0.0;
    double strike_price = 0.0;
    double time_to_expiry = 0.0;
    double risk_free_rate = 0.0;
    double option_price = -1.0;  // Negative means not provided
    double volatility = -1.0;    // Negative means not provided
    std::string batch_file = "";
    std::string output_file = "";
    bool help_requested = false;
    bool is_valid = true;
};

// Parse command line arguments
Arguments parse_arguments(int argc, char** argv) {
    Arguments args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        if (arg == "--help") {
            args.help_requested = true;
            return args;
        } else if (arg == "--call") {
            args.is_call = true;
        } else if (arg == "--put") {
            args.is_call = false;
        } else if (arg == "--price" && i + 1 < argc) {
            try {
                args.option_price = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid option price" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--volatility" && i + 1 < argc) {
            try {
                args.volatility = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid volatility" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--asset" && i + 1 < argc) {
            try {
                args.asset_price = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid asset price" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--strike" && i + 1 < argc) {
            try {
                args.strike_price = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid strike price" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--time" && i + 1 < argc) {
            try {
                args.time_to_expiry = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid time to expiry" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--rate" && i + 1 < argc) {
            try {
                args.risk_free_rate = std::stod(argv[++i]);
            } catch (...) {
                std::cerr << "Error: Invalid risk-free rate" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--batch" && i + 1 < argc) {
            args.batch_file = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            args.output_file = argv[++i];
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            args.is_valid = false;
            return args;
        }
    }

    // Validate required parameters for single calculation
    if (args.batch_file.empty()) {
        if (args.asset_price <= 0 || args.strike_price <= 0 || args.time_to_expiry <= 0) {
            std::cerr << "Error: Asset price, strike price, and time to expiry must be positive"
                      << std::endl;
            args.is_valid = false;
        }

        if (args.option_price < 0 && args.volatility < 0) {
            std::cerr << "Error: Either option price or volatility must be provided" << std::endl;
            args.is_valid = false;
        }

        if (args.option_price >= 0 && args.volatility >= 0) {
            std::cerr << "Warning: Both option price and volatility provided, will calculate price "
                         "from volatility"
                      << std::endl;
            args.option_price = -1.0;  // Prioritize volatility calculation
        }
    }

    return args;
}

// Process batch file (simple CSV format)
bool process_batch_file(const std::string& input_file, const std::string& output_file) {
    std::ifstream infile(input_file);
    if (!infile) {
        std::cerr << "Error: Could not open input file '" << input_file << "'" << std::endl;
        return false;
    }

    std::ofstream outfile;
    if (!output_file.empty()) {
        outfile.open(output_file);
        if (!outfile) {
            std::cerr << "Error: Could not open output file '" << output_file << "'" << std::endl;
            return false;
        }

        // Write header
        outfile << "Type,Asset,Strike,Time,Rate,Price,Volatility" << std::endl;
    }

    std::string line;
    // Skip header line
    std::getline(infile, line);

    int processed = 0;
    int errors = 0;

    // Process each line
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 6) {
            std::cerr << "Error: Invalid CSV format" << std::endl;
            errors++;
            continue;
        }

        try {
            bool is_call = (tokens[0] == "call" || tokens[0] == "Call");
            double asset = std::stod(tokens[1]);
            double strike = std::stod(tokens[2]);
            double time = std::stod(tokens[3]);
            double rate = std::stod(tokens[4]);
            double price_or_vol = std::stod(tokens[5]);
            bool is_price = (tokens.size() > 6 && tokens[6] == "price") || tokens.size() == 6;

            double result = std::numeric_limits<double>::quiet_NaN(); // Initialize to NaN
            if (is_price) {
                // Calculate implied volatility
                result =
                    calculate_implied_volatility(is_call, asset, strike, time, rate, price_or_vol);

                // Print result to console
                std::cout << "Option: " << (is_call ? "Call" : "Put") << ", S=" << asset
                          << ", K=" << strike << ", T=" << time << ", r=" << rate
                          << ", price=" << price_or_vol << ", implied volatility=" << result
                          << std::endl;

                // Write to output file
                if (outfile) {
                    outfile << (is_call ? "Call" : "Put") << "," << asset << "," << strike << ","
                            << time << "," << rate << "," << price_or_vol << "," << result
                            << std::endl;
                }
            } else {
                // Calculate option price
                result = black_scholes_price(is_call, asset, strike, time, rate, price_or_vol);

                // Print result to console
                std::cout << "Option: " << (is_call ? "Call" : "Put") << ", S=" << asset
                          << ", K=" << strike << ", T=" << time << ", r=" << rate
                          << ", volatility=" << price_or_vol << ", price=" << result << std::endl;

                // Write to output file
                if (outfile) {
                    outfile << (is_call ? "Call" : "Put") << "," << asset << "," << strike << ","
                            << time << "," << rate << "," << result << "," << price_or_vol
                            << std::endl;
                }
            }
            processed++;
        } catch (const std::exception& e) {
            std::cerr << "Error processing line: " << line << ". " << e.what() << std::endl;
            errors++;
        }
    }

    std::cout << "Batch processing complete. Processed " << processed << " items with " << errors
              << " errors." << std::endl;
    return true;
}

int main(int argc, char** argv) {
    Arguments args = parse_arguments(argc, argv);

    if (args.help_requested) {
        print_usage();
        return 0;
    }

    if (!args.is_valid) {
        print_usage();
        return 1;
    }

    // Process batch mode if batch file is provided
    if (!args.batch_file.empty()) {
        if (!process_batch_file(args.batch_file, args.output_file)) {
            return 1;
        }
        return 0;
    }

    // Process single calculation
    try {
        if (args.option_price < 0) {
            // Calculate option price from volatility
            double price =
                black_scholes_price(args.is_call, args.asset_price, args.strike_price,
                                    args.time_to_expiry, args.risk_free_rate, args.volatility);

            std::cout << "Option: " << (args.is_call ? "Call" : "Put") << std::endl;
            std::cout << "Asset price: " << args.asset_price << std::endl;
            std::cout << "Strike price: " << args.strike_price << std::endl;
            std::cout << "Time to expiry: " << args.time_to_expiry << " years" << std::endl;
            std::cout << "Risk-free rate: " << args.risk_free_rate << std::endl;
            std::cout << "Volatility: " << args.volatility << std::endl;
            std::cout << "Option price: " << std::fixed << std::setprecision(6) << price
                      << std::endl;

            // Write to output file if specified
            if (!args.output_file.empty()) {
                std::ofstream outfile(args.output_file);
                if (!outfile) {
                    std::cerr << "Error: Could not open output file '" << args.output_file << "'"
                              << std::endl;
                    return 1;
                }
                outfile << "Type,Asset,Strike,Time,Rate,Price,Volatility" << std::endl;
                outfile << (args.is_call ? "Call" : "Put") << "," << args.asset_price << ","
                        << args.strike_price << "," << args.time_to_expiry << ","
                        << args.risk_free_rate << "," << price << "," << args.volatility
                        << std::endl;
                std::cout << "Results written to " << args.output_file << std::endl;
            }

        } else {
            // Calculate implied volatility from option price
            double iv = calculate_implied_volatility(args.is_call, args.asset_price,
                                                     args.strike_price, args.time_to_expiry,
                                                     args.risk_free_rate, args.option_price);

            std::cout << "Option: " << (args.is_call ? "Call" : "Put") << std::endl;
            std::cout << "Asset price: " << args.asset_price << std::endl;
            std::cout << "Strike price: " << args.strike_price << std::endl;
            std::cout << "Time to expiry: " << args.time_to_expiry << " years" << std::endl;
            std::cout << "Risk-free rate: " << args.risk_free_rate << std::endl;
            std::cout << "Option price: " << args.option_price << std::endl;
            std::cout << "Implied volatility: " << std::fixed << std::setprecision(6) << iv
                      << std::endl;

            // Write to output file if specified
            if (!args.output_file.empty()) {
                std::ofstream outfile(args.output_file);
                if (!outfile) {
                    std::cerr << "Error: Could not open output file '" << args.output_file << "'"
                              << std::endl;
                    return 1;
                }
                outfile << "Type,Asset,Strike,Time,Rate,Price,Volatility" << std::endl;
                outfile << (args.is_call ? "Call" : "Put") << "," << args.asset_price << ","
                        << args.strike_price << "," << args.time_to_expiry << ","
                        << args.risk_free_rate << "," << args.option_price << "," << iv
                        << std::endl;
                std::cout << "Results written to " << args.output_file << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
