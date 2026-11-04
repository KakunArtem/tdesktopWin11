[CmdletBinding()]
param(
  [ValidateSet('folds', 'warm', 'sky', 'night')][string]$Wallpaper,
  [ValidatePattern('^#[0-9a-fA-F]{6}$')][string]$Color,
  [ValidateRange(0, 86400)][int]$Seconds = 0,
  [long]$WindowHandle = 0,
  [switch]$Worker,
  [string]$ReadyFile
)
. (Join-Path $PSScriptRoot 'Ui-Checks.Common.ps1')

if ($MyInvocation.InvocationName -ne '.') {
  if ([bool]$Wallpaper -eq [bool]$Color) { throw 'Choose exactly one of -Wallpaper or -Color.' }
  if ($Worker) {
    $backdrop = $null
    try {
      $options = @{ WindowHandle = $WindowHandle }
      if ($Wallpaper) { $options.Wallpaper = $Wallpaper } else { $options.Color = $Color }
      $backdrop = Show-Backdrop @options
      [IO.File]::WriteAllText($ReadyFile, $backdrop.Handle.ToInt64().ToString())
      $started = [DateTime]::UtcNow
      while ($Seconds -eq 0 -or ([DateTime]::UtcNow - $started).TotalSeconds -lt $Seconds) { Wait-CheckPaint -Milliseconds 50 }
    } finally { Hide-Backdrop $backdrop }
  } else {
    $target = Get-CheckWindow -WindowHandle $WindowHandle
    $folder = Join-Path $env:LOCALAPPDATA 'tdesktop-ui-checks/backdrops'
    New-Item -ItemType Directory -Path $folder -Force | Out-Null
    $ready = Join-Path $folder ([Guid]::NewGuid().ToString('N') + '.txt')
    $choice = "-Color $Color"
    if ($Wallpaper) { $choice = "-Wallpaper $Wallpaper" }
    $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`" -Worker $choice -Seconds $Seconds -WindowHandle $($target.ToInt64()) -ReadyFile `"$ready`""
    $child = Start-Process -FilePath "$PSHOME/powershell.exe" -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardError "$ready.error"
    $deadline = [DateTime]::UtcNow.AddSeconds(15)
    while (-not (Test-Path -LiteralPath $ready) -and -not $child.HasExited -and [DateTime]::UtcNow -lt $deadline) { Start-Sleep -Milliseconds 50 }
    if (-not (Test-Path -LiteralPath $ready)) {
      if (-not $child.HasExited) { $child.Kill() }
      throw "Backdrop failed to start. See $ready.error"
    }
    $child.Id
  }
}
