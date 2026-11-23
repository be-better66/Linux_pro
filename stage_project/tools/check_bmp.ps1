[CmdletBinding()]
param(
    [string]$Dir = (Join-Path $PSScriptRoot '..\res\car'),
    [int]$Width = 800,
    [int]$Height = 480,
    [switch]$Any
)

# 检查 BMP 是否是 GEC6818 工程能直接读取的规格：
# 默认 800x480、24 位、无压缩。
# -Any 只检查 24 位无压缩，不限制尺寸（适合混合目录）。

if (-not (Test-Path $Dir)) {
    Write-Host "目录不存在: $Dir" -ForegroundColor Red
    exit 1
}

$files = Get-ChildItem -Path $Dir -File -Filter '*.bmp' | Sort-Object Name
if ($files.Count -eq 0) {
    Write-Host "目录里没有 .bmp 文件: $Dir" -ForegroundColor Yellow
    exit 1
}

$bad = 0
foreach ($f in $files) {
    $b = [System.IO.File]::ReadAllBytes($f.FullName)
    if ($b.Length -lt 54 -or $b[0] -ne 0x42 -or $b[1] -ne 0x4D) {
        Write-Host ("[FAIL] {0}  不是 BMP 文件" -f $f.Name) -ForegroundColor Red
        $bad++
        continue
    }

    $w = [BitConverter]::ToInt32($b, 18)
    $h = [BitConverter]::ToInt32($b, 22)
    $bpp = [BitConverter]::ToInt16($b, 28)
    $comp = [BitConverter]::ToInt32($b, 30)

    $size_ok = $Any -or (($w -eq $Width) -and ([Math]::Abs($h) -eq $Height))
    $ok = $size_ok -and ($bpp -eq 24) -and ($comp -eq 0)

    $mark = if ($ok) { '[ OK ]' } else { '[FAIL]' }
    $color = if ($ok) { 'Green' } else { 'Red' }
    $expect = if ($Any) { '任意尺寸' } else { "$Width x $Height" }
    Write-Host ("{0} {1,-18} {2}x{3} {4}bpp comp={5}  期望: {6}, 24bpp, comp=0" -f $mark, $f.Name, $w, $h, $bpp, $comp, $expect) -ForegroundColor $color
    if (-not $ok) { $bad++ }
}

Write-Host ""
if ($bad -eq 0) {
    Write-Host "全部通过。" -ForegroundColor Green
    exit 0
} else {
    Write-Host ("有 {0} 个文件不符合要求。" -f $bad) -ForegroundColor Red
    exit 1
}
