param(
    [string]$OutDir = (Join-Path $PSScriptRoot '..\res\car')
)

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'

function New-Canvas {
    param([int]$W,[int]$H,[string]$C1,[string]$C2,[double]$Angle = 45)
    $bmp = New-Object System.Drawing.Bitmap -ArgumentList $W,$H,([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $rect = New-Object System.Drawing.Rectangle -ArgumentList 0,0,$W,$H
    $a = [System.Drawing.ColorTranslator]::FromHtml($C1)
    $b = [System.Drawing.ColorTranslator]::FromHtml($C2)
    $br = New-Object System.Drawing.Drawing2D.LinearGradientBrush -ArgumentList $rect,$a,$b,$Angle
    $g.FillRectangle($br,$rect)
    $br.Dispose()
    return @{ Bitmap = $bmp; Graphics = $g; W = $W; H = $H }
}

function Add-Circle {
    param($G,[double]$X,[double]$Y,[double]$R,[System.Drawing.Color]$C)
    $b = New-Object System.Drawing.SolidBrush -ArgumentList $C
    $G.FillEllipse($b,[float]($X-$R),[float]($Y-$R),[float](2*$R),[float](2*$R))
    $b.Dispose()
}

function Add-Round {
    param($G,[double]$X,[double]$Y,[double]$W,[double]$H,[double]$R,[System.Drawing.Color]$C)
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = $R * 2
    $p.AddArc([float]$X,[float]$Y,[float]$d,[float]$d,180,90)
    $p.AddArc([float]($X+$W-$d),[float]$Y,[float]$d,[float]$d,270,90)
    $p.AddArc([float]($X+$W-$d),[float]($Y+$H-$d),[float]$d,[float]$d,0,90)
    $p.AddArc([float]$X,[float]($Y+$H-$d),[float]$d,[float]$d,90,90)
    $p.CloseFigure()
    $b = New-Object System.Drawing.SolidBrush -ArgumentList $C
    $G.FillPath($b,$p)
    $b.Dispose(); $p.Dispose()
}

function Add-Text {
    param($G,[string]$T,[int]$Size,[double]$X,[double]$Y,[double]$W,[double]$H,[string]$Hex,[string]$Style='Bold')
    $font = New-Object System.Drawing.Font -ArgumentList 'Microsoft YaHei',$Size,([System.Drawing.FontStyle]::$Style),([System.Drawing.GraphicsUnit]::Pixel)
    $brush = New-Object System.Drawing.SolidBrush -ArgumentList ([System.Drawing.ColorTranslator]::FromHtml($Hex))
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $rect = New-Object System.Drawing.RectangleF -ArgumentList ([float]$X),([float]$Y),([float]$W),([float]$H)
    $G.DrawString($T,$font,$brush,$rect,$sf)
    $sf.Dispose(); $brush.Dispose(); $font.Dispose()
}

function Save-Canvas {
    param($C,[string]$Path)
    $C.Graphics.Dispose()
    $C.Bitmap.Save($Path,[System.Drawing.Imaging.ImageFormat]::Bmp)
    $C.Bitmap.Dispose()
    Write-Host ("  " + (Split-Path $Path -Leaf))
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# ---------------- 白天背景 ----------------
$c = New-Canvas 800 480 '#07182B' '#0D3A5C' 30
$g = $c.Graphics
Add-Circle $g 690 90 180 ([System.Drawing.Color]::FromArgb(28,255,255,255))
Add-Circle $g 110 420 150 ([System.Drawing.Color]::FromArgb(22,0,194,255))
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(35,0,194,255)),2
$g.DrawLine($pen, 400, 210, 120, 480)
$g.DrawLine($pen, 400, 210, 680, 480)
$g.DrawLine($pen, 400, 210, 400, 480)
$pen.Dispose()
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(18,255,255,255)),1
for ($i=0; $i -lt 6; $i++) { $y = 300 + $i*30; $g.DrawLine($pen, 0, $y, 800, $y-40) }
$pen.Dispose()
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'bg_day.bmp')

# ---------------- 夜间背景 ----------------
$c = New-Canvas 800 480 '#02050A' '#0A1524' 30
$g = $c.Graphics
Add-Circle $g 680 100 170 ([System.Drawing.Color]::FromArgb(18,0,140,220))
Add-Circle $g 120 420 140 ([System.Drawing.Color]::FromArgb(14,0,100,180))
$rnd = New-Object System.Random 7
for ($i=0; $i -lt 70; $i++) {
    $x = $rnd.Next(0,800); $y = $rnd.Next(0,300)
    $a = $rnd.Next(20,70)
    Add-Circle $g $x $y 1 ([System.Drawing.Color]::FromArgb($a,255,255,255))
}
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(26,0,160,240)),2
$g.DrawLine($pen, 400, 210, 120, 480)
$g.DrawLine($pen, 400, 210, 680, 480)
$pen.Dispose()
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'bg_night.bmp')

