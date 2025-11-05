include(FetchContent)

FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.13.0
)

FetchContent_MakeAvailable(simdjson)
set_target_properties(simdjson PROPERTIES CXX_CLANG_TIDY "")
