[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Source,

    [string]$Dest = (Join-Path $PSScriptRoot '..\res\car'),

    [string]$Prefix = 'img',

    [int]$Start = 1,

    [int]$Width = 800,

    [int]$Height = 480,

    [switch]$Contain
)

# 把 jpg / png / bmp 批量转换成 GEC6818 可用的 800x480、24 位、无压缩 BMP
# 默认 Cover：等比放大并居中裁剪，铺满屏幕
# -Contain ：等比缩小并上下/左右补黑边

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'

if (-not (Test-Path $Source)) {
    throw "源目录不存在: $Source"
}
New-Item -ItemType Directory -Force -Path $Dest | Out-Null

$files = Get-ChildItem -Path $Source -File |
         Where-Object { $_.Extension -match '^\.(jpg|jpeg|png|bmp)$' } |
         Sort-Object Name

if ($files.Count -eq 0) {
    throw "源目录里没有 jpg/png/bmp: $Source"
}

$index = $Start
foreach ($f in $files) {
    $img = [System.Drawing.Image]::FromFile($f.FullName)
    try {
        if ($Contain) {
            $scale = [Math]::Min($Width / $img.Width, $Height / $img.Height)
        } else {
            $scale = [Math]::Max($Width / $img.Width, $Height / $img.Height)
        }

        $dw = [int][Math]::Round($img.Width * $scale)
        $dh = [int][Math]::Round($img.Height * $scale)
        $dx = [int](($Width - $dw) / 2)
        $dy = [int](($Height - $dh) / 2)

        $bmp = New-Object System.Drawing.Bitmap -ArgumentList $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.Clear([System.Drawing.Color]::Black)
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
        $g.DrawImage($img, $dx, $dy, $dw, $dh)
        $g.Dispose()

        $out = Join-Path $Dest ("{0}{1}.bmp" -f $Prefix, $index)
        $bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Bmp)
        $bmp.Dispose()

        Write-Host ("[OK] {0}  ->  {1}" -f $f.Name, $out)
        $index++
    }
    finally {
        $img.Dispose()
    }
}

Write-Host ""
Write-Host ("完成，共 {0} 张。记得把新文件名写进 res/car/car.cfg 的对应路径。" -f ($index - $Start))
