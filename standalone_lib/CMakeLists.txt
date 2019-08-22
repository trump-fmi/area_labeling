cmake_minimum_required(VERSION 3.14)
project(lib_area_labeling VERSION 1.0)

# add_subdirectory(../lib/c_circle_apx)
# add_subdirectory(../lib/c_label_fit)
# add_subdirectory(../lib/c_paths)
# add_subdirectory(../lib/c_segments_to_graph)

add_subdirectory(lib)
add_subdirectory(bin)

install(TARGETS
        liblabeling
        labeling
    DESTINATION ${PROJECT_BINARY_DIR}/bin
)