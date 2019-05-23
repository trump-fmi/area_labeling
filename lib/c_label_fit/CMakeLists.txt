cmake_minimum_required(VERSION 3.14)
project(label_fit LANGUAGES CXX)

FIND_PACKAGE(PythonInterp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_executable(label_fit label_fit.cpp)
target_link_libraries(label_fit CGAL gmp mpfr)

PYTHON_ADD_MODULE(c_label_fit py_label_fit.cpp)
target_INCLUDE_DIRECTORIES(c_label_fit PUBLIC ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_LINK_LIBRARIES(c_label_fit CGAL gmp mpfr ${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!

