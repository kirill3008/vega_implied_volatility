#include "src/io/file_io.h"

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace iv_calculator::io;

// Temporary file paths for testing
const std::string kTempCsvFile = "temp_test.csv";
const std::string kTempJsonFile = "temp_test.json";

// Helper function to create a test CSV file
void CreateTestCsvFile() {
    std::ofstream file(kTempCsvFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create test CSV file";

    file << "Type,Asset,Strike,Time,Rate,Price,Volatility\n";
    file << "Call,100,100,1,0.05,10.45,0.2\n";
    file << "Put,100,100,1,0.05,5.57,0.2\n";
    file.close();
}

// Helper function to cleanup temporary files
void CleanupTempFiles() {
    std::remove(kTempCsvFile.c_str());
    std::remove(kTempJsonFile.c_str());
}
// Helper function to create a test JSON file
void CreateTestJsonFile() {
    std::ofstream file(kTempJsonFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create test JSON file";

    file << "[\n";
    file << "    {\n";
    file << "        \"type\": \"Call\",\n";
    file << "        \"asset_price\": 100.0,\n";
    file << "        \"strike_price\": 100.0,\n";
    file << "        \"time_to_expiry\": 1.0,\n";
    file << "        \"risk_free_rate\": 0.05,\n";
    file << "        \"option_price\": 10.45,\n";
    file << "        \"volatility\": 0.2\n";
    file << "    },\n";
    file << "    {\n";
    file << "        \"type\": \"Put\",\n";
    file << "        \"asset_price\": 100.0,\n";
    file << "        \"strike_price\": 100.0,\n";
    file << "        \"time_to_expiry\": 1.0,\n";
    file << "        \"risk_free_rate\": 0.05,\n";
    file << "        \"option_price\": 5.57,\n";
    file << "        \"volatility\": 0.2\n";
    file << "    }\n";
    file << "]";
    file.close();
}

// Test fixture for IO tests
class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        CreateTestCsvFile();
        CreateTestJsonFile();
    }

    void TearDown() override { CleanupTempFiles(); }
};

