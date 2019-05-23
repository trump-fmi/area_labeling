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

add_executable(circle_apx circle_apx.cpp)
target_LINK_LIBRARIES(circle_apx nlopt)

PYTHON_ADD_MODULE(c_circle_apx py_circle_apx.cpp)
target_INCLUDE_DIRECTORIES(c_circle_apx PUBLIC ${BOOST_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_LINK_LIBRARIES(c_circle_apx ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} nlopt) # Deprecated but so convenient!
