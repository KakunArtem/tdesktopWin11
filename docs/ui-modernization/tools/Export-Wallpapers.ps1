[CmdletBinding()]
param(
  [ValidateRange(64, 16384)][int]$Width = 2560,
  [ValidateRange(64, 16384)][int]$Height = 1440,
  [string]$OutDir = (Join-Path $env:LOCALAPPDATA 'tdesktop-ui-checks/wallpapers'),
  [string]$EdgePath
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Ui-Checks.Common.ps1')
Assert-CheckOutput $OutDir
if (-not $EdgePath) {
  $EdgePath = @(
    "${env:ProgramFiles(x86)}/Microsoft/Edge/Application/msedge.exe",
    "$env:ProgramFiles/Microsoft/Edge/Application/msedge.exe"
  ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $EdgePath -or -not (Test-Path -LiteralPath $EdgePath)) { throw 'Microsoft Edge not found; specify -EdgePath.' }
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
$OutDir = (Resolve-Path -LiteralPath $OutDir).Path
$profile = Join-Path $OutDir ('.edge-' + [Guid]::NewGuid().ToString('N'))
$source = [Uri](Join-Path $PSScriptRoot 'wallpapers.html')
try {
  foreach ($name in @('folds', 'warm', 'sky', 'night')) {
    $path = Join-Path $OutDir "$name.png"
    $arguments = @('--headless', '--disable-gpu', '--no-first-run', '--hide-scrollbars',
      '--force-device-scale-factor=1', '--run-all-compositor-stages-before-draw',
      '--virtual-time-budget=1000', "--user-data-dir=`"$profile`"", "--screenshot=`"$path`"",
      "--window-size=$Width,$Height", "`"$($source.AbsoluteUri)?wallpaper=$name`"")
    $worker = Start-Process -FilePath $EdgePath -ArgumentList $arguments -WindowStyle Hidden -PassThru
    if (-not $worker.WaitForExit(30000)) { $worker.Kill(); throw "Edge timed out rendering $name." }
    if ($worker.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $path)) { throw "Edge failed rendering $name." }
    $path
  }
} finally {
  $resolvedProfile = [IO.Path]::GetFullPath($profile)
  if ($resolvedProfile.StartsWith($OutDir + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -and
      (Test-Path -LiteralPath $resolvedProfile)) {
    Remove-Item -LiteralPath $resolvedProfile -Recurse -Force -ErrorAction SilentlyContinue
  }
}
