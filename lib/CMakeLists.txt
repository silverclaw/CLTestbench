find_package(PNG)
include(CMakeDependentOption)
cmake_dependent_option(USE_LIBPNG "Add support for loading PNG files." ON
                       "PNG_FOUND" OFF)

add_library(cltb_objs OBJECT
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
get_target_property(KHRONOS_HEADERS OpenCL::Headers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(cltb_objs SYSTEM PUBLIC ${KHRONOS_HEADERS})

target_compile_definitions(cltb_objs PUBLIC CL_TARGET_OPENCL_VERSION=220)

set_target_properties(cltb_objs PROPERTIES
    CXX_STANDARD 17
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)

configure_file(cltb_config.h.in cltb_config.h)

if (USE_LIBPNG)
    target_link_libraries(cltb_objs PUBLIC PNG::PNG)
endif (USE_LIBPNG)

target_link_libraries(cltb_objs PUBLIC ${CMAKE_DL_LIBS})

