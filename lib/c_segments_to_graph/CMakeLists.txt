cmake_minimum_required (VERSION 3.14)
project (segments-to-graph)

FIND_PACKAGE(PythonInterp)

if (PYTHONINTERP_FOUND)
        FIND_PACKAGE(Boost COMPONENTS python3)
        FIND_PACKAGE(PythonInterp 3)
        FIND_PACKAGE(PythonLibs 3 REQUIRED)
else()
    message("Python not found")
endif()

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")

ENABLE_TESTING()

add_executable(segs_to_graph segments_to_graph.cpp)
target_link_libraries(segs_to_graph CGAL gmp mpfr)


PYTHON_ADD_MODULE(c_segments_to_graph py_segments_to_graph.cpp)
target_INCLUDE_DIRECTORIES(c_segments_to_graph PUBLIC ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_LINK_LIBRARIES(c_segments_to_graph CGAL gmp mpfr ${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!
