cmake_minimum_required(VERSION 3.13)
project(area_labeling VERSION 1.0)

add_subdirectory(c_circle_apx)
add_subdirectory(c_label_fit)
add_subdirectory(c_paths)
add_subdirectory(c_segments_to_graph)

install(TARGETS
        c_circle_apx
        c_label_fit
        c_longest_paths
        c_segments_to_graph
    DESTINATION ${PROJECT_BINARY_DIR}/bin
)
install(DIRECTORY python/
    DESTINATION ${PROJECT_BINARY_DIR}/bin
    FILES_MATCHING PATTERN "*.py")
