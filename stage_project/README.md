# GEC6818 智能车载中控（Smart Cockpit）

> Linux 阶段性小项目：framebuffer 显示 + 触摸 + BMP 解码 + 中文 TTF + CMake / Makefile

本工程按 `inc/ + src/ + res/` 的结构组织，做成了一个 **车载中控屏** 的阶段性项目：

- 主页六宫格：导航 / 仪表 / 音乐 / 电话 / 相册 / 设置
- 仪表盘：车速表 + 转速表 + 指针动画 + 数字车速 + 挡位 + 转向灯
- 音乐页：专辑封面、歌名、进度条、上一首/播放暂停/下一首（默认尝试 madplay 出声，没有则自动模拟）
- 相册页：3 列缩略图、点击查看、左右切换、右上角关闭（参考多媒体工程的相册）
- 倒车影像：占位画面 + 倒车辅助线
- 日夜两套主题、页面切换（`page_anim` 可配滑动转场）、配置驱动

> 项目书：[docs/项目书.md](docs/项目书.md)（插图占位见 `docs/figures/`）

## 1. 功能清单

| 页面 | 功能 |
| --- | --- |
| 主页 | 6 个功能卡片、状态栏（时间/日期/温度/音量/信号）、右下角昼夜切换 |
| 导航 | 代码绘制的模拟地图、路线、目的地、距离/时间、结束导航 |
| 仪表 | 车速表 0~240、转速表 0~8、指针动画、数字车速、P/R/N/D 挡位、左右转向灯、油量/水温/里程 |
| 音乐 | 专辑封面、歌名/歌手、进度条、上一首/播放暂停/下一首、音量；按配置的 1/2/4 秒自动切歌 |
| 相册 | 3 列缩略图、点击看大图、左右滑动/点两侧切换、右上角关闭、左上角返回 |
| 电话 | 联系人列表、选择联系人、呼叫/挂断（模拟，不真正拨号） |
| 设置 | 主题、亮度、音量、仪表演示开关、进入倒车影像 |
| 倒车 | 全屏倒车画面 + 绿色/黄色/红色辅助线，点击返回 |
| 全局 | 左右滑动切页、左上角返回、页面切换（默认直接切换，可配滑动转场）、夜间主题 |

## 2. 操作说明

| 操作 | 效果 |
| --- | --- |
| 点击主页卡片 | 进入导航 / 仪表 / 音乐 / 电话 / 相册 / 设置 |
| 主页右下角“白天/夜间” | 切换昼夜主题 |
| 左右滑动屏幕 | 上一页 / 下一页（音乐页例外，见下） |
| 点击左上角 | 返回主页（倒车页返回仪表） |
| 仪表页点击表盘 | 开关车速演示动画 |
| 仪表页点击 P/R/N/D | 切换挡位（会关闭演示） |
| 仪表页点击左上/右上转向灯 | 开关左/右转向灯 |
| 音乐页点击播放键 | 播放/暂停（audio=1 时尝试用 madplay 出声，没有则只走界面） |
| 音乐页左右滑动 | 上一首 / 下一首 |
| 音乐页点击进度条 | 跳转播放位置（界面模拟） |
| 音乐页 +/- | 调节界面音量 |
| 相册页点击缩略图 | 打开全屏大图 |
| 相册页全屏左右滑动 | 上一张 / 下一张 |
| 相册页全屏点左/右半屏 | 上一张 / 下一张 |
| 相册页全屏点右上角 × | 回到缩略图 |
| 电话页点击联系人 | 选中联系人 |
| 电话页点击“呼叫” | 进入“正在呼叫”界面，可挂断 |
| 设置页点击“主题”整行 | 昼夜切换 |
| 设置页点击 +/- | 调节亮度和音量（模拟） |
| 设置页点击“倒车影像” | 进入倒车画面，点击任意位置返回 |

## 3. 目录结构

