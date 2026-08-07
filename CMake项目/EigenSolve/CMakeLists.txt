cmake_minimum_required(VERSION 3.16)

project(EigenSolve LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(ROOT REQUIRED COMPONENTS Matrix MathCore)

add_executable(EigenSolve
    src/main.cpp
    src/MatrixSolver.cpp
)

target_include_directories(EigenSolve PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EigenSolve PRIVATE
    Eigen3::Eigen
    ROOT::Matrix
    ROOT::MathCore
)
