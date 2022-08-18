option(BUILD_TESTING "Build tests" OFF)
if (BUILD_TESTING)

find_package(Catch2 3 QUIET)

if (NOT CATCH2_FOUND)
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.0
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif (NOT CATCH2_FOUND)

add_executable(tests
    test_dummydriver.cpp
    test_png.cpp
    test_images.cpp
    test_istringview.cpp
    test_dataobject.cpp
    test_tokens.cpp)

target_link_libraries(tests PRIVATE cltb_objs Catch2::Catch2WithMain)
set_target_properties(tests PROPERTIES
    CXX_STANDARD 17
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN on)


add_library(dummycl SHARED dummydriver.c)

target_compile_definitions(dummycl PUBLIC CL_TARGET_OPENCL_VERSION=220)
set_target_properties(dummycl PROPERTIES
    VISIBILITY_INLINES_HIDDEN on)


include(CTest)
include(Catch)

catch_discover_tests(tests)

endif (BUILD_TESTING)