```
stage_project/
├── CMakeLists.txt
├── Makefile
├── build.sh
├── cmake/arm-linux-toolchain.cmake
├── inc/
│   ├── lcd.h                  # /dev/fb0
│   ├── touch.h                # /dev/input/event0
│   ├── bmp.h                  # BMP 解码
│   ├── text.h                 # 中文 TTF
│   ├── car.h                  # 车载配置/状态/页面接口
│   ├── car_photo.h            # 相册页接口
│   ├── mp3.h                  # 音乐控制接口
│   └── truetype.h             # stb_truetype
├── src/
│   ├── lcd.c
│   ├── touch.c
│   ├── bmp.c
│   ├── text.c
│   ├── truetype.c
│   ├── car.c                  # 主循环、页面切换、转场
│   ├── car_pages.c            # 主页/导航/仪表/音乐/电话/设置/倒车
│   ├── car_ui.c               # 绘图原语、状态栏、提示栏
│   ├── car_cfg.c              # 车载配置解析
│   ├── car_photo.c            # 相册：扫描/缩略图/全屏切换
│   ├── mp3.c                  # 音乐控制（madplay 子进程 + 播放状态）
│   └── main.c
├── res/
│   ├── car/                   # 车载图片 + car.cfg + 相册图标 icon_pic.bmp
│   ├── music/                 # 1.mp3 2.mp3 3.mp3
│   ├── photo/                 # photo_N.bmp 全图 + thumb_N.bmp 缩略图
│   └── font/simfang.ttf
├── lib/                       # 第三方库目录（当前为空，见 README）
├── tools/                     # 素材转换/检查脚本
└── docs/
    ├── 项目书.md
    ├── figures/               # 项目书插图（作者补充）
    ├── 相册说明.md
    └── 上板测试清单.md
```

> 和参考工程 `project/` 一样：`inc/` 放头文件，`src/` 下所有 `.c` 直接编译成一个可执行文件，
> 不再区分 `drivers/` 和 `app/`，也没有静态库步骤。

## 4. 开发环境

- 开发板：GEC6818，800×480，32bpp framebuffer
- 主机：Ubuntu 20.04（或装有交叉工具链的 Linux）
- 交叉编译器：`arm-linux-gcc`（或 `arm-linux-gnueabi-gcc`）
- CMake ≥ 3.10，GNU make
- 音频：默认尝试用 `madplay` 播放 `res/music/` 下的 2~3 秒短音频；没有 madplay 时自动纯模拟
- 不需要 libjpeg

## 5. 编译

> **CMake 和 Makefile 是两套等价方案，任选一套，不是先后步骤。**
> 下面的打包/部署流程使用 Makefile，因为 `make pack` 会直接把 `carmode` 和 `res/` 打成一个包。

| 方式 | 命令 | 输出 |
| --- | --- | --- |
| CMake | `cmake -S . -B build ...` + `cmake --build build -j4` | `build/bin/carmode` |
| Makefile | `make -j4` | `out/carmode` |

> 两套不要同时执行；混用只会多出一个 `build/` 或 `out/` 目录，二进制内容是一样的。

### 5.1 CMake（可选，二选一）

```bash
cd stage_project
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-linux-toolchain.cmake
cmake --build build -j4
```

或者：

```bash
sh build.sh
```

输出：`build/bin/carmode`

### 5.2 Makefile（后面打包/部署用这套）

```bash
cd stage_project
make -j4             # 输出 out/carmode
make pack            # 输出 out/carmode.tar.gz（carmode + res）
make STATIC=1        # 静态链接
make clean
```

> 所有 `src/*.c` 直接编译进 `carmode`，没有额外的库步骤，和参考工程的
> `arm-linux-gcc ./src/*.c -o project -I ./inc` 思路一致。

> 如果工程在 VMware 共享目录（`/mnt/hgfs/...`），`make pack` 会先把内容复制到
> `/tmp/carmode_pkg` 再打包，避免 tar 报 `file changed as we read it`。

## 6. 部署与运行

> 完整的编译、部署、逐页测试清单见 **[docs/上板测试清单.md](docs/上板测试清单.md)**。

把 **carmode 和 res/ 放在同一级目录**，在包含 `res/` 的目录运行：

```
carmode/
├── carmode
└── res/
    ├── car/
    ├── music/
    └── font/
```

```bash
cd /root/carmode
./carmode
```

权限不足时：

```bash
chmod 666 /dev/fb0
chmod 666 /dev/input/event0
```

### tftp

PC 端在项目根目录启动 tftp 服务，开发板：

