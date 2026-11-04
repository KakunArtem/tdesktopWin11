# Places a solid window behind Telegram, captures Telegram's window in red
# and in blue, and reports how many pixels changed between the captures.
param(
  [ValidateSet('Transparent', 'Opaque')][string]$Expect = 'Transparent',
  [string]$OutDir = (Join-Path $env:TEMP 'tdesktop-backdrop-check'),
  [int]$Step = 3,
  [long]$WindowHandle = 0
)
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
Add-Type @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public static class BackdropWin32 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  public delegate bool EnumProc(IntPtr hWnd, IntPtr lParam);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc proc, IntPtr lParam);
  [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
  [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr hWnd, int attribute, out RECT rect, int size);
  public static RECT VisibleBounds(IntPtr hWnd) {
    RECT r;
    if (DwmGetWindowAttribute(hWnd, 9, out r, Marshal.SizeOf(typeof(RECT))) != 0) { GetWindowRect(hWnd, out r); }
    return r;
  }
  public static IntPtr LargestVisibleWindow(uint pid) {
    IntPtr best = IntPtr.Zero; long bestArea = 0;
    EnumWindows((hWnd, l) => {
      uint owner; GetWindowThreadProcessId(hWnd, out owner);
      RECT r;
      if (owner == pid && IsWindowVisible(hWnd) && GetWindowRect(hWnd, out r)) {
        long area = (long)(r.Right - r.Left) * (r.Bottom - r.Top);
        if (area > bestArea) { bestArea = area; best = hWnd; }
      }
      return true;
    }, IntPtr.Zero);
    return best;
  }
}
'@
[BackdropWin32]::SetProcessDPIAware() | Out-Null

if ($WindowHandle -ne 0) {
  $telegram = [IntPtr]$WindowHandle
} else {
  $process = Get-Process Telegram -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -like '*tdesktopWin11*' } |
    Select-Object -First 1
  if (-not $process) { Write-Error 'Start out\Debug\Telegram.exe first.'; exit 2 }
  $telegram = [BackdropWin32]::LargestVisibleWindow([uint32]$process.Id)
}
if ($telegram -eq [IntPtr]::Zero) { Write-Error 'No visible Telegram window found.'; exit 2 }
$bounds = [BackdropWin32]::VisibleBounds($telegram)
$inset = 2
$rect = New-Object BackdropWin32+RECT
$rect.Left = $bounds.Left + $inset; $rect.Top = $bounds.Top + $inset
$rect.Right = $bounds.Right - $inset; $rect.Bottom = $bounds.Bottom - $inset
$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top

$form = New-Object System.Windows.Forms.Form
$form.FormBorderStyle = 'None'
$form.ShowInTaskbar = $false
$form.StartPosition = 'Manual'
$form.Bounds = New-Object System.Drawing.Rectangle(($rect.Left - 40), ($rect.Top - 40), ($width + 80), ($height + 80))
$form.BackColor = [System.Drawing.Color]::Red
$form.Show()
$SWP_NOACTIVATE = 0x0010
[BackdropWin32]::SetWindowPos($form.Handle, $telegram, $form.Left, $form.Top, $form.Width, $form.Height, $SWP_NOACTIVATE) | Out-Null
[BackdropWin32]::SetForegroundWindow($telegram) | Out-Null

function Capture([string]$name) {
  [System.Windows.Forms.Application]::DoEvents()
  Start-Sleep -Milliseconds 900
  $bitmap = New-Object System.Drawing.Bitmap($width, $height)
  $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
  $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
  $graphics.Dispose()
  $bitmap.Save((Join-Path $OutDir "$name.png"))
  return $bitmap
}

New-Item -ItemType Directory -Force $OutDir | Out-Null
$red = Capture 'behind-red'
$form.BackColor = [System.Drawing.Color]::Blue
$form.Refresh()
$blue = Capture 'behind-blue'
$form.Close()

$changed = 0; $total = 0
for ($y = 0; $y -lt $height; $y += $Step) {
  for ($x = 0; $x -lt $width; $x += $Step) {
    $a = $red.GetPixel($x, $y); $b = $blue.GetPixel($x, $y)
    $total++
    if ([math]::Abs($a.R - $b.R) + [math]::Abs($a.G - $b.G) + [math]::Abs($a.B - $b.B) -gt 30) { $changed++ }
  }
}
$red.Dispose(); $blue.Dispose()
$fraction = $changed / [math]::Max(1, $total)
"Changed pixels: $changed of $total sampled ({0:P2}). Captures: $OutDir" -f $fraction
if ($Expect -eq 'Transparent') { if ($fraction -gt 0.002) { 'PASS: backdrop shows what is behind the window'; exit 0 } else { 'FAIL: window is opaque'; exit 1 } }
else { if ($fraction -le 0.002) { 'PASS: window is opaque'; exit 0 } else { 'FAIL: window shows what is behind it'; exit 1 } }
