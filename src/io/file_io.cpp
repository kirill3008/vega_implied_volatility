#include "file_io.h"

#include <fstream>
#include <iostream>
#include <simdjson.h>
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
            std::vector<OptionData> options;

            // Load JSON file
            simdjson::dom::parser parser;
            simdjson::dom::element json_data;

            try {
                json_data = parser.load(filepath);
            } catch (const simdjson::simdjson_error& e) {
                throw std::runtime_error("Could not parse file: " + filepath +
                                         ", error: " + e.what());
            }

            // Verify the document is an array
            simdjson::dom::array json_array;
            try {
                json_array = json_data.get_array();
            } catch (const simdjson::simdjson_error& e) {
                throw std::runtime_error("JSON data must be an array of options");
            }

            // Process each option in the array
            for (simdjson::dom::element json_option : json_array) {
                OptionData option;
                std::string_view type_sv;
                double value = NAN;

                // Extract option type
                auto error = json_option["type"].get_string().get(type_sv);
                if (error) {
                    throw std::runtime_error("Option type is missing or invalid");
                }
                std::string type(type_sv);
                option.is_call = (type == "Call" || type == "call");

                // Extract asset price
                error = json_option["asset_price"].get_double().get(value);
                if (error) {
                    throw std::runtime_error("Asset price is missing or invalid");
                }
                option.asset_price = value;

                // Extract strike price
                error = json_option["strike_price"].get_double().get(value);
                if (error) {
                    throw std::runtime_error("Strike price is missing or invalid");
                }
                option.strike_price = value;

                // Extract time to expiry
                error = json_option["time_to_expiry"].get_double().get(value);
                if (error) {
                    throw std::runtime_error("Time to expiry is missing or invalid");
                }
                option.time_to_expiry = value;

                // Extract risk-free rate
                error = json_option["risk_free_rate"].get_double().get(value);
                if (error) {
                    throw std::runtime_error("Risk-free rate is missing or invalid");
                }
                option.risk_free_rate = value;

                // Extract option price
                error = json_option["option_price"].get_double().get(value);
                if (error) {
                    throw std::runtime_error("Option price is missing or invalid");
                }
                option.option_price = value;

                // Extract volatility if available (optional field)
                if (!json_option["volatility"].get_double().get(value)) {
                    option.volatility = value;
                }

                options.push_back(option);
            }

            return options;
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
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }

            try {
                // Start JSON array
                file << "[\n";

                for (size_t i = 0; i < options.size(); ++i) {
                    const auto& option = options[i];

                    // Write JSON object for each option
                    file << "    {\n";
                    file << R"(        "type": ")" << (option.is_call ? "Call" : "Put") << "\",\n";
                    file << "        \"asset_price\": " << option.asset_price << ",\n";
                    file << "        \"strike_price\": " << option.strike_price << ",\n";
                    file << "        \"time_to_expiry\": " << option.time_to_expiry << ",\n";
                    file << "        \"risk_free_rate\": " << option.risk_free_rate << ",\n";
                    file << "        \"option_price\": " << option.option_price << ",\n";
                    file << "        \"volatility\": " << option.volatility;
                    file << "\n    }";

                    // Add comma if not the last element
                    if (i < options.size() - 1) {
                        file << ",";
                    }
                    file << "\n";
                }

                // End JSON array
                file << "]";
                return true;
            } catch (const std::exception& e) {
                std::cerr << "Error writing JSON: " << e.what() << std::endl;
                return false;
            }
        }

    }  // namespace io
}  // namespace iv_calculator
