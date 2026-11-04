$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
if (-not ('UiChecks.Native' -as [type])) {
  Add-Type -Path (Join-Path $PSScriptRoot 'UiChecks.Native.cs') -ReferencedAssemblies System.Drawing, System.Windows.Forms
}
[UiChecks.Native]::PhysicalPixels()

function Assert-CheckOutput([string]$Path) {
  $checkout = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..')).TrimEnd('\', '/')
  $resolved = [IO.Path]::GetFullPath($Path)
  if ($resolved -eq $checkout -or $resolved.StartsWith($checkout + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Capture output must stay outside the repository.'
  }
}

function Get-CheckFixtureBounds {
  param([int]$Monitor = 0, [int]$Width, [int]$Height, [int]$X = 80, [int]$Y = 80)
  $screens = [Windows.Forms.Screen]::AllScreens
  if ($Monitor -lt 0 -or $Monitor -ge $screens.Count) { throw "Monitor $Monitor unavailable; found $($screens.Count) monitor(s)." }
  $area = $screens[$Monitor].WorkingArea
  $rect = New-Object Drawing.Rectangle(($area.Left + $X), ($area.Top + $Y), $Width, $Height)
  if (-not $area.Contains($rect)) { throw 'Fixture does not fit on the selected monitor.' }
  return $rect
}

function Get-CheckWindow {
  param([long]$WindowHandle = 0, [ValidateSet('Main', 'Popup')][string]$Window = 'Main')
  if ($WindowHandle -ne 0) {
    $handle = [IntPtr]$WindowHandle
  } else {
    $checkout = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
    $executable = Join-Path $checkout 'out/Debug/Telegram.exe'
    $processes = @(Get-Process Telegram -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $executable })
    if ($processes.Count -ne 1) { throw 'Start exactly one checkout out\Debug\Telegram.exe, or specify -WindowHandle.' }
    $handle = [UiChecks.Native]::SelectWindow([uint32]$processes[0].Id, ($Window -eq 'Popup'))
  }
  if (-not $handle -or -not [UiChecks.Native]::Visible($handle)) { throw "No visible $Window target window found." }
  return [IntPtr]$handle
}

function Wait-CheckPaint {
  param([int]$Milliseconds = 600)
  $until = [DateTime]::UtcNow.AddMilliseconds($Milliseconds)
  do {
    [Windows.Forms.Application]::DoEvents()
    Start-Sleep -Milliseconds 20
  } while ([DateTime]::UtcNow -lt $until)
}

function Get-CheckCapture {
  param([IntPtr]$Handle, [switch]$Client, [int[]]$Region, [ValidateRange(0, 20)][int]$Inset = 0, [switch]$AllowInactive)
  $clientBounds = [UiChecks.Native]::ClientBounds($Handle)
  $bounds = [UiChecks.Native]::FrameBounds($Handle)
  if ($Client) { $bounds = $clientBounds }
  if (-not $Client -and -not $Region -and $Inset) { $bounds.Inflate(-$Inset, -$Inset) }
  if ($Region) {
    if ($Region.Count -ne 4 -or $Region[0] -lt 0 -or $Region[1] -lt 0 -or $Region[2] -le 0 -or $Region[3] -le 0 -or
        [long]$Region[0] + $Region[2] -gt $clientBounds.Width -or [long]$Region[1] + $Region[3] -gt $clientBounds.Height) {
      throw 'INVALID region: expected x,y,width,height inside physical client bounds.'
    }
    $bounds = New-Object Drawing.Rectangle(($clientBounds.X + $Region[0]), ($clientBounds.Y + $Region[1]), $Region[2], $Region[3])
  }
  if ($bounds.Width -le 0 -or $bounds.Height -le 0) { throw 'Target has empty bounds.' }
  $active = [UiChecks.Native]::Active($Handle)
  if (-not $active -and -not $AllowInactive) { throw 'Target is inactive. Activate its owner window before capture, or explicitly use -AllowInactive for an inactive-state check.' }
  [UiChecks.Native]::RequireExposed($Handle, $bounds)
  $bitmap = [UiChecks.Native]::Capture($bounds)
  try {
    if ([UiChecks.Native]::ClientBounds($Handle) -ne $clientBounds) { throw 'Target moved or resized during capture; retry.' }
    if ([UiChecks.Native]::Active($Handle) -ne $active) { throw 'Target activation changed during capture; retry.' }
    [UiChecks.Native]::RequireExposed($Handle, $bounds)
    $bitmap.Tag = $active
    return $bitmap
  } catch { $bitmap.Dispose(); throw }
}

