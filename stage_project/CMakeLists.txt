# ============================================================
#  GEC6818 智能车载中控 - CMake 构建脚本
#
#  目录结构参考参考工程 project/：
#      inc/    头文件
#      src/    所有 .c（不再分 drivers/app）
#      res/    素材
#
#  交叉编译：
#     cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-linux-toolchain.cmake
#     cmake --build build -j4
# ============================================================

cmake_minimum_required(VERSION 3.10)

project(gec6818_car VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_compile_options(-Wall -Wextra -O2)

# 第三方 stb_truetype 关闭告警
set(TRUETYPE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/truetype.c)
set_source_files_properties(${TRUETYPE_SRC} PROPERTIES COMPILE_OPTIONS "-w")

# 和参考工程一样：src 下所有 .c 直接编译成一个可执行文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/log/log.c)

add_executable(carmode ${SOURCES})
target_include_directories(carmode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/log)
target_link_libraries(carmode PRIVATE m pthread)

set_target_properties(carmode PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "C compiler : ${CMAKE_C_COMPILER}")
message(STATUS "Output     : ${CMAKE_BINARY_DIR}/bin/carmode")
