GEC6818 车载相册资源
=====================

目录结构：

  photo_N.bmp   800 x 480   24 位无压缩 BMP，全屏查看用
  thumb_N.bmp   220 x 124   24 位无压缩 BMP，主页缩略图用

命名要求：

  - N 从 1 开始，程序按 N 的数字顺序排序；
  - 必须同时有 photo_N.bmp 和 thumb_N.bmp；
  - 当前相册页按 3 列 x 2 行显示，最多 6 张。

原始 1.jpg ~ 5.jpg 是素材原图，程序不直接读取，保留用于重新生成 BMP。

重新生成：

  powershell -ExecutionPolicy Bypass -File tools\convert_photos.ps1 -Src res\photo

  默认输出：
    photo_N.bmp  800x480
    thumb_N.bmp  220x124
    背景色 #0B1220

检查：

  powershell -ExecutionPolicy Bypass -File tools\check_bmp.ps1 -Dir res\photo -Any

配置：

  res/car/car.cfg 里的 photo_dir=res/photo
