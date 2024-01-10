cmake_minimum_required(VERSION 3.14)
# CMP0063 - respect the visibility policy for all targets.
cmake_policy(SET CMP0063 NEW)
project(CLTestbench LANGUAGES C CXX VERSION 0.1)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "No build type selected, defaulting to Debug.")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)." FORCE)
endif()

find_package(OpenCL)
if (NOT OpenCL_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        OpenCLHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(OpenCLHeaders)
    get_target_property(OpenCL_INCLUDE_DIR OpenCL::Headers INTERFACE_INCLUDE_DIRECTORIES)
endif()

find_program(CCACHE_BIN ccache DOC "Path to CCache binary")
if (CCACHE_BIN)
    option(USE_CCACHE "Enable use of CCache." ON)
    if (USE_CCACHE)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_BIN})
    endif(USE_CCACHE)
endif(CCACHE_BIN)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(test)
