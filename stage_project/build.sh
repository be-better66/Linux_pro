#!/bin/sh
# GEC6818 智能车载中控 - Ubuntu 交叉编译脚本
set -e

cd "$(dirname "$0")"

BUILD_DIR="${BUILD_DIR:-build}"

cmake -S . -B "$BUILD_DIR" \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-linux-toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" -j"${JOBS:-4}"

echo ""
echo "编译完成：$BUILD_DIR/bin/carmode"
echo "查看文件类型：file $BUILD_DIR/bin/carmode"
