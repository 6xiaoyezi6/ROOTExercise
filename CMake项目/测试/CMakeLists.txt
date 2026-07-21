cmake_minimum_required(VERSION 3.16)

### 定义项目,项目名称是 DrawEnergyLossGraph 并且只使用C++（LANGUAGES CXX可以省去）
#project(DrawEnergyLossGraph)也是可以的
project(DrawEnergyLossGraph LANGUAGES CXX)

# 要求编译器使用C++17
#set(变量名 变量值)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") ### 可执行文件放到 build/bin

find_package(ROOT REQUIRED COMPONENTS Core RIO Graf Gpad)


# 把两个.cpp 编译成可执行程序 编译成的程序名字EnergyLossTGraph+要编译的.cpp文件
# add_executable(可执行程序名 源文件1名称；源文件2名称) 可执行程序名和源文件名之间是空格，源文件名之间可以是空格也可以是分号;
# add_executable(EnergyLossTGraph src/main.cpp;src/GraphPlotter.cpp)可以用下面的命令代替
#set(src_list src/main.cpp;src/GraphPlotter.cpp)
#add_executable(EnergyLossTGraph ${src_list}) # ${}是固定用法 

#也可以用自动搜索功能 aux_source_directory(<目录> <变量名>)
aux_source_directory(src src_list) #自动搜索src目录下的源码，然后存入src_list变量中
add_executable(EnergyLossTGraph ${src_list})




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
