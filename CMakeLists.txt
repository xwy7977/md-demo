cmake_minimum_required(VERSION 3.10)
project(md-demo C)

set(CMAKE_C_STANDARD 99)

# 添加md4c子目录
add_subdirectory(md4c)

add_executable(md-demo main.c)

# 包含md4c头文件
target_include_directories(md-demo PRIVATE md4c)

# 链接md4c库
target_link_libraries(md-demo md4c)