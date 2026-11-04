[CmdletBinding()]
param(
  [string]$Samples,
  [string]$Out,
  [switch]$SelfTest,
  [switch]$AllowInactive,
  [long]$WindowHandle = 0,
  [ValidateRange(0, 15)][int]$Monitor = 0
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Show-Backdrop.ps1') -WindowHandle $WindowHandle

if ($SelfTest) {
  $fixture = New-Object UiChecks.BackdropForm
  $fixture.Bounds = Get-CheckFixtureBounds -Monitor $Monitor -Width 360 -Height 120
  $fixture.TopMost = $true
  $font = New-Object Drawing.Font('Segoe UI', 13, [Drawing.FontStyle]::Regular, [Drawing.GraphicsUnit]::Pixel)
  foreach ($row in 0..1) {
    $label = New-Object Windows.Forms.Label
    $label.Bounds = New-Object Drawing.Rectangle(0, ($row * 60), 360, 60)
    $label.Font = $font
    $label.Text = 'Antialiased text sample 0123456789'
    $label.ForeColor = [Drawing.ColorTranslator]::FromHtml(@('#3a4b5f', '#999999')[$row])
    $label.BackColor = [Drawing.ColorTranslator]::FromHtml(@('#e9eef5', '#ffffff')[$row])
    $fixture.Controls.Add($label)
  }
  try {
    $fixture.Show()
    Wait-CheckPaint
    $bitmap = Get-CheckCapture -Handle $fixture.Handle -Client -AllowInactive
    try {
      $good = Measure-CheckSample $bitmap ([pscustomobject]@{ name = 'readable'; rect = @(0, 0, 360, 60); kind = 'text'; text = '#3a4b5f' })
      $bad = Measure-CheckSample $bitmap ([pscustomobject]@{ name = 'unreadable'; rect = @(0, 60, 360, 60); kind = 'text'; text = '#999999' })
      $invalid = Measure-CheckSample $bitmap ([pscustomobject]@{ name = 'outside'; rect = @(350, 110, 20, 20); kind = 'text'; text = '#000000' })
      if ($good.Result -ne 'PASS' -or [Math]::Abs($good.Ratio - 7.66) -gt 0.1 -or
          $bad.Result -ne 'FAIL' -or [Math]::Abs($bad.Ratio - 2.85) -gt 0.1 -or $invalid.Result -ne 'INVALID') {
        throw "Contrast self-test failed: $($good | ConvertTo-Json -Compress), $($bad | ConvertTo-Json -Compress), $($invalid.Result)"
      }
      'SELFTEST PASS: native antialiased labels 7.66 PASS / 2.85 FAIL; outside rectangle INVALID'
    } finally { $bitmap.Dispose() }
  } finally { $fixture.Dispose(); $font.Dispose() }
  exit 0
}

if (-not $Samples) { throw 'Specify -Samples <JSON file> or -SelfTest.' }
$definitions = @(Get-Content -LiteralPath $Samples -Raw | ConvertFrom-Json)
if ($definitions.Count -eq 0) { throw 'Samples array must not be empty.' }
$target = Get-CheckWindow -WindowHandle $WindowHandle
$folder = Join-Path $env:LOCALAPPDATA ('tdesktop-ui-checks/contrast/' + [DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $folder -Force | Out-Null
if (-not $Out) { $Out = Join-Path $folder 'report.md' }
Assert-CheckOutput $Out
$lines = @('# Native contrast check', '', "Date: $([DateTime]::Now.ToString('s')). Physical client pixels. Captures: $folder", '',
  '| Sample | Backdrop | Background | Text | Ratio | Threshold | Result |', '| --- | --- | --- | --- | --- | --- | --- |')
$failures = 0
$initialBounds = [UiChecks.Native]::ClientBounds($target)
$initialActive = [UiChecks.Native]::Active($target)
$lines[2] += " Activation: $initialActive; AllowInactive: $AllowInactive."
foreach ($color in @('#000000', '#ffffff')) {
  $backdrop = $null; $bitmap = $null
  try {
    $backdrop = Show-Backdrop -Color $color -WindowHandle $target.ToInt64()
    if ([UiChecks.Native]::ClientBounds($target) -ne $initialBounds) { throw 'Target moved or resized; samples no longer match.' }
    $bitmap = Get-CheckCapture -Handle $target -Client -AllowInactive:$AllowInactive
    if ([bool]$bitmap.Tag -ne $initialActive) { throw 'Target activation changed between backdrops; retry.' }
    $bitmap.Save((Join-Path $folder ($color.Substring(1) + '.png')))
    foreach ($sample in $definitions) {
      $result = Measure-CheckSample $bitmap $sample
      if ($result.Result -ne 'PASS') { $failures++ }
      $name = ($result.Name -replace '\|', '\|' -replace '[\r\n]', ' ')
      $ratio = $result.Ratio.ToString('F2', [Globalization.CultureInfo]::InvariantCulture)
      $lines += "| $name | $color | $($result.Background) | $($result.Text) | $ratio | $($result.Threshold) | $($result.Result) $($result.Reason -replace '[|\r\n]', ' ') |"
    }
  } finally {
    if ($bitmap) { $bitmap.Dispose() }
    Hide-Backdrop $backdrop
  }
}
[IO.File]::WriteAllText([IO.Path]::GetFullPath($Out), (($lines -join "`r`n") + "`r`n"), (New-Object Text.UTF8Encoding($false)))
$lines
"Failures: $failures. Report: $Out"
if ($failures -gt 0) { exit 1 }
exit 0
