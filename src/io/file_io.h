#pragma once

#include <string>
#include <vector>

namespace iv_calculator {
    namespace io {

        /**
         * @brief Structure representing option data
         */
        struct OptionData {
            bool is_call = true;        // Call or Put option
            double asset_price = 0;     // Current price of underlying asset
            double strike_price = 0;    // Strike price
            double time_to_expiry = 0;  // Time to expiration in years
            double risk_free_rate = 0;  // Risk-free interest rate
            double option_price = 0;    // Market price of option (for IV calculation)
            double volatility = 0;      // Implied volatility (output)
        };

        /**
         * @brief Read option data from a CSV file
         *
         * @param filepath Path to the CSV file
         * @return std::vector<OptionData> Vector of option data
         */
        std::vector<OptionData> read_csv(const std::string& filepath);

        /**
         * @brief Read option data from a JSON file
         *
         * @param filepath Path to the JSON file
         * @return std::vector<OptionData> Vector of option data
         */
        std::vector<OptionData> read_json(const std::string& filepath);

        /**
         * @brief Write option data to a CSV file
         *
         * @param filepath Path to the output CSV file
         * @param options Vector of option data to write
         * @return bool Success status
         */
        bool write_csv(const std::string& filepath, const std::vector<OptionData>& options);

        /**
         * @brief Write option data to a JSON file
         *
         * @param filepath Path to the output JSON file
         * @param options Vector of option data to write
         * @return bool Success status
         */
        bool write_json(const std::string& filepath, const std::vector<OptionData>& options);

    }  // namespace io
}  // namespace iv_calculator
