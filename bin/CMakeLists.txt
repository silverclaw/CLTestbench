find_package(PkgConfig)

add_executable(cltb main.cpp)
target_link_libraries(cltb cltb_objs)

if (PkgConfig_FOUND)
pkg_check_modules(LIBEDIT IMPORTED_TARGET libedit)
pkg_check_modules(READLINE IMPORTED_TARGET readline)

if (NOT LIBEDIT_FOUND AND NOT READLINE_FOUND)
    message(SEND_ERROR "One of libedit or readline must be available.")
endif ()

include(CMakeDependentOption)
cmake_dependent_option(CLTB_USE_LIBEDIT "Use libedit for line input." OFF
                       "LIBEDIT_FOUND" OFF)
cmake_dependent_option(CLTB_USE_READLINE "Use readline for line input." OFF
                       "READLINE_FOUND" OFF)

if (LIBEDIT_FOUND AND NOT READLINE_FOUND)
    set(CLTB_USE_LIBEDIT ON FORCE)
elseif (NOT LIBEDIT_FOUND AND READLINE_FOUND)
    set(CLTB_USE_READLINE ON FORCE)
elseif (LIBEDIT_FOUND AND READLINE_FOUND)
    # Both found - pick Readline to start.
    set(CLTB_USE_READLINE ON)
endif ()

if (CLTB_USE_LIBEDIT)
    target_link_libraries(cltb PkgConfig::LIBEDIT)
elseif (CLTB_USE_READLINE)
    target_link_libraries(cltb PkgConfig::READLINE)
endif ()

endif (PkgConfig_FOUND)

set_target_properties(cltb PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)

install(TARGETS cltb)

