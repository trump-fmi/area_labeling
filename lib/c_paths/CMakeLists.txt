cmake_minimum_required (VERSION 3.14)
project (circle-apx LANGUAGES CXX)


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

add_executable(longest_paths longest_paths.cpp)

PYTHON_ADD_MODULE(c_longest_paths py_longest_paths.cpp)
target_INCLUDE_DIRECTORIES(c_longest_paths PUBLIC ${BOOST_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_LINK_LIBRARIES(c_longest_paths ${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!