```bash
tftp -g -r carmode 192.168.1.3
chmod +x carmode
tftp -g -r carmode.tar.gz 192.168.1.3
tar xzf carmode.tar.gz
```

生成压缩包：

```bash
make pack
# out/carmode.tar.gz，解压后是 carmode + res/
```

### NFS

```bash
mount -t nfs -o nolock 192.168.1.3:/home/xxx/stage_project /mnt
cd /mnt
./build/bin/carmode
```

## 7. 配置文件

文件：`res/car/car.cfg`

| 键 | 含义 | 示例 |
| --- | --- | --- |
| `night` | 0 白天 / 1 夜间 | `night=0` |
| `volume` | 界面音量 0~30 | `volume=12` |
| `temperature` | 状态栏温度 16~30 | `temperature=24` |
| `demo` | 1 进入仪表自动演示 | `demo=1` |
| `audio` | 1 尝试 madplay 出声（默认）/ 0 纯模拟 | `audio=1` |
| `page_anim` | 1 页面滑动转场 / 0 直接切换（默认，避免闪屏） | `page_anim=0` |
| `font` | 字库路径 | `res/font/simfang.ttf` |
| `logo` | 开机 Logo | `res/car/logo.bmp` |
| `bg_day` / `bg_night` | 白天/夜间主页背景 | `res/car/bg_day.bmp` |
| `icon_nav` / `icon_dash` | 导航/仪表图标 | `res/car/icon_nav.bmp` |
| `icon_music` / `icon_phone` / `icon_setting` | 音乐/电话/设置图标 | ... |
| `icon_pic` | 相册图标 | `res/car/icon_pic.bmp` |
| `icon_sun` / `icon_moon` | 昼夜小图标 | 24×24，可选 |
| `dash_bg` | 仪表盘背景 | 800×480，可选 |
| `album` | 专辑封面 | 300×300 |
| `photo_dir` | 相册目录 | `res/photo` |
| `reverse` | 倒车画面 | 800×480 |
| `song` | 标题\|歌手\|路径\|秒数 | 见下 |

歌曲格式：

```ini
song=夜空中最亮的星|逃跑计划|res/music/1.mp3|3
```

> 主题、亮度、音量在界面上的修改只在本次运行有效；要永久生效，直接改 `car.cfg`。

## 8. 素材：放哪里、多大

快速版：

| 文件 | 尺寸 | 必须 | 用途 |
| --- | --- | --- | --- |
| `res/car/bg_day.bmp` | 800×480 | 是 | 白天主页背景 |
| `res/car/bg_night.bmp` | 800×480 | 是 | 夜间主页背景 |
| `res/car/logo.bmp` | 400×200 | 是 | 开机 Logo |
| `res/car/icon_nav.bmp` | 96×96 | 是 | 导航图标 |
| `res/car/icon_dash.bmp` | 96×96 | 是 | 仪表图标 |
| `res/car/icon_music.bmp` | 96×96 | 是 | 音乐图标 |
| `res/car/icon_phone.bmp` | 96×96 | 是 | 电话图标 |
| `res/car/icon_setting.bmp` | 96×96 | 是 | 设置图标 |
| `res/car/icon_pic.bmp` | 96×96 | 是 | 相册图标 |
| `res/car/album.bmp` | 300×300 | 是 | 专辑封面 |
| `res/photo/photo_N.bmp` | 800×480 | 相册 | 全屏大图 |
| `res/photo/thumb_N.bmp` | 220×124 | 相册 | 缩略图 |
| `res/car/reverse.bmp` | 800×480 | 是 | 倒车画面 |
| `res/car/dash_bg.bmp` | 800×480 | 否 | 仪表盘背景 |
| `res/car/icon_sun.bmp` | 24×24 | 否 | 白天小图标 |
| `res/car/icon_moon.bmp` | 24×24 | 否 | 夜间小图标 |
| `res/music/1.mp3` 等 | — | 否 | 音乐 |

格式：**BMP、24 位、无压缩**。图标背景建议用 `#16283E`（和卡片一致）。

单张转换（Windows）：

```powershell
powershell -ExecutionPolicy Bypass -File tools\convert_one.ps1 `
    -In D:\素材\bg_day.jpg -Out res\car\bg_day.bmp -W 800 -H 480

