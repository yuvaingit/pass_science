cmake_minimum_required(VERSION 3.15)
project(UnifiedEngineeringWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization & warning flags
if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc /O2 /MD)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Include directories
include_directories(
    src/backend/include
)

# Core numerical & file processing static library (Task A, B, C, D, E, F)
add_library(workbench_core STATIC
    src/backend/src/stl_parser.cpp
    src/backend/src/ansys_csv_parser.cpp
    src/backend/src/thermal_solver.cpp
    src/backend/src/thread_pool.cpp
    src/backend/src/batch_solver.cpp
    src/backend/src/hardware_mock.cpp
    src/backend/src/backpropagation.cpp
)

# Threads library
find_package(Threads REQUIRED)
target_link_libraries(workbench_core PUBLIC Threads::Threads)

# Standalone Test Suite Executable
add_executable(workbench_tests
    tests/main_tests.cpp
)
target_link_libraries(workbench_tests PRIVATE workbench_core)

# Main Application Server / Launcher Executable
add_executable(workbench_server
    src/backend/src/main.cpp
)
target_link_libraries(workbench_server PRIVATE workbench_core)

if(WIN32)
    target_link_libraries(workbench_server PRIVATE ws2_32 shell32)

    # Windows 11 Native GUI Launcher (Launch.exe)
    add_executable(Launch WIN32
        src/launcher/launch_win.cpp
    )
    target_link_libraries(Launch PRIVATE shell32)
    set_target_properties(Launch PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/..")
endif()

# Output directories
set_target_properties(workbench_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(workbench_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

enable_testing()
add_test(NAME AllWorkbenchTests COMMAND workbench_tests)
