[CmdletBinding()]
param(
  [ValidateSet('Wallpapers', 'Backdrop', 'Contrast', 'Regions', 'Captures', 'All')][string]$Check = 'All',
  [ValidateRange(0, 15)][int]$Monitor = 0
)
$ErrorActionPreference = 'Stop'
$tools = Join-Path $PSScriptRoot '../tools'
$output = Join-Path $env:LOCALAPPDATA ('tdesktop-ui-checks/self-tests/' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $output -Force | Out-Null
. (Join-Path $tools 'Ui-Checks.Common.ps1')
Get-CheckFixtureBounds -Monitor $Monitor -Width 600 -Height 200 | Out-Null
function Assert($Condition, [string]$Message) {
  if (-not $Condition) { throw $Message }
}
function Invoke-Tool([string]$Name, [string]$Arguments, [string]$Log) {
  $parameters = "-NoProfile -ExecutionPolicy Bypass -File `"$tools/$Name.ps1`" $Arguments"
  $child = Start-Process powershell -ArgumentList $parameters -PassThru -WindowStyle Hidden -RedirectStandardOutput "$Log.stdout" -RedirectStandardError "$Log.stderr"
  $child.Handle | Out-Null
  $deadline = [DateTime]::UtcNow.AddSeconds(30)
  while (-not $child.WaitForExit(20) -and [DateTime]::UtcNow -lt $deadline) { Wait-CheckPaint -Milliseconds 30 }
  if (-not $child.HasExited) { $child.Kill(); throw "$Name timed out" }
  return $child.ExitCode
}
if ($Check -in @('Wallpapers', 'All')) {
  & (Join-Path $tools 'Export-Wallpapers.ps1') -Width 320 -Height 180 -OutDir $output
  Add-Type -AssemblyName System.Drawing
  $hashes = @()
  foreach ($name in @('folds', 'warm', 'sky', 'night')) {
    $path = Join-Path $output "$name.png"
    $bitmap = [Drawing.Bitmap]::FromFile($path)
    try { Assert ($bitmap.Width -eq 320 -and $bitmap.Height -eq 180) "$name has wrong dimensions" }
    finally { $bitmap.Dispose() }
    $hashes += (Get-FileHash $path).Hash
  }
  Assert (@($hashes | Select-Object -Unique).Count -eq 4) 'Wallpapers must be distinct'
  'PASS: four distinct wallpapers at requested physical dimensions'
}
if ($Check -in @('Backdrop', 'All')) {
  . (Join-Path $tools 'Show-Backdrop.ps1')
  $fixture = New-Object UiChecks.BackdropForm
  $fixture.StartPosition = 'Manual'
  $fixture.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 320 -Height 200
  $fixture.BackColor = [Drawing.Color]::White
  $fixture.Show()
  [Windows.Forms.Application]::DoEvents()
  $foreground = [UiChecks.Native]::GetForegroundWindow()
  $backdrop = $null
  try {
    $backdrop = Show-Backdrop -Color '#123456' -WindowHandle $fixture.Handle.ToInt64()
    [Windows.Forms.Application]::DoEvents()
    Assert ([UiChecks.Native]::GetForegroundWindow() -eq $foreground) 'Backdrop stole focus'
    Assert ([UiChecks.Native]::GetWindow($fixture.Handle, 2) -eq $backdrop.Handle) 'Backdrop is not directly below target'
    Assert ($backdrop.Bounds -eq [Windows.Forms.Screen]::FromHandle($fixture.Handle).Bounds) 'Backdrop does not cover target monitor'
    Assert ($backdrop.BackColor.ToArgb() -eq [Drawing.ColorTranslator]::FromHtml('#123456').ToArgb()) 'Wrong backdrop color'
    'PASS: monitor bounds, color, z-order and unchanged focus'
  } finally {
    if ($backdrop) { Hide-Backdrop $backdrop }
    $fixture.Dispose()
  }
}
if ($Check -in @('Contrast', 'All')) {
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Measure-Contrast.ps1') -SelfTest -Monitor $Monitor
  Assert ($LASTEXITCODE -eq 0) 'Native contrast self-test failed'
  . (Join-Path $tools 'Show-Backdrop.ps1')
  $fixture = New-Object UiChecks.BackdropForm
  $fixture.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 320 -Height 120
  $fixture.TopMost = $true
  $fixture.BackColor = [Drawing.ColorTranslator]::FromHtml('#e9eef5')
  $label = New-Object Windows.Forms.Label
  $label.Text = 'Readable sample'; $label.Size = New-Object Drawing.Size(300, 60)
  $label.ForeColor = [Drawing.ColorTranslator]::FromHtml('#3a4b5f')
  $fixture.Controls.Add($label)
  $fixture.Show(); Wait-CheckPaint
  try {
    foreach ($outside in @($false, $true)) {
      $width = 300
      if ($outside) { $width = 400 }
      $samplePath = Join-Path $output 'samples.json'
      $definition = '[{"name":"fixture","rect":[0,0,' + $width + ',60],"kind":"text","text":"#3a4b5f"}]'
      [IO.File]::WriteAllText($samplePath, $definition)
      $report = Join-Path $output "contrast-$outside.md"
      $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$tools/Measure-Contrast.ps1`" -AllowInactive -WindowHandle $($fixture.Handle.ToInt64()) -Samples `"$samplePath`" -Out `"$report`""
      $child = Start-Process powershell -ArgumentList $arguments -PassThru -WindowStyle Hidden -RedirectStandardOutput "$report.stdout" -RedirectStandardError "$report.stderr"
      $child.Handle | Out-Null
      $deadline = [DateTime]::UtcNow.AddSeconds(30)
      while (-not $child.WaitForExit(20) -and [DateTime]::UtcNow -lt $deadline) { Wait-CheckPaint -Milliseconds 30 }
      if (-not $child.HasExited) { $child.Kill(); throw 'Contrast CLI timed out' }
      $expected = [int]$outside
      Assert ($child.ExitCode -eq $expected) "Contrast CLI exit $($child.ExitCode), expected $expected; see $report.stderr"
      $result = 'PASS'
      if ($outside) { $result = 'INVALID' }
      Assert ((Get-Content -LiteralPath $report -Raw) -match $result) "Report missing $result"
    }
    'PASS: contrast CLI reports both backdrops; invalid rectangle exits 1'
  } finally { $fixture.Dispose() }
}
if ($Check -in @('Regions', 'All')) {
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Test-Backdrop.ps1') -SelfTest -Monitor $Monitor
  Assert ($LASTEXITCODE -eq 0) 'Region and popup self-test failed'
}
if ($Check -in @('Captures', 'All')) {
  . (Join-Path $tools 'Ui-Checks.Common.ps1')
  $rejected = $false
  try { Assert-CheckOutput (Join-Path $PSScriptRoot 'forbidden-output.png') }
  catch { $rejected = $_.Exception.Message -match 'outside the repository' }
  Assert $rejected 'Output inside the repository must be rejected before writing'
  $bitmap = New-Object Drawing.Bitmap(32, 24)
  $graphics = [Drawing.Graphics]::FromImage($bitmap)
  $graphics.Clear([Drawing.Color]::White); $graphics.Dispose()
  $before = Join-Path $output 'before.png'; $after = Join-Path $output 'after.png'
  $bitmap.Save($before)
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Compare-Captures.ps1') -Before $before -After $before
  Assert ($LASTEXITCODE -eq 0) 'Identical images must compare equal'
  $bitmap.SetPixel(3, 4, [Drawing.Color]::Black); $bitmap.Save($after); $bitmap.Dispose()
  $comparison = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Compare-Captures.ps1') -Before $before -After $after
  Assert ($LASTEXITCODE -eq 1 -and ($comparison -match 'Changed bounds: 3,4,1,1')) 'Changed pixel bounds incorrect'
  $scaled = New-Object Drawing.Bitmap(40, 30)
  $scaledPath = Join-Path $output 'scaled.png'; $scaled.Save($scaledPath); $scaled.Dispose()
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Compare-Captures.ps1') -Before $before -After $scaledPath
  Assert ($LASTEXITCODE -eq 2) 'Size mismatch must exit 2'
  $alpha = New-Object Drawing.Bitmap(32, 24)
  $graphics = [Drawing.Graphics]::FromImage($alpha)
  $graphics.Clear([Drawing.Color]::White); $graphics.Dispose()
  $alpha.SetPixel(3, 4, [Drawing.Color]::FromArgb(0, 255, 255, 255))
  $alphaPath = Join-Path $output 'alpha.png'; $alpha.Save($alphaPath); $alpha.Dispose()
  & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Compare-Captures.ps1') -Before $before -After $alphaPath
  Assert ($LASTEXITCODE -eq 1) 'Alpha difference was missed'
  'PASS: identical / changed bounds / mismatched-size comparison contracts'
  $fixture = New-Object UiChecks.BackdropForm
  $fixture.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 240 -Height 120
  $fixture.TopMost = $true; $fixture.BackColor = [Drawing.Color]::White
  $fixture.Show(); Wait-CheckPaint
  try {
    $inactiveRejected = $false
    try { $unexpected = Get-CheckCapture -Handle $fixture.Handle -Client; $unexpected.Dispose() }
    catch { $inactiveRejected = $_.Exception.Message -match 'inactive' }
    Assert $inactiveRejected 'An inactive target was accepted as active evidence'
    $state = [Guid]::NewGuid().ToString('N')
    $log = Join-Path $output 'inactive-rejected'
    $arguments = "-Plan self-tests -State $state -Tag rejected -WindowHandle $($fixture.Handle.ToInt64())"
    Assert ((Invoke-Tool 'Capture-Matrix' $arguments $log) -ne 0) 'Default capture CLI accepted an inactive target'
    Assert ((Get-Content "$log.stderr" -Raw) -match 'inactive') 'Inactive CLI rejection has no explanation'
    $captures = @()
    foreach ($tag in @('first', 'second')) {
      $log = Join-Path $output $tag
      $arguments = "-Plan self-tests -State $state -Tag $tag -AllowInactive -WindowHandle $($fixture.Handle.ToInt64())"
      Assert ((Invoke-Tool 'Capture-Matrix' $arguments $log) -eq 0) "Capture failed; see $log.stderr"
      $capture = (Get-Content "$log.stdout" -Raw).Trim()
      $metadata = Get-Content ([IO.Path]::ChangeExtension($capture, '.json')) -Raw | ConvertFrom-Json
      Assert (-not $metadata.Active -and $metadata.AllowInactive) 'Inactive capture metadata missing'
      $image = [Drawing.Bitmap]::FromFile($capture)
      try {
        Assert ($image.Width -eq 240 -and $image.Height -eq 120) 'Client capture size incorrect'
        Assert ($image.GetPixel(20, 20).ToArgb() -eq [Drawing.Color]::White.ToArgb()) 'Capture is not the opaque fixture'
      } finally { $image.Dispose() }
      $captures += $capture
    }
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $tools 'Compare-Captures.ps1') -Before $captures[0] -After $captures[1]
    Assert ($LASTEXITCODE -eq 0) 'Unchanged native captures must compare equal'
    'PASS: inactive target rejected by default; opt-in captures record state, exact dimensions, opaque pixels and zero differences'
  } finally { $fixture.Dispose() }
}
"Evidence: $output"
