#!/bin/sh
# 把 jpg/png 批量转换成 GEC6818 可用的 800x480、24 位、无压缩 BMP
#
# 用法：
#   bash tools/prepare_images.sh <源目录> [输出目录] [文件名前缀]
# 例：
#   bash tools/prepare_images.sh ~/Pictures/car res/car img
#
# 依赖：ImageMagick（sudo apt install imagemagick）
set -e

SRC=${1:?用法: bash tools/prepare_images.sh <源目录> [输出目录] [前缀]}
DST=${2:-res/car}
PREFIX=${3:-img}

if command -v magick >/dev/null 2>&1; then
    MAGICK=magick
elif command -v convert >/dev/null 2>&1; then
    MAGICK=convert
else
    echo "未找到 ImageMagick，请先安装：sudo apt install imagemagick" >&2
    exit 1
fi

mkdir -p "$DST"

n=1
found=0
for f in "$SRC"/*.jpg "$SRC"/*.jpeg "$SRC"/*.png "$SRC"/*.bmp; do
    [ -e "$f" ] || continue
    found=1
    out="$DST/$PREFIX$n.bmp"
    # BMP3: 强制 24 位 BMP；-extent 800x480 居中裁剪
    "$MAGICK" "$f" -auto-orient -resize '800x480^' -gravity center \
        -extent 800x480 -type TrueColor -depth 8 "BMP3:$out"
    echo "[OK] $f -> $out"
    n=$((n + 1))
done

if [ "$found" -eq 0 ]; then
    echo "源目录里没有 jpg/png/bmp: $SRC" >&2
    exit 1
fi

echo ""
echo "完成，共 $((n - 1)) 张。记得把新文件名写进 res/car/car.cfg 的对应路径。"
