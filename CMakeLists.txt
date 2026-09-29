cmake_minimum_required(VERSION 3.12)
project(dcm-did-mvp C)

# Set C99 standard as required
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Add compiler flags for warnings and debugging
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/dcm_mvp.c
)

# Create library
add_library(dcm_mvp ${SOURCES})

# Enable testing
enable_testing()

# Add test executable
# NOTE: This template assumes Unity test framework or self-contained test harness
# Adjust path and link libraries based on chosen test framework
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_dcm_mvp.c")
    add_executable(test_dcm_mvp
        tests/test_dcm_mvp.c
        ${SOURCES}
    )
    
    # Link test framework if available
    # For Unity: uncomment and adjust path below
    # target_link_libraries(test_dcm_mvp unity)
    
    # Add test
    add_test(
        NAME test_dcm_mvp
        COMMAND test_dcm_mvp
    )
    
    # Enable verbose test output
    set_tests_properties(test_dcm_mvp PROPERTIES
        VERBOSE TRUE
    )
endif()

# Print build configuration
message(STATUS "Build configuration:")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
