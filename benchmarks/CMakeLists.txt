# Find Google Benchmark
find_package(benchmark REQUIRED)

if(benchmark_FOUND)
    message(STATUS "Google Benchmark found: ${benchmark_VERSION}")
    
    # Benchmark for Black-Scholes core calculations
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/black_scholes_benchmark.cpp")
        add_executable(black_scholes_benchmark
            black_scholes_benchmark.cpp
        )
        
        target_link_libraries(black_scholes_benchmark
            iv_core
            benchmark::benchmark
        )
    endif()
    
    # Benchmark for file IO operations  
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file_io_benchmark.cpp")
        add_executable(file_io_benchmark
            file_io_benchmark.cpp
        )
        
        target_link_libraries(file_io_benchmark
            iv_core
            benchmark::benchmark
        )
    endif()
    
else()
    message(WARNING "Google Benchmark not found, benchmarks will not be built")
endif()
