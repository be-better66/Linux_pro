[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$In,

    [Parameter(Mandatory = $true)]
    [string]$Out,

    [int]$W = 800,
    [int]$H = 480,

    [string]$Background = '#000000',

    [switch]$Contain
)

# 把一张图片转换成 GEC6818 工程可用的 24 位无压缩 BMP。
# 默认 Cover：等比放大并居中裁剪，铺满 W x H。
# -Contain ：等比缩小，四周补背景色。

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'

if (-not (Test-Path $In)) { throw "输入文件不存在: $In" }
if ($W -le 0 -or $H -le 0) { throw "尺寸不合法" }

$outDir = Split-Path -Parent $Out
if ($outDir -and -not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null
}

$img = [System.Drawing.Image]::FromFile((Resolve-Path $In).Path)
try {
    if ($Contain) {
        $scale = [Math]::Min($W / $img.Width, $H / $img.Height)
    } else {
        $scale = [Math]::Max($W / $img.Width, $H / $img.Height)
    }

    $dw = [int][Math]::Round($img.Width * $scale)
    $dh = [int][Math]::Round($img.Height * $scale)
    $dx = [int](($W - $dw) / 2)
    $dy = [int](($H - $dh) / 2)

    $bmp = New-Object System.Drawing.Bitmap -ArgumentList $W, $H, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.Clear([System.Drawing.ColorTranslator]::FromHtml($Background))
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $g.DrawImage($img, $dx, $dy, $dw, $dh)
    $g.Dispose()

    $bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Bmp)
    $bmp.Dispose()

    Write-Host ("[OK] {0} -> {1} ({2}x{3}, 24bpp)" -f (Split-Path $In -Leaf), $Out, $W, $H)
}
finally {
    $img.Dispose()
}
