GEC6818 智能车载中控 - 素材说明
================================

所有图片统一要求：
  - BMP 格式
  - 24 位真彩色
  - 无压缩（BI_RGB）

文件名和尺寸：

  res/car/bg_day.bmp        800 x 480   白天主页背景
  res/car/bg_night.bmp      800 x 480   夜间主页背景
  res/car/logo.bmp          400 x 200   开机 Logo
  res/car/icon_nav.bmp       96 x 96     导航图标
  res/car/icon_dash.bmp      96 x 96     仪表图标
  res/car/icon_music.bmp     96 x 96     音乐图标
  res/car/icon_phone.bmp     96 x 96     电话图标
  res/car/icon_setting.bmp   96 x 96     设置图标
  res/car/icon_pic.bmp       96 x 96     相册图标
  res/car/icon_sun.bmp       24 x 24     白天图标（可选）
  res/car/icon_moon.bmp      24 x 24     夜间图标（可选）
  res/car/dash_bg.bmp       800 x 480   仪表盘背景（可选）
  res/car/album.bmp         300 x 300   专辑封面
  res/car/reverse.bmp       800 x 480   倒车影像占位图

图标建议：
  - 背景色用 #16283E（和卡片颜色一致），看起来像无缝贴在卡片上；
  - 图形主体用亮青色 #00C2FF 或白色；
  - 六个图标（导航/仪表/音乐/电话/相册/设置）风格保持一致。

相册资源：
  res/photo/photo_N.bmp      800 x 480   全屏大图
  res/photo/thumb_N.bmp      220 x 124   缩略图
  把 jpg/png 批量转成上述 BMP：
    powershell -ExecutionPolicy Bypass -File tools\convert_photos.ps1 -Src res\photo
  当前相册页按 3 列 x 2 行显示，最多 6 张。

替换方法：
  1. 用同名文件直接覆盖；
  2. 单张转换（推荐，能指定输出文件名）：

     powershell -ExecutionPolicy Bypass -File tools\convert_one.ps1 `
         -In D:\素材\bg_day.jpg -Out res\car\bg_day.bmp -W 800 -H 480

     powershell -ExecutionPolicy Bypass -File tools\convert_one.ps1 `
         -In D:\素材\nav.png -Out res\car\icon_nav.bmp `
         -W 96 -H 96 -Background "#16283E"

  3. 批量转换（同尺寸的一组图）：

     powershell -ExecutionPolicy Bypass -File tools\prepare_images.ps1 `
         -Source D:\素材\icons -Dest res\car -Prefix icon -Width 96 -Height 96

     输出的 icon1.bmp、icon2.bmp 需要自己改名成 icon_nav.bmp 等。
  4. 检查：

     powershell -ExecutionPolicy Bypass -File tools\check_bmp.ps1 -Dir res\car -Any

配置在 res/car/car.cfg。
