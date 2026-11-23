[CmdletBinding()]
param(
    [string]$Src = "res\photo",
    [int]$FullW = 800,
    [int]$FullH = 480,
    [int]$ThumbW = 220,
    [int]$ThumbH = 124,
    [string]$Background = "#0B1220"
)

# 把 res/photo 下的 jpg/png 批量转成相册用的 24 位 BMP：
#   photo_N.bmp  800x480 全图
#   thumb_N.bmp  220x124 缩略图
# 原图保留，不删除。

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = "Stop"

if (-not (Test-Path $Src)) { throw "目录不存在: $Src" }

$files = Get-ChildItem $Src -File |
    Where-Object { $_.Extension -match '^\.(jpg|jpeg|png)$' } |
    Sort-Object Name

if ($files.Count -eq 0) { throw "$Src 下没有 jpg/png" }

$n = 0
foreach ($f in $files) {
    $n++
    $img = [System.Drawing.Image]::FromFile($f.FullName)
    try {
        foreach ($spec in @(
            @{ Prefix = "photo_"; W = $FullW;  H = $FullH  },
            @{ Prefix = "thumb_"; W = $ThumbW; H = $ThumbH }
        )) {
            $w = $spec.W
            $h = $spec.H
            $scale = [Math]::Min($w / $img.Width, $h / $img.Height)
            $dw = [int][Math]::Round($img.Width * $scale)
            $dh = [int][Math]::Round($img.Height * $scale)
            $dx = [int](($w - $dw) / 2)
            $dy = [int](($h - $dh) / 2)

            $bmp = New-Object System.Drawing.Bitmap -ArgumentList $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            $g.Clear([System.Drawing.ColorTranslator]::FromHtml($Background))
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
            $g.DrawImage($img, $dx, $dy, $dw, $dh)
            $g.Dispose()

            $out = Join-Path $Src ("{0}{1}.bmp" -f $spec.Prefix, $n)
            $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Bmp)
            $bmp.Dispose()
            Write-Host ("[OK] {0} -> {1} ({2}x{3})" -f $f.Name, (Split-Path $out -Leaf), $w, $h)
        }
    }
    finally {
        $img.Dispose()
    }
}

Write-Host ("[DONE] {0} 张，生成 photo_N.bmp / thumb_N.bmp" -f $n)
