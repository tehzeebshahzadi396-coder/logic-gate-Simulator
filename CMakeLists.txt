cmake_minimum_required(VERSION 3.10)
project(DigitalCircuitSimulator 
    VERSION 1.0.0
    DESCRIPTION "Digital Circuit & Logic Gate Simulator in C++17"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings across platforms
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Core static library
add_library(circuit_sim_lib STATIC
    src/Wire.cpp
    src/Gate.cpp
    src/Gates.cpp
    src/Circuit.cpp
    src/NetlistParser.cpp
    src/TruthTableGenerator.cpp
)

target_include_directories(circuit_sim_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Main command-line application
add_executable(logic_sim src/main.cpp)
target_link_libraries(logic_sim PRIVATE circuit_sim_lib)

# Test runner executable
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE circuit_sim_lib)

enable_testing()
add_test(NAME CircuitSimulatorTestSuite COMMAND run_tests)
set_tests_properties(CircuitSimulatorTestSuite PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
