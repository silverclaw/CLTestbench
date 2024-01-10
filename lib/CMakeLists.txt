find_package(PNG)
include(CMakeDependentOption)
cmake_dependent_option(CLTB_USE_LIBPNG "Add support for loading PNG files." ON
                       "PNG_FOUND" OFF)

add_library(cltb_objs OBJECT
            bind.cpp
            clone.cpp
            driver.cpp
            istringview.cpp
            library.cpp
            token.cpp
            pngwrap.cpp
            testbench.cpp
            info.cpp
            select.cpp
            eval.cpp
            program.cpp
            kernel.cpp
            buffer.cpp
            image.cpp
            file.cpp
            save.cpp
            script.cpp
            options.cpp
            run.cpp
            help.cpp)

target_include_directories(cltb_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(cltb_objs SYSTEM PUBLIC ${OpenCL_INCLUDE_DIR})
# CLTestbench requires a somewhat basic OpenCL version.
target_compile_definitions(cltb_objs PUBLIC CL_TARGET_OPENCL_VERSION=220)

set_target_properties(cltb_objs PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 17
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)

configure_file(cltb_config.h.in cltb_config.h)

if (CLTB_USE_LIBPNG AND PNG_FOUND)
    target_link_libraries(cltb_objs PUBLIC PNG::PNG)
endif (CLTB_USE_LIBPNG AND PNG_FOUND)

target_link_libraries(cltb_objs PUBLIC ${CMAKE_DL_LIBS})

add_library(CLIntercept SHARED
    intercept.cpp
    library.cpp)

set_target_properties(CLIntercept PROPERTIES
    CXX_STANDARD 17
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)

# Target an arbitrary API version to implement.  We're not looking for conformance,
# so it ultimately won't matter.
target_compile_definitions(CLIntercept PUBLIC CL_TARGET_OPENCL_VERSION=300)
target_include_directories(CLIntercept SYSTEM PUBLIC ${OpenCL_INCLUDE_DIR})
