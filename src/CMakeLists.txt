add_library(iv_core
    core/black_scholes.cpp
)

# Public includes are in the include directory
target_include_directories(iv_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Create console application
add_executable(iv_calculator
    interface/cli.cpp
)

target_link_libraries(iv_calculator PRIVATE iv_core)
