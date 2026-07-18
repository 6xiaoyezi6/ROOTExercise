cmake_minimum_required(VERSION 3.16)

### 定义项目,项目名称是 DrawEnergyLossGraph 并且只使用C++
project(DrawEnergyLossGraph LANGUAGES CXX)

# 要求编译器使用C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") ### 可执行文件放到 build/bin

find_package(ROOT REQUIRED COMPONENTS Core RIO Graf Gpad)


# 把两个.cpp 编译成可执行程序 编译成的程序名字EnergyLossTGraph+要编译的.cpp文件 
add_executable(EnergyLossTGraph
    src/main.cpp
    src/GraphPlotter.cpp
)

# 指定头文件目录
target_include_directories(EnergyLossTGraph PRIVATE include)

target_link_libraries(EnergyLossTGraph PRIVATE
    ROOT::Core
    ROOT::RIO
    ROOT::Graf
    ROOT::Gpad
)

# 创建output文件夹
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output")
