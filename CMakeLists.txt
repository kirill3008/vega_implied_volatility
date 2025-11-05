cmake_minimum_required(VERSION 3.15)

# Set clang as the preferred compiler if available
if(NOT DEFINED CMAKE_C_COMPILER)
    find_program(CLANG_EXECUTABLE NAMES clang)
    if(CLANG_EXECUTABLE)
        set(CMAKE_C_COMPILER "${CLANG_EXECUTABLE}")
    endif()
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(CLANGPP_EXECUTABLE NAMES clang++)
    if(CLANGPP_EXECUTABLE)
        set(CMAKE_CXX_COMPILER "${CLANGPP_EXECUTABLE}")
    endif()
endif()

project(implied_volatility VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR})

# Add third-party dependencies first
add_subdirectory(third_party)

# Clang-Tidy integration
option(ENABLE_CLANG_TIDY "Enable clang-tidy checks during build" ON)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" "clang-tidy-*")
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        # Only apply clang-tidy to our own source files
        # The CMAKE_CXX_CLANG_TIDY variable applies to all targets, which is not what we want
        # We'll set this per-target instead
        set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()

option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)

add_subdirectory(src)

# Apply clang-tidy to our core library (but not external dependencies)
if(DEFINED CLANG_TIDY_COMMAND)
    set_target_properties(iv_core PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    set_target_properties(iv_calculator PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Add clang-format target
find_program(CLANG_FORMAT_EXE NAMES "clang-format" "clang-format-*")
if(CLANG_FORMAT_EXE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")
    
    # Get all source files
    file(GLOB_RECURSE ALL_SOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/src/*.cpp 
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/include/*.h
        ${CMAKE_SOURCE_DIR}/tests/*.cpp
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on all source files"
        VERBATIM
    )
else()
    message(WARNING "clang-format not found!")
endif()
