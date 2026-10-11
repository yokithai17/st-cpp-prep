cmake_minimum_required(VERSION 3.20)
project(st_cpp_prep LANGUAGES CXX)

add_executable(environment_check tools/environment_check.cpp)
target_compile_features(environment_check PRIVATE cxx_std_20)
set_target_properties(environment_check PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(environment_check PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(environment_check PRIVATE /W4 /Zc:__cplusplus)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME environment_check COMMAND environment_check)
endif()
