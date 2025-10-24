#include "src/io/file_io.h"

#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

using namespace iv_calculator::io;

// Temporary file paths for testing
const std::string kTempCsvFile = "temp_test.csv";

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
}

// Test fixture for IO tests
class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        CreateTestCsvFile();
    }
    
    void TearDown() override {
        CleanupTempFiles();
    }
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
