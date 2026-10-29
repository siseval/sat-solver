cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(solver)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

add_executable(solver
        solver.cpp
        formula.cpp
        problem.cpp
        assignment.cpp
        main.cpp
)

target_compile_definitions(solver 
    PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
