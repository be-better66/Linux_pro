# GEC6818 交叉编译工具链配置
#
# 用法：
#   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux-toolchain.cmake
#
# 如果你的交叉编译器不叫 arm-linux-gcc，可以这样覆盖：
#   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux-toolchain.cmake \
#         -DCROSS_COMPILE=arm-linux-gnueabi-

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED CROSS_COMPILE)
    if(DEFINED ENV{CROSS_COMPILE})
        set(CROSS_COMPILE "$ENV{CROSS_COMPILE}")
    else()
        set(CROSS_COMPILE "arm-linux-")
    endif()
endif()

set(CMAKE_C_COMPILER   "${CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")
set(CMAKE_AR           "${CROSS_COMPILE}ar"     CACHE FILEPATH "")
set(CMAKE_RANLIB       "${CROSS_COMPILE}ranlib" CACHE FILEPATH "")

# 交叉编译时不做链接测试，避免缺少目标板库导致 configure 失败
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
