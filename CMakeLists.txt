cmake_minimum_required(VERSION 3.21)
project(W20PP 
    VERSION 0.1.0
    DESCRIPTION "Modern C++20 Windows UI Framework"
    LANGUAGES CXX
)

# Require C++20 minimum (C++23 preferred)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# -----------------------------------------------------------------------------
# W20PP Header-Only Library
# -----------------------------------------------------------------------------
add_library(w20pp INTERFACE)
add_library(W20PP::W20PP ALIAS w20pp)

target_include_directories(w20pp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(w20pp INTERFACE cxx_std_20)

# Windows-specific configuration
if(WIN32)
    target_compile_definitions(w20pp INTERFACE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
    
    # Link Windows UI libraries
    target_link_libraries(w20pp INTERFACE
        user32
        gdi32
        dwmapi
        uxtheme
        comctl32
        shcore
    )
endif()

# MSVC-specific flags
if(MSVC)
    target_compile_options(w20pp INTERFACE
        /W4
        /permissive-
        /Zc:__cplusplus
        /Zc:preprocessor
    )
else()
    target_compile_options(w20pp INTERFACE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# -----------------------------------------------------------------------------
# Examples (optional)
# -----------------------------------------------------------------------------
option(W20PP_BUILD_EXAMPLES "Build example applications" ON)

if(W20PP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# -----------------------------------------------------------------------------
# Tests (optional)
# -----------------------------------------------------------------------------
option(W20PP_BUILD_TESTS "Build unit tests" OFF)

if(W20PP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
