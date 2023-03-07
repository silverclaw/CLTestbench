find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBEDIT IMPORTED_TARGET libedit)
pkg_check_modules(READLINE IMPORTED_TARGET readline)

if (NOT LIBEDIT_FOUND AND NOT READLINE_FOUND)
    message(SEND_ERROR "One of libedit or readline must be available.")
endif ()

include(CMakeDependentOption)
cmake_dependent_option(USE_LIBEDIT "Use libedit instead of readline." OFF
                       "LIBEDIT_FOUND;READLINE_FOUND" OFF)

if (LIBEDIT_FOUND AND NOT READLINE_FOUND)
    set(USE_LIBEDIT ON FORCE)
endif ()

add_executable(cltb main.cpp)
target_link_libraries(cltb cltb_objs)
if (USE_LIBEDIT)
    target_link_libraries(cltb PkgConfig::LIBEDIT)
else (USE_LIBEDIT)
    target_link_libraries(cltb PkgConfig::READLINE)
endif (USE_LIBEDIT)

set_target_properties(cltb PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)

install(TARGETS cltb)

