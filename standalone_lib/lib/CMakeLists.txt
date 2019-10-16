cmake_minimum_required (VERSION 3.13)
project (labeling LANGUAGES CXX)

ENABLE_TESTING()

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(CGAL REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)

pkg_check_modules(NLOpt REQUIRED nlopt)
# pkg_check_modules(GMP REQUIRED gmp)
pkg_check_modules(MPFR REQUIRED mpfr)

add_library(liblabeling SHARED
    liblabeling.cpp
)

target_link_libraries(liblabeling PUBLIC nlopt CGAL gmp mpfr)

target_compile_features(liblabeling
    PUBLIC cxx_std_17
)

target_include_directories(liblabeling 
    PUBLIC include
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/../lib/c_circle_apx
        ${CMAKE_SOURCE_DIR}/../lib/c_label_fit
        ${CMAKE_SOURCE_DIR}/../lib/c_paths
        ${CMAKE_SOURCE_DIR}/../lib/c_segments_to_graph
)
target_compile_definitions(liblabeling PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS)

set_target_properties(liblabeling
    PROPERTIES
        OUTPUT_NAME "labeling"
)