# ---------------- 开机 Logo 400x200 ----------------
$c = New-Canvas 400 200 '#05080F' '#081522' 90
$g = $c.Graphics
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(255,0,180,255)),5
$g.DrawEllipse($pen, 156, 18, 88, 88)
$pen.Dispose()
Add-Round $g 190 38 20 48 7 ([System.Drawing.Color]::FromArgb(255,0,180,255))
Add-Text $g 'GEC AUTO' 30 20 112 360 40 '#FFFFFF'
Add-Text $g 'SMART COCKPIT SYSTEM' 10 20 158 360 20 '#5FA8C8'
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'logo.bmp')

# ---------------- 首页图标 96x96 ----------------
function New-Icon {
    param([string]$Name,[string]$Kind)
    $bmp = New-Object System.Drawing.Bitmap -ArgumentList 96,96,([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $card = [System.Drawing.Color]::FromArgb(255,22,40,62)
    $cyan = [System.Drawing.Color]::FromArgb(255,0,194,255)
    $g.Clear($card)
    Add-Round $g 0 0 96 96 18 $card
    $pen = New-Object System.Drawing.Pen -ArgumentList $cyan,4

    switch ($Kind) {
        'nav' {
            $g.DrawEllipse($pen, 23, 23, 50, 50)
            $p1 = New-Object System.Drawing.PointF -ArgumentList 48,26
            $p2 = New-Object System.Drawing.PointF -ArgumentList 62,62
            $p3 = New-Object System.Drawing.PointF -ArgumentList 48,54
            $p4 = New-Object System.Drawing.PointF -ArgumentList 34,62
            $pts = @($p1,$p2,$p3,$p4)
            $b = New-Object System.Drawing.SolidBrush -ArgumentList $cyan
            $g.FillPolygon($b,$pts)
            $b.Dispose()
        }
        'music' {
            $b = New-Object System.Drawing.SolidBrush -ArgumentList $cyan
            $g.FillEllipse($b, 22, 58, 20, 20)
            $g.FillEllipse($b, 54, 48, 20, 20)
            $b.Dispose()
            $g.FillRectangle((New-Object System.Drawing.SolidBrush -ArgumentList $cyan), 38, 24, 5, 40)
            $g.FillRectangle((New-Object System.Drawing.SolidBrush -ArgumentList $cyan), 70, 14, 5, 40)
            $g.FillRectangle((New-Object System.Drawing.SolidBrush -ArgumentList $cyan), 38, 24, 37, 5)
        }
        'phone' {
            $state = $g.Save()
            $g.TranslateTransform(48, 48)
            $g.RotateTransform(45)
            Add-Round $g -30 -9 60 18 9 $cyan
            Add-Round $g -38 -14 20 28 8 $cyan
            Add-Round $g 18 -14 20 28 8 $cyan
            $g.Restore($state)
        }
        'setting' {
            $g.DrawEllipse($pen, 26, 26, 44, 44)
            $g.DrawEllipse($pen, 38, 38, 20, 20)
            for ($i=0; $i -lt 8; $i++) {
                $a = $i * 45 * [Math]::PI / 180
                $x1 = 48 + [Math]::Cos($a) * 25
                $y1 = 48 + [Math]::Sin($a) * 25
                $x2 = 48 + [Math]::Cos($a) * 36
                $y2 = 48 + [Math]::Sin($a) * 36
                $g.DrawLine($pen, [float]$x1, [float]$y1, [float]$x2, [float]$y2)
            }
        }
        'dash' {
            $g.DrawArc($pen, 18, 20, 60, 60, 135, 270)
            $a = 300 * [Math]::PI / 180
            $g.DrawLine($pen, 48, 50, [float](48+[Math]::Cos($a)*24), [float](50+[Math]::Sin($a)*24))
            $b = New-Object System.Drawing.SolidBrush -ArgumentList $cyan
            $g.FillEllipse($b, 43, 45, 10, 10)
            $b.Dispose()
        }
    }
    $pen.Dispose()
    $g.Dispose()
    $bmp.Save((Join-Path $OutDir $Name),[System.Drawing.Imaging.ImageFormat]::Bmp)
    $bmp.Dispose()
    Write-Host ("  " + $Name)
}

New-Icon 'icon_nav.bmp' 'nav'
New-Icon 'icon_dash.bmp' 'dash'
New-Icon 'icon_music.bmp' 'music'
New-Icon 'icon_phone.bmp' 'phone'
New-Icon 'icon_setting.bmp' 'setting'

# ---------------- 昼夜小图标 24x24 ----------------
$card = [System.Drawing.Color]::FromArgb(255,22,40,62)
$bmp = New-Object System.Drawing.Bitmap -ArgumentList 24,24,([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.Clear($card)
$sun = [System.Drawing.Color]::FromArgb(255,255,196,0)
Add-Circle $g 12 12 5 $sun
$pen = New-Object System.Drawing.Pen -ArgumentList $sun,2
for ($i=0; $i -lt 8; $i++) {
    $a = $i * 45 * [Math]::PI / 180
    $g.DrawLine($pen, [float](12+[Math]::Cos($a)*7), [float](12+[Math]::Sin($a)*7), [float](12+[Math]::Cos($a)*10), [float](12+[Math]::Sin($a)*10))
}
$pen.Dispose(); $g.Dispose()
$bmp.Save((Join-Path $OutDir 'icon_sun.bmp'),[System.Drawing.Imaging.ImageFormat]::Bmp)
$bmp.Dispose()
Write-Host '  icon_sun.bmp'

$bmp = New-Object System.Drawing.Bitmap -ArgumentList 24,24,([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.Clear($card)
Add-Circle $g 11 12 8 ([System.Drawing.Color]::FromArgb(255,220,230,255))
Add-Circle $g 15 9 7 $card
$g.Dispose()
$bmp.Save((Join-Path $OutDir 'icon_moon.bmp'),[System.Drawing.Imaging.ImageFormat]::Bmp)
$bmp.Dispose()
Write-Host '  icon_moon.bmp'

# ---------------- 仪表盘背景 ----------------
$c = New-Canvas 800 480 '#050A12' '#0C1826' 0
$g = $c.Graphics
Add-Circle $g 300 240 190 ([System.Drawing.Color]::FromArgb(18,0,120,200))
Add-Circle $g 660 290 120 ([System.Drawing.Color]::FromArgb(16,255,80,80))
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(12,255,255,255)),1
for ($i=0; $i -lt 24; $i++) { $g.DrawLine($pen, 0, $i*24, 800, $i*24-80) }
$pen.Dispose()
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(40,0,194,255)),2
$g.DrawLine($pen, 20, 430, 780, 430)
$pen.Dispose()
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'dash_bg.bmp')

# ---------------- 专辑封面 300x300 ----------------
$c = New-Canvas 300 300 '#2A0845' '#0B1B2E' 35
$g = $c.Graphics
for ($i=0; $i -lt 5; $i++) {
    $r = 40 + $i*28
    $pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(60,255,180,255)),2
    $g.DrawEllipse($pen, 150-$r, 140-$r, 2*$r, 2*$r)
    $pen.Dispose()
}
Add-Text $g 'MUSIC' 14 20 246 260 30 '#B8A0D0'
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'album.bmp')

# ---------------- 倒车影像占位 800x480 ----------------
$c = New-Canvas 800 480 '#12161A' '#050708' 90
$g = $c.Graphics
$pen = New-Object System.Drawing.Pen -ArgumentList ([System.Drawing.Color]::FromArgb(60,200,220,240)),2
$g.DrawLine($pen, 400, 180, 40, 480)
$g.DrawLine($pen, 400, 180, 760, 480)
$g.DrawLine($pen, 400, 180, 400, 480)
for ($i=0; $i -lt 5; $i++) { $y = 250 + $i*46; $g.DrawLine($pen, 400-($i*70), $y, 400+($i*70), $y) }
$pen.Dispose()
$b = New-Object System.Drawing.SolidBrush -ArgumentList ([System.Drawing.Color]::FromArgb(255,28,34,40))
$g.FillRectangle($b, 0, 0, 90, 480)
$g.FillRectangle($b, 710, 0, 90, 480)
$b.Dispose()
Add-Text $g 'REVERSE CAMERA' 12 250 430 300 22 '#506070'
$g.Dispose()
Save-Canvas $c (Join-Path $OutDir 'reverse.bmp')

Write-Host 'Done.'
