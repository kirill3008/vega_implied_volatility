#include "src/core/black_scholes.h"
#include "src/io/file_io.h"
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
    std::cout << "  --input-file FILE      Process batch data from file" << std::endl;
    std::cout << "  --input-format FORMAT  Input file format: csv or json (default: csv)"
              << std::endl;
    std::cout << "  --output-file FILE     Write results to file" << std::endl;
    std::cout << "  --output-format FORMAT Output file format: csv or json (default: csv)"
              << std::endl;
    std::cout << "  --batch FILE           [Deprecated] Process batch data from CSV file (use "
                 "--input-file instead)"
              << std::endl;
    std::cout << "  --output FILE          [Deprecated] Write results to CSV file (use "
                 "--output-file instead)"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout
        << "  iv_calculator --call --asset 100 --strike 100 --time 1 --rate 0.05 --volatility 0.2"
        << std::endl;
    std::cout << "  iv_calculator --put --asset 100 --strike 100 --time 1 --rate 0.05 --price 5.57"
              << std::endl;
    std::cout << "  iv_calculator --input-file options.json --input-format json --output-file "
                 "results.json --output-format json"
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
    std::string input_file = "";
    std::string input_format = "csv";
    std::string output_format = "csv";
    bool help_requested = false;
    bool is_valid = true;
};

// Parse command line arguments
Arguments parse_arguments(int argc, char** argv) {
    Arguments args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

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
            // For backward compatibility
            args.input_file = args.batch_file;
            args.input_format = "csv";
        } else if (arg == "--output" && i + 1 < argc) {
            args.output_file = argv[++i];
            // For backward compatibility
            args.output_format = "csv";
        } else if (arg == "--input-file" && i + 1 < argc) {
            args.input_file = argv[++i];
        } else if (arg == "--input-format" && i + 1 < argc) {
            args.input_format = argv[++i];
            if (args.input_format != "csv" && args.input_format != "json") {
                std::cerr << "Error: Input format must be 'csv' or 'json'" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else if (arg == "--output-file" && i + 1 < argc) {
            args.output_file = argv[++i];
        } else if (arg == "--output-format" && i + 1 < argc) {
            args.output_format = argv[++i];
            if (args.output_format != "csv" && args.output_format != "json") {
                std::cerr << "Error: Output format must be 'csv' or 'json'" << std::endl;
                args.is_valid = false;
                return args;
            }
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            args.is_valid = false;
            return args;
        }
    }

    // Validate required parameters for single calculation
    if (args.input_file.empty()) {
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

// Process batch file using the io module
bool process_batch_file_with_io(const std::string& input_file, const std::string& input_format,
                                const std::string& output_file, const std::string& output_format) {
    try {
        // Load options data from input file
        std::vector<iv_calculator::io::OptionData> options;

        if (input_format == "csv") {
            options = iv_calculator::io::read_csv(input_file);
        } else if (input_format == "json") {
            options = iv_calculator::io::read_json(input_file);
        } else {
            std::cerr << "Error: Unsupported input format '" << input_format << "'" << std::endl;
            return false;
        }

        std::cout << "Loaded " << options.size() << " options from " << input_file << std::endl;

        // Calculate implied volatility for each option
        int processed = 0;
        int errors = 0;

        for (auto& option : options) {
            try {
                // If volatility is set but price isn't, calculate price
                if (option.volatility > 0 && option.option_price <= 0) {
                    option.option_price = black_scholes_price(
                        option.is_call, option.asset_price, option.strike_price,
                        option.time_to_expiry, option.risk_free_rate, option.volatility);

                    // Print result to console
                    std::cout << "Option: " << (option.is_call ? "Call" : "Put")
                              << ", S=" << option.asset_price << ", K=" << option.strike_price
                              << ", T=" << option.time_to_expiry << ", r=" << option.risk_free_rate
                              << ", volatility=" << option.volatility
                              << ", price=" << option.option_price << std::endl;
                }
                // If price is set but volatility isn't, calculate implied volatility
                else if (option.option_price > 0 && option.volatility <= 0) {
                    option.volatility = calculate_implied_volatility(
                        option.is_call, option.asset_price, option.strike_price,
                        option.time_to_expiry, option.risk_free_rate, option.option_price);

                    // Print result to console
                    std::cout << "Option: " << (option.is_call ? "Call" : "Put")
                              << ", S=" << option.asset_price << ", K=" << option.strike_price
                              << ", T=" << option.time_to_expiry << ", r=" << option.risk_free_rate
                              << ", price=" << option.option_price
                              << ", implied volatility=" << option.volatility << std::endl;
                }
                // If both are set, we'll just use them as-is
                processed++;
            } catch (const std::exception& e) {
                std::cerr << "Error processing option: " << e.what() << std::endl;
                errors++;
            }
        }

        // Write results to output file if specified
        if (!output_file.empty()) {
            bool success = false;
            if (output_format == "csv") {
                success = iv_calculator::io::write_csv(output_file, options);
            } else if (output_format == "json") {
                success = iv_calculator::io::write_json(output_file, options);
            } else {
                std::cerr << "Error: Unsupported output format '" << output_format << "'"
                          << std::endl;
                return false;
            }

            if (success) {
                std::cout << "Results written to " << output_file << std::endl;
            } else {
                std::cerr << "Error writing to " << output_file << std::endl;
                return false;
            }
        }

        std::cout << "Batch processing complete. Processed " << processed << " items with "
                  << errors << " errors." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
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

            double result = std::numeric_limits<double>::quiet_NaN();  // Initialize to NaN
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
    if (!args.input_file.empty()) {
        // Use the new IO-based batch processor if input format is JSON or the new flags are used
        if (args.input_format == "json" || args.output_format == "json" ||
            args.input_file != args.batch_file) {
            if (!process_batch_file_with_io(args.input_file, args.input_format, args.output_file,
                                            args.output_format)) {
                return 1;
            }
        } else {
            // Use legacy batch processor for backward compatibility
            if (!process_batch_file(args.batch_file, args.output_file)) {
                return 1;
            }
        }
        return 1;
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
