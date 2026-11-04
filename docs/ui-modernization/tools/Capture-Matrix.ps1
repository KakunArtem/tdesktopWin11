[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)][ValidatePattern('^[a-zA-Z0-9][a-zA-Z0-9._-]{0,79}$')][string]$Plan,
  [Parameter(Mandatory = $true)][ValidatePattern('^[a-zA-Z0-9][a-zA-Z0-9._-]{0,79}$')][string]$State,
  [ValidateSet('folds', 'warm', 'sky', 'night')][string]$Wallpaper,
  [ValidatePattern('^[a-zA-Z0-9][a-zA-Z0-9._-]{0,79}$')][string]$Tag,
  [long]$WindowHandle = 0,
  [switch]$AllowInactive
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Ui-Checks.Common.ps1')
$target = Get-CheckWindow -WindowHandle $WindowHandle
$folder = Join-Path $env:LOCALAPPDATA "tdesktop-ui-checks/$Plan"
New-Item -ItemType Directory -Path $folder -Force | Out-Null
$name = $State
if ($Wallpaper) { $name += "-$Wallpaper" }
if ($Tag) { $name += "-$Tag" }
$path = Join-Path $folder "$name.png"
Assert-CheckOutput $path
$backdrop = $null; $bitmap = $null
try {
  if ($Wallpaper) { $backdrop = Show-Backdrop -Wallpaper $Wallpaper -WindowHandle $target.ToInt64() }
  $bitmap = Get-CheckCapture -Handle $target -Client -AllowInactive:$AllowInactive
  $bitmap.Save($path)
  $metadata = [ordered]@{
    Date = [DateTime]::Now.ToString('s'); Plan = $Plan; State = $State; Wallpaper = $Wallpaper; Tag = $Tag
    Width = $bitmap.Width; Height = $bitmap.Height; Dpi = [UiChecks.Native]::GetDpiForWindow($target)
    Active = [bool]$bitmap.Tag; AllowInactive = [bool]$AllowInactive
    Commit = (& git -C (Join-Path $PSScriptRoot '../../..') rev-parse HEAD)
    Windows = [Environment]::OSVersion.Version.ToString()
    GPU = @(Get-CimInstance Win32_VideoController | ForEach-Object { $_.Name })
  }
  [IO.File]::WriteAllText((Join-Path $folder "$name.json"), (($metadata | ConvertTo-Json) + "`r`n"), (New-Object Text.UTF8Encoding($false)))
  $path
} finally {
  if ($bitmap) { $bitmap.Dispose() }
  Hide-Backdrop $backdrop
}
