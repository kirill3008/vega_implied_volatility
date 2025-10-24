find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Create test executable
add_executable(core_tests
    core_tests/black_scholes_test.cpp
)

# Link against our library and Google Test
target_link_libraries(core_tests
    iv_core
    ${GTEST_BOTH_LIBRARIES}
    pthread  # Required on Linux
)

# Add test to CTest
add_test(NAME CoreTests COMMAND core_tests)
