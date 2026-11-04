[CmdletBinding()]
param(
  [ValidateSet('Transparent', 'Opaque')][string]$Expect = 'Transparent',
  [string]$OutDir = (Join-Path $env:LOCALAPPDATA ('tdesktop-ui-checks/backdrop/' + [DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))),
  [ValidateRange(1, 100)][int]$Step = 3,
  [long]$WindowHandle = 0,
  [string[]]$Region,
  [ValidateSet('Main', 'Popup')][string]$Window = 'Main',
  [switch]$SelfTest,
  [switch]$AllowInactive,
  [ValidateRange(0, 15)][int]$Monitor = 0
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Show-Backdrop.ps1') -WindowHandle $WindowHandle
Assert-CheckOutput $OutDir

function Compare-Backdrops {
  param([IntPtr]$Handle, [int[]]$SampleRegion, [string]$Directory, [switch]$AllowInactiveCapture)
  New-Item -ItemType Directory -Path $Directory -Force | Out-Null
  $backdrop = $null; $red = $null; $blue = $null
  $initial = [UiChecks.Native]::ClientBounds($Handle)
  $initialActive = [UiChecks.Native]::Active($Handle)
  try {
    $backdrop = Show-Backdrop -Color '#ff0000' -WindowHandle $Handle.ToInt64()
    $red = Get-CheckCapture -Handle $Handle -Region $SampleRegion -Inset 2 -AllowInactive:$AllowInactiveCapture
    if ([bool]$red.Tag -ne $initialActive) { throw 'Target activation changed before first capture; retry.' }
    $red.Save((Join-Path $Directory 'behind-red.png'))
    $backdrop.BackColor = [Drawing.Color]::Blue
    $backdrop.Refresh(); Wait-CheckPaint
    if ([UiChecks.Native]::ClientBounds($Handle) -ne $initial) { throw 'Target moved or resized between backdrops; retry.' }
    $blue = Get-CheckCapture -Handle $Handle -Region $SampleRegion -Inset 2 -AllowInactive:$AllowInactiveCapture
    if ([bool]$blue.Tag -ne $initialActive) { throw 'Target activation changed between backdrops; retry.' }
    $blue.Save((Join-Path $Directory 'behind-blue.png'))
    $difference = [UiChecks.Pixels]::Compare($red, $blue, 30, $Step, $true)
    $difference | Add-Member -NotePropertyName Active -NotePropertyValue $initialActive
    if ($difference.Changed -gt 0) {
      $bounds = $difference.Bounds
      if ($SampleRegion) { $bounds.Offset($SampleRegion[0], $SampleRegion[1]) }
      else {
        $frame = [UiChecks.Native]::FrameBounds($Handle)
        $bounds.Offset(($frame.X + 2 - $initial.X), ($frame.Y + 2 - $initial.Y))
      }
      $difference.Bounds = $bounds
    }
    return $difference
  } finally {
    if ($red) { $red.Dispose() }; if ($blue) { $blue.Dispose() }
    Hide-Backdrop $backdrop
  }
}

if ($SelfTest) {
  $fixture = New-Object UiChecks.BackdropForm
  $fixture.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 320 -Height 180
  $fixture.TopMost = $true; $fixture.BackColor = [Drawing.Color]::White
  $popup = $null
  try {
    $fixture.Show(); Wait-CheckPaint
    $opaque = Compare-Backdrops $fixture.Handle @(12, 9, 80, 40) (Join-Path $OutDir 'opaque') -AllowInactiveCapture
    if ($opaque.Changed -ne 0) { throw 'Opaque fixture changed' }
    $fixture.Opacity = 0.7; Wait-CheckPaint
    $transparent = Compare-Backdrops $fixture.Handle @(12, 9, 80, 40) (Join-Path $OutDir 'transparent') -AllowInactiveCapture
    if ($transparent.SampledChanged -ne $transparent.SampledTotal -or $transparent.Bounds -ne (New-Object Drawing.Rectangle(12, 9, 80, 40))) { throw 'Transparent region bounds incorrect' }
    $popup = New-Object UiChecks.BackdropForm
    $popup.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 180 -Height 100 -X 420
    $popup.TopMost = $true; $popup.BackColor = [Drawing.Color]::White
    $popup.Show(); Wait-CheckPaint
    if ([UiChecks.Native]::SelectWindow([uint32]$PID, $false) -ne $fixture.Handle -or
        [UiChecks.Native]::SelectWindow([uint32]$PID, $true) -ne $popup.Handle) { throw 'Main/popup selection incorrect' }
    $popupResult = Compare-Backdrops $popup.Handle @(0, 0, 180, 100) (Join-Path $OutDir 'popup') -AllowInactiveCapture
    if ($popupResult.Changed -ne 0) { throw 'Opaque popup changed' }
    $invalid = $false
    try { Get-CheckCapture -Handle $popup.Handle -Region @(170, 0, 20, 10) | Out-Null }
    catch { $invalid = $_.Exception.Message -match 'INVALID region' }
    if (-not $invalid) { throw 'Invalid client region was accepted' }
    "SELFTEST PASS: opaque, transparent bounds 12,9,80,40, popup selection, invalid region; DPI $([UiChecks.Native]::GetDpiForWindow($fixture.Handle)). Evidence: $OutDir"
  } finally { if ($popup) { $popup.Dispose() }; $fixture.Dispose() }
  exit 0
}

$sampleRegion = @()
if ($Region) {
  foreach ($part in (($Region -join ',') -split ',')) {
    $number = 0
    if (-not [int]::TryParse($part, [ref]$number)) { throw 'INVALID region: expected four integers.' }
    $sampleRegion += $number
  }
  if ($sampleRegion.Count -ne 4) { throw 'INVALID region: expected x,y,width,height.' }
}
$target = Get-CheckWindow -WindowHandle $WindowHandle -Window $Window
$difference = Compare-Backdrops $target $sampleRegion $OutDir -AllowInactiveCapture:$AllowInactive
$fraction = $difference.SampledChanged / [Math]::Max(1, $difference.SampledTotal)
$bounds = $difference.Bounds
"Changed pixels: $($difference.SampledChanged) of $($difference.SampledTotal) sampled ($($fraction.ToString('P2'))). Captures: $OutDir"
"Changed bounds: $($bounds.X),$($bounds.Y),$($bounds.Width),$($bounds.Height)"
"Activation: $($difference.Active); AllowInactive: $AllowInactive"
$transparent = $fraction -gt 0.002
if (($Expect -eq 'Transparent') -eq $transparent) { "PASS: expected $Expect"; exit 0 }
"FAIL: expected $Expect"; exit 1
