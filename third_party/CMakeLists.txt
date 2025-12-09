# Try to find system-installed simdjson first - this should work with Ubuntu's libsimdjson-dev
find_package(simdjson QUIET)

if(simdjson_FOUND)
    message(STATUS "simdjson found in system: ${simdjson_VERSION}")
    set(SIMDJSON_FOUND TRUE)
else()
    # Manual configuration for cases where find_package doesn't work
    message(STATUS "simdjson not found via find_package, trying manual configuration...")
    
    # Check if the library files exist (Ubuntu package layout)
    if(EXISTS "/usr/include/simdjson.h" AND EXISTS "/usr/lib/x86_64-linux-gnu/libsimdjson.so")
        message(STATUS "Found simdjson in standard Ubuntu locations")
        
        # Set the library and include paths directly
        set(SIMDJSON_LIBRARY "/usr/lib/x86_64-linux-gnu/libsimdjson.so")
        set(SIMDJSON_INCLUDE_DIR "/usr/include")
        
        message(STATUS "Using simdjson library: ${SIMDJSON_LIBRARY}")
        message(STATUS "Using simdjson includes: ${SIMDJSON_INCLUDE_DIR}")
        
        # Set variables for use elsewhere
        set(SIMDJSON_FOUND TRUE)
        set(SIMDJSON_LIBRARIES ${SIMDJSON_LIBRARY})
        set(SIMDJSON_INCLUDE_DIRS ${SIMDJSON_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "simdjson library not found. Please install it using:\n"
                           "  sudo apt-get install libsimdjson-dev")
    endif()
endif()