function Measure-CheckSample {
  param([Drawing.Bitmap]$Bitmap, $Sample)
  $result = [pscustomobject]@{ Name = [string]$Sample.name; Background = ''; Text = ''; Ratio = 0.0; Threshold = 4.5; Result = 'INVALID'; Reason = '' }
  try {
    if ($Sample.kind -notin @('text', 'large', 'icon')) { throw 'kind must be text, large or icon' }
    if ($Sample.kind -ne 'text') { $result.Threshold = 3.0 }
    $rect = @($Sample.rect)
    if ($rect.Count -ne 4) { throw 'rect must have four integers' }
    foreach ($number in $rect) {
      if ($number -isnot [int] -and $number -isnot [long]) { throw 'rect must contain integers' }
    }
    if ($rect[0] -lt 0 -or $rect[1] -lt 0 -or $rect[2] -le 0 -or $rect[3] -le 0 -or
        [long]$rect[0] + $rect[2] -gt $Bitmap.Width -or [long]$rect[1] + $rect[3] -gt $Bitmap.Height) { throw 'rect extends outside client capture' }
    if ($Sample.text -and $Sample.text -notmatch '^#[0-9a-fA-F]{6}$') { throw 'text must be #rrggbb' }
    $rectangle = New-Object Drawing.Rectangle($rect[0], $rect[1], $rect[2], $rect[3])
    $measured = [UiChecks.Pixels]::Measure($Bitmap, $rectangle, [string]$Sample.text)
    $result.Background = $measured.Background; $result.Text = $measured.Text; $result.Ratio = $measured.Ratio
    $result.Result = 'FAIL'
    if ($result.Ratio -ge $result.Threshold) { $result.Result = 'PASS' }
  } catch { $result.Reason = $_.Exception.Message }
  return $result
}

function Show-Backdrop {
  param(
    [ValidateSet('folds', 'warm', 'sky', 'night')][string]$Wallpaper,
    [ValidatePattern('^#[0-9a-fA-F]{6}$')][string]$Color,
    [long]$WindowHandle = 0
  )
  if ([bool]$Wallpaper -eq [bool]$Color) { throw 'Choose exactly one of -Wallpaper or -Color.' }
  $target = Get-CheckWindow -WindowHandle $WindowHandle
  $form = New-Object UiChecks.BackdropForm
  try {
    $form.Bounds = [Windows.Forms.Screen]::FromHandle($target).Bounds
    if ($Wallpaper) {
      $path = Join-Path $env:LOCALAPPDATA "tdesktop-ui-checks/wallpapers/$Wallpaper.png"
      if (-not (Test-Path -LiteralPath $path)) { throw 'Run Export-Wallpapers.ps1 first.' }
      $form.BackgroundImage = [Drawing.Image]::FromFile($path)
      $form.BackgroundImageLayout = 'Stretch'
    } else { $form.BackColor = [Drawing.ColorTranslator]::FromHtml($Color) }
    $form.Show()
    if (-not [UiChecks.Native]::SetWindowPos($form.Handle, $target, $form.Left, $form.Top, $form.Width, $form.Height, 0x0010)) {
      throw 'Cannot place backdrop behind target.'
    }
    Wait-CheckPaint
    return $form
  } catch { Hide-Backdrop $form; throw }
}

function Hide-Backdrop($Backdrop) {
  if ($Backdrop) {
    $image = $Backdrop.BackgroundImage
    $Backdrop.Dispose()
    if ($image) { $image.Dispose() }
  }
}
