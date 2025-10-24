#include "file_io.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace iv_calculator {
    namespace io {

        // Read from CSV file
        std::vector<OptionData> read_csv(const std::string& filepath) {
            std::vector<OptionData> options;
            std::ifstream file(filepath);

            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + filepath);
            }

            // Skip header line
            std::string line;
            std::getline(file, line);

            // Read data lines
            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string token;
                OptionData option;

                // Parse type (call/put)
                if (std::getline(ss, token, ',')) {
                    option.is_call = (token == "Call" || token == "call");
                }

                // Parse asset price
                if (std::getline(ss, token, ',')) {
                    option.asset_price = std::stod(token);
                }

                // Parse strike price
                if (std::getline(ss, token, ',')) {
                    option.strike_price = std::stod(token);
                }

                // Parse time to expiry
                if (std::getline(ss, token, ',')) {
                    option.time_to_expiry = std::stod(token);
                }

                // Parse risk-free rate
                if (std::getline(ss, token, ',')) {
                    option.risk_free_rate = std::stod(token);
                }

                // Parse option price
                if (std::getline(ss, token, ',')) {
                    option.option_price = std::stod(token);
                }

                // Parse volatility if available
                if (std::getline(ss, token, ',')) {
                    option.volatility = std::stod(token);
                }

                options.push_back(option);
            }

            return options;
        }

        // Simple JSON parsing function
        // Note: For production, consider using a proper JSON library like nlohmann/json
        std::vector<OptionData> read_json(const std::string& filepath) {
            // Placeholder for future implementation
            std::cerr << "JSON parsing not fully implemented yet" << std::endl;
            return {};
        }

        // Write to CSV file
        bool write_csv(const std::string& filepath, const std::vector<OptionData>& options) {
            std::ofstream file(filepath);

            if (!file.is_open()) {
                return false;
            }

            // Write header
            file << "Type,Asset,Strike,Time,Rate,Price,Volatility\n";

            // Write data
            for (const auto& option : options) {
                file << (option.is_call ? "Call" : "Put") << "," << option.asset_price << ","
                     << option.strike_price << "," << option.time_to_expiry << ","
                     << option.risk_free_rate << "," << option.option_price << ","
                     << option.volatility << "\n";
            }

            return true;
        }

        // Simple JSON writing function
        // Note: For production, consider using a proper JSON library like nlohmann/json
        bool write_json(const std::string& filepath, const std::vector<OptionData>& options) {
            // Placeholder for future implementation
            std::cerr << "JSON writing not fully implemented yet" << std::endl;
            return false;
        }

    }  // namespace io
}  // namespace iv_calculator
