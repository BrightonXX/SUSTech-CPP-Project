# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(SimpleImageLib LANGUAGES CXX)

# --- 设置 C++ 标准 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 特定构建类型的标志
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mavx2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")


# --- 查找并启用 OpenMP ---
find_package(OpenMP)
if(OpenMP_FOUND)
    message(STATUS "OpenMP found, enabling parallel execution.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}") # 添加编译标志如 -fopenmp
    set(OpenMP_Target OpenMP::OpenMP_CXX) # 定义 OpenMP 目标
else()
    message(WARNING "OpenMP not found, parallel execution disabled.")
    set(OpenMP_Target "") 
endif()


# --- 定义共享库 ILib ---
add_library(ILib SHARED
    image_io.cpp
    image_proc.cpp
)

# --- 设置 ILib 的包含目录 ---
# 让 ILib 自身和链接到它的目标都能找到 image.h
target_include_directories(ILib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- 链接 ILib 库的依赖 (链接 OpenMP) ---
target_link_libraries(ILib PUBLIC ${OpenMP_Target})


# --- 定义可执行文件 demo ---
add_executable(demo demo.cpp)

# --- 链接 demo 程序到 ILib 库 ---
target_link_libraries(demo PRIVATE ILib)

# --- 输出编译标志，方便调试 ---
get_target_property(ILib_CXX_FLAGS ILib COMPILE_FLAGS)
message(STATUS "ILib Compile Flags: ${ILib_CXX_FLAGS}")
get_target_property(demo_CXX_FLAGS demo COMPILE_FLAGS)
message(STATUS "demo Compile Flags: ${demo_CXX_FLAGS}")
