[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)][string]$Before,
  [Parameter(Mandatory = $true)][string]$After,
  [ValidateRange(0, 255)][int]$Tolerance = 8
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Ui-Checks.Common.ps1')
$first = $null; $second = $null
try {
  $first = [Drawing.Bitmap]::FromFile([IO.Path]::GetFullPath($Before))
  $second = [Drawing.Bitmap]::FromFile([IO.Path]::GetFullPath($After))
  if ($first.Size -ne $second.Size) {
    "Size mismatch: before $($first.Width)x$($first.Height), after $($second.Width)x$($second.Height)."
    exit 2
  }
  $result = [UiChecks.Pixels]::Compare($first, $second, $Tolerance, 1, $false)
  "Different pixels: $($result.Changed) of $([long]$first.Width * $first.Height)"
  $bounds = $result.Bounds
  "Changed bounds: $($bounds.X),$($bounds.Y),$($bounds.Width),$($bounds.Height)"
  if ($result.Changed -gt 0) { exit 1 }
  exit 0
} finally {
  if ($first) { $first.Dispose() }; if ($second) { $second.Dispose() }
}
