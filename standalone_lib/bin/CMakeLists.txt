cmake_minimum_required (VERSION 3.14)
project (app_labeling LANGUAGES CXX)

add_executable(labeling app.cpp)
target_LINK_LIBRARIES(labeling liblabeling)

add_executable(labeling2 app2.cpp)
target_LINK_LIBRARIES(labeling2 liblabeling)