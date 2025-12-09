add_library(iv_core
    core/black_scholes.cpp
    io/file_io.cpp
)

# Public includes are in the include directory
target_include_directories(iv_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link simdjson to our core library
target_link_libraries(iv_core PRIVATE -lsimdjson)

# Add simdjson include directories
target_include_directories(iv_core PRIVATE ${SIMDJSON_INCLUDE_DIRS})

# Create console application
add_executable(iv_calculator
    interface/cli.cpp
)

target_link_libraries(iv_calculator PRIVATE iv_core)