// Test reading from CSV file
TEST_F(FileIOTest, ReadCsvTest) {
    auto options = read_csv(kTempCsvFile);

    ASSERT_EQ(options.size(), 2) << "Should have read 2 options from file";

    // Check first option (Call)
    EXPECT_TRUE(options[0].is_call);
    EXPECT_DOUBLE_EQ(options[0].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(options[0].strike_price, 100.0);
    EXPECT_DOUBLE_EQ(options[0].time_to_expiry, 1.0);
    EXPECT_DOUBLE_EQ(options[0].risk_free_rate, 0.05);
    EXPECT_DOUBLE_EQ(options[0].option_price, 10.45);
    EXPECT_DOUBLE_EQ(options[0].volatility, 0.2);

    // Check second option (Put)
    EXPECT_FALSE(options[1].is_call);
    EXPECT_DOUBLE_EQ(options[1].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(options[1].strike_price, 100.0);
    EXPECT_DOUBLE_EQ(options[1].time_to_expiry, 1.0);
    EXPECT_DOUBLE_EQ(options[1].risk_free_rate, 0.05);
    EXPECT_DOUBLE_EQ(options[1].option_price, 5.57);
    EXPECT_DOUBLE_EQ(options[1].volatility, 0.2);
}

// Test writing to CSV file
TEST_F(FileIOTest, WriteCsvTest) {
    std::vector<OptionData> options;

    OptionData call;
    call.is_call = true;
    call.asset_price = 100.0;
    call.strike_price = 110.0;
    call.time_to_expiry = 0.5;
    call.risk_free_rate = 0.03;
    call.option_price = 4.0;
    call.volatility = 0.15;
    options.push_back(call);

    OptionData put;
    put.is_call = false;
    put.asset_price = 100.0;
    put.strike_price = 90.0;
    put.time_to_expiry = 0.5;
    put.risk_free_rate = 0.03;
    put.option_price = 2.0;
    put.volatility = 0.15;
    options.push_back(put);

    // Write to a new file
    EXPECT_TRUE(write_csv(kTempCsvFile, options));

    // Read the file back in
    auto read_options = read_csv(kTempCsvFile);

    ASSERT_EQ(read_options.size(), 2) << "Should have read 2 options from file";

    // Check first option (Call)
    EXPECT_TRUE(read_options[0].is_call);
    EXPECT_DOUBLE_EQ(read_options[0].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(read_options[0].strike_price, 110.0);
    EXPECT_DOUBLE_EQ(read_options[0].time_to_expiry, 0.5);
    EXPECT_DOUBLE_EQ(read_options[0].risk_free_rate, 0.03);
    EXPECT_DOUBLE_EQ(read_options[0].option_price, 4.0);
    EXPECT_DOUBLE_EQ(read_options[0].volatility, 0.15);

    // Check second option (Put)
    EXPECT_FALSE(read_options[1].is_call);
    EXPECT_DOUBLE_EQ(read_options[1].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(read_options[1].strike_price, 90.0);
    EXPECT_DOUBLE_EQ(read_options[1].time_to_expiry, 0.5);
    EXPECT_DOUBLE_EQ(read_options[1].risk_free_rate, 0.03);
    EXPECT_DOUBLE_EQ(read_options[1].option_price, 2.0);
    EXPECT_DOUBLE_EQ(read_options[1].volatility, 0.15);
}

// Test error handling when file doesn't exist
TEST(FileIOErrorTest, NonexistentFileTest) {
    const std::string nonexistent = "nonexistent_file.csv";
    EXPECT_THROW(read_csv(nonexistent), std::runtime_error);
}

// Test reading from JSON file
TEST_F(FileIOTest, ReadJsonTest) {
    auto options = read_json(kTempJsonFile);

    ASSERT_EQ(options.size(), 2) << "Should have read 2 options from JSON file";

    // Check first option (Call)
    EXPECT_TRUE(options[0].is_call);
    EXPECT_DOUBLE_EQ(options[0].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(options[0].strike_price, 100.0);
    EXPECT_DOUBLE_EQ(options[0].time_to_expiry, 1.0);
    EXPECT_DOUBLE_EQ(options[0].risk_free_rate, 0.05);
    EXPECT_DOUBLE_EQ(options[0].option_price, 10.45);
    EXPECT_DOUBLE_EQ(options[0].volatility, 0.2);

    // Check second option (Put)
    EXPECT_FALSE(options[1].is_call);
    EXPECT_DOUBLE_EQ(options[1].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(options[1].strike_price, 100.0);
    EXPECT_DOUBLE_EQ(options[1].time_to_expiry, 1.0);
    EXPECT_DOUBLE_EQ(options[1].risk_free_rate, 0.05);
    EXPECT_DOUBLE_EQ(options[1].option_price, 5.57);
    EXPECT_DOUBLE_EQ(options[1].volatility, 0.2);
}

// Test writing to JSON file
TEST_F(FileIOTest, WriteJsonTest) {
    std::vector<OptionData> options;

    OptionData call;
    call.is_call = true;
    call.asset_price = 100.0;
    call.strike_price = 110.0;
    call.time_to_expiry = 0.5;
    call.risk_free_rate = 0.03;
    call.option_price = 4.0;
    call.volatility = 0.15;
    options.push_back(call);

    OptionData put;
    put.is_call = false;
    put.asset_price = 100.0;
    put.strike_price = 90.0;
    put.time_to_expiry = 0.5;
    put.risk_free_rate = 0.03;
    put.option_price = 2.0;
    put.volatility = 0.15;
    options.push_back(put);

    // Write to a new file
    EXPECT_TRUE(write_json(kTempJsonFile, options));

    // Read the file back in
    auto read_options = read_json(kTempJsonFile);

    ASSERT_EQ(read_options.size(), 2) << "Should have read 2 options from JSON file";

    // Check first option (Call)
    EXPECT_TRUE(read_options[0].is_call);
    EXPECT_DOUBLE_EQ(read_options[0].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(read_options[0].strike_price, 110.0);
    EXPECT_DOUBLE_EQ(read_options[0].time_to_expiry, 0.5);
    EXPECT_DOUBLE_EQ(read_options[0].risk_free_rate, 0.03);
    EXPECT_DOUBLE_EQ(read_options[0].option_price, 4.0);
    EXPECT_DOUBLE_EQ(read_options[0].volatility, 0.15);

    // Check second option (Put)
    EXPECT_FALSE(read_options[1].is_call);
    EXPECT_DOUBLE_EQ(read_options[1].asset_price, 100.0);
    EXPECT_DOUBLE_EQ(read_options[1].strike_price, 90.0);
    EXPECT_DOUBLE_EQ(read_options[1].time_to_expiry, 0.5);
    EXPECT_DOUBLE_EQ(read_options[1].risk_free_rate, 0.03);
    EXPECT_DOUBLE_EQ(read_options[1].option_price, 2.0);
    EXPECT_DOUBLE_EQ(read_options[1].volatility, 0.15);
}

// Test error handling for malformed JSON
TEST_F(FileIOTest, MalformedJsonTest) {
    // Create a malformed JSON file
    std::ofstream file(kTempJsonFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create malformed JSON test file";
    file << "{ This is not valid JSON }";
    file.close();

    EXPECT_THROW(read_json(kTempJsonFile), std::runtime_error);
}

// Test error handling for JSON with missing fields
TEST_F(FileIOTest, MissingFieldsJsonTest) {
    // Create a JSON file with missing required fields
    std::ofstream file(kTempJsonFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create JSON with missing fields test file";
    file << "[{\"type\": \"Call\"}]";
    file.close();

    EXPECT_THROW(read_json(kTempJsonFile), std::runtime_error);
}

// Test error handling when JSON file doesn't exist
TEST(FileIOErrorTest, NonexistentJsonFileTest) {
    const std::string nonexistent = "nonexistent_file.json";
    EXPECT_THROW(read_json(nonexistent), std::runtime_error);
}