powershell -ExecutionPolicy Bypass -File tools\convert_one.ps1 `
    -In D:\素材\nav.png -Out res\car\icon_nav.bmp -W 96 -H 96 -Background "#16283E"
```

批量转换：

```powershell
powershell -ExecutionPolicy Bypass -File tools\prepare_images.ps1 `
    -Source D:\素材\icons -Dest res\car -Prefix icon -Width 96 -Height 96
```

检查：

```powershell
powershell -ExecutionPolicy Bypass -File tools\check_bmp.ps1 -Dir res\car -Any
```

## 9. 代码结构

| 模块 | 文件 | 职责 | 关键知识点 |
| --- | --- | --- | --- |
| LCD | `src/lcd.c` | open `/dev/fb0`、ioctl、mmap、画点/矩形/贴图 | 文件 IO、mmap、显存 |
| 触摸 | `src/touch.c` | 读 `/dev/input/event0`，解析 EV_ABS/BTN_TOUCH，识别点击/滑动 | input 子系统、select |
| BMP | `src/bmp.c` | 解析 BMP、行对齐、BGR→RGB、翻转、居中 | 结构体、字节序 |
| 文字 | `src/text.c` | stb_truetype 渲染 UTF-8 中文，alpha 混合 | 字体、UTF-8 |
| 绘图 | `src/car_ui.c` | 圆角矩形、圆、圆弧、线段、三角形、状态栏 | 图形学基础 |
| 页面 | `src/car_pages.c` | 主页/导航/仪表/音乐/相册/电话/设置/倒车 | 状态机、布局 |
| 主循环 | `src/car.c` | 触摸分派、页面切换/转场、开机动画 | 事件循环、动画 |
| 相册 | `src/car_photo.c` | 扫描 `res/photo`、缩略图、全屏、上一张/下一张、关闭 | 目录遍历、BMP、触摸分区 |
| 音频控制 | `src/mp3.c` | 启动/停止 madplay、播放/暂停、上一首/下一首 | 子进程、system、信号 |
| 配置 | `src/car_cfg.c` | 解析 `car.cfg` 和歌曲列表 | 文件解析 |
| 构建 | `CMakeLists.txt` / `Makefile` | `src/*.c` 一次编译成 `carmode` | CMake/Makefile |

与参考工程的关系：

- `lcd.c` 参考 `3_开发板的应用/7_点击屏幕红变蓝` 与 `project/src/lcd.c`
- `touch.c` 参考 `3_开发板的应用/8_滑动方向`，增加 `EVIOCGABS` 自动量程和 select 超时
- `bmp.c` 参考 `project/src/bmp.c`，改为逐行读取、越界裁剪
- `text.c` 基于 `project` 的 `truetype.c`，改为直接混合到 LCD，字库只加载一次
- `mp3.c` 对应 `project/src/mp3.c`：保留 madplay 控制，去掉 `while(1)` 整屏阻塞循环，交给车机主状态机调用

## 10. 构建与测试状态

| 项目 | 结果 |
| --- | --- |
| 5 个驱动 .c + 7 个车载 .c（共 12 个）`-Wall -Wextra -fsyntax-only` | 全部通过 |
| BMP 解码 | 24/32 位、行填充、上下翻转、居中、缺图，单元测试通过 |
| 配置解析 | 主题/音量/温度/歌曲/UTF-8，单元测试通过（并修复了默认歌曲字符串被改写导致的崩溃） |
| LCD 绘图 | 越界裁剪、stride、混合、snapshot，单元测试通过 |
| 触摸 | 点击/四方向滑动/幽灵点击，单元测试通过 |
| 中文渲染 | 用 simfang 子集真实渲染中文，测量/像素/边界通过 |

> 当前环境没有 arm-linux-gcc，**还没有在真实 GEC6818 上跑过**。
> 第一次上板优先检查 `/dev/fb0`、`/dev/input/event0`、字库路径、运行目录。

## 11. 常见问题

**Q1：黑屏 / `open /dev/fb0` 失败**

```bash
ls /dev/fb*
cat /sys/class/graphics/fb0/bits_per_pixel
```

确认是 32bpp；16bpp 需要改 `lcd.c`。

**Q2：触摸没反应**

```bash
ls /dev/input/
cat /proc/bus/input/devices
```

不是 `event0` 就改 `touch.c` 的设备路径。触摸偏移时，`touch.c` 会通过 EVIOCGABS 自动读量程；X/Y 反了就交换映射。

**Q2-补充：点图标没反应 / 点成了滑动**

电阻屏点击时手指会有抖动，程序默认：

- 点击阈值 `TAP_SLOP = 60`；
- 60~100px 的“小滑动”按点击处理；
- 左上角返回键的命中区是 `80×52`，按钮本身也画大了。

如果还是不准，运行：

```bash
./carmode 2>&1 | tee run.log
```

点几下图标和左上角，把日志里的 `[touch] type=... x=... y=... start=...` 发我：

- `type=1` 是点击；
- `type=2/3` 是滑动；
- 如果 `x/y` 和你点的位置差很多，就是触摸坐标映射问题，需要校准。

调试日志在 `src/car.c` 的 `CAR_TOUCH_DEBUG`，改成 `0` 可关闭。

**Q3：图标显示成方块 / 位置不对**

- 必须是 **96×96、24 位 BMP**；尺寸不对会盖住标题。
- 图标背景建议 `#16283E`，和卡片一致。
- 用 `tools/check_bmp.ps1 -Dir res/car -Any` 先检查格式。

**Q4：音乐没声音**

默认 `audio=1` 会尝试播放，先检查：

1. `res/music/1.mp3`、`2.mp3`、`3.mp3` 是否真的存在；
2. 板子上有没有 `madplay`：

```bash
which madplay
```

3. `res/car/car.cfg` 里的 `song=` 路径和文件名是否一致。

没有 madplay 时不会响，但音乐页仍会正常模拟走秒和切歌。
如果本来就只想做界面模拟，把 `audio` 改成 `0` 即可。

**Q5：倒车/专辑封面没显示**

对应文件缺失时会用渐变/占位绘制，不会崩溃；按 `res/car/README_素材说明.txt` 放图即可。相册图按 `docs/相册说明.md`。

**Q6：主题/亮度/音量改了，重启又变回去**

界面修改只在本次运行有效；要永久生效，改 `res/car/car.cfg` 的 `night=0/1`、`volume=`、`temperature=`。

**Q7：编译报 `arm-linux-gcc: not found`**

```bash
make CROSS_COMPILE=/opt/toolchain/bin/arm-linux-
# 或
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-linux-toolchain.cmake -DCROSS_COMPILE=/opt/toolchain/bin/arm-linux-
```

**Q8：板子提示 `GLIBC_2.xx not found`**

换板子配套的工具链，或 `make STATIC=1` 静态链接。

**Q9：`make pack` 报 `tar: file changed as we read it`**

这是因为工程在 `/mnt/hgfs/...` 共享目录里，tar 读取时目录时间戳被 Windows 改动了。
现在 `Makefile` 的 `pack` 已经改成先复制到 `/tmp/carmode_pkg` 再打包，直接重新执行：

```bash
make pack
```

即可。最稳妥的做法还是把工程复制到 Ubuntu 的 home 目录再编译：

```bash
cp -r /mnt/hgfs/share/多媒体娱乐终端/stage_project ~/stage_project
cd ~/stage_project
make clean && make -j4 && make pack
```

**Q10：相册不显示缩略图 / 大图**

确认 `res/photo` 下同时有：

```text
photo_1.bmp ... photo_5.bmp   800x480
thumb_1.bmp ... thumb_5.bmp   220x124
```

重新生成：

```powershell
powershell -ExecutionPolicy Bypass -File tools\convert_photos.ps1 -Src res\photo
```

详细说明见 [docs/相册说明.md](docs/相册说明.md)。

## 12. 可以继续加的功能

- 真实 GPS / 地图瓦片（导航页现在是代码绘制的模拟地图）
- 真实蓝牙电话（需要 BlueZ）
- 倒车摄像头（V4L2 采集）
- 亮度写 `/sys/class/backlight/*/brightness`
- 配置保存回文件（主题/音量）
- 空调温度控制页
- 双闪/转向灯音效
- U 盘导入音乐/图片

