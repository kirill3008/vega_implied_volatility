#!/bin/bash

# This script demonstrates how to use the JSON functionality in the IV Calculator

# Make this script executable with: chmod +x run_json_example.sh

# Check if the project is built
if [ ! -f "../build/src/iv_calculator" ]; then
  echo "Error: IV Calculator not found. Please build the project first."
  echo "Run: cmake -B build -S .. && cmake --build build"
  exit 1
 fi

# Create a test JSON file if it doesn't exist
JSON_FILE="options_data.json"

if [ ! -f "$JSON_FILE" ]; then
  echo "Creating example JSON file: $JSON_FILE"
  cat > "$JSON_FILE" << EOL
[
  {
    "type": "Call",
    "asset_price": 100.0,
    "strike_price": 100.0,
    "time_to_expiry": 1.0,
    "risk_free_rate": 0.05,
    "option_price": 10.45
  },
  {
    "type": "Call",
    "asset_price": 100.0,
    "strike_price": 105.0,
    "time_to_expiry": 1.0,
    "risk_free_rate": 0.05,
    "option_price": 7.82
  },
  {
    "type": "Put",
    "asset_price": 100.0,
    "strike_price": 100.0,
    "time_to_expiry": 1.0,
    "risk_free_rate": 0.05,
    "option_price": 5.57
  }
]
EOL
  echo "Created example JSON file."
fi

# Output file for results
OUTPUT_FILE="results.json"

echo "Running IV Calculator with JSON input..."
echo "=======================================\n"

# Run the calculator with JSON input/output
../build/src/iv_calculator --input-file="$JSON_FILE" --input-format=json --output-file="$OUTPUT_FILE" --output-format=json

echo "\n\nResults saved to $OUTPUT_FILE"
echo "\nResults preview:"
cat "$OUTPUT_FILE"

echo "\n\nExample complete!"
