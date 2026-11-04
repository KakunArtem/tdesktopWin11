using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace UiChecks {
public sealed class BackdropForm : Form {
	public BackdropForm() {
		FormBorderStyle = FormBorderStyle.None;
		ShowInTaskbar = false;
		AutoScaleMode = AutoScaleMode.None;
		StartPosition = FormStartPosition.Manual;
	}
	protected override bool ShowWithoutActivation { get { return true; } }
	protected override CreateParams CreateParams {
		get { var p = base.CreateParams; p.ExStyle |= 0x08000080; return p; }
	}
}

public static class Native {
	[StructLayout(LayoutKind.Sequential)]
	public struct RECT { public int Left, Top, Right, Bottom; }
	[StructLayout(LayoutKind.Sequential)]
	public struct POINT { public int X, Y; }
	private delegate bool EnumProc(IntPtr handle, IntPtr data);
	[DllImport("user32.dll")] private static extern bool EnumWindows(EnumProc callback, IntPtr data);
	[DllImport("user32.dll")] public static extern bool IsWindow(IntPtr handle);
	[DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr handle);
	[DllImport("user32.dll")] public static extern bool IsIconic(IntPtr handle);
	[DllImport("user32.dll")] private static extern uint GetWindowThreadProcessId(IntPtr handle, out uint process);
	[DllImport("user32.dll")] private static extern bool GetClientRect(IntPtr handle, out RECT rect);
	[DllImport("user32.dll")] private static extern bool ClientToScreen(IntPtr handle, ref POINT point);
	[DllImport("user32.dll")] private static extern bool GetWindowRect(IntPtr handle, out RECT rect);
	[DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
	[DllImport("user32.dll")] private static extern IntPtr GetAncestor(IntPtr handle, uint flags);
	[DllImport("user32.dll")] public static extern IntPtr GetWindow(IntPtr handle, uint command);
	[DllImport("user32.dll")] public static extern uint GetDpiForWindow(IntPtr handle);
	[DllImport("user32.dll", SetLastError = true)]
	public static extern bool SetWindowPos(IntPtr handle, IntPtr after, int x, int y, int width, int height, uint flags);
	[DllImport("user32.dll")] private static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);
	[DllImport("user32.dll")] private static extern IntPtr GetDC(IntPtr handle);
	[DllImport("user32.dll")] private static extern int ReleaseDC(IntPtr handle, IntPtr dc);
	[DllImport("gdi32.dll", SetLastError = true)]
	private static extern bool BitBlt(IntPtr destination, int x, int y, int width, int height, IntPtr source, int sourceX, int sourceY, uint operation);
	[DllImport("dwmapi.dll")] private static extern int DwmGetWindowAttribute(IntPtr handle, int key, out RECT rect, int size);
	[DllImport("dwmapi.dll")] private static extern int DwmGetWindowAttribute(IntPtr handle, int key, out int value, int size);

	public static void PhysicalPixels() {
		if (SetThreadDpiAwarenessContext(new IntPtr(-4)) == IntPtr.Zero) {
			throw new InvalidOperationException("Cannot enable per-monitor physical pixel coordinates.");
		}
	}
	private static Rectangle RectangleOf(RECT rect) {
		return Rectangle.FromLTRB(rect.Left, rect.Top, rect.Right, rect.Bottom);
	}
	public static Rectangle ClientBounds(IntPtr handle) {
		RECT rect;
		var point = new POINT();
		if (!GetClientRect(handle, out rect) || !ClientToScreen(handle, ref point)) {
			throw new InvalidOperationException("Cannot read target client bounds.");
		}
		return new Rectangle(point.X, point.Y, rect.Right, rect.Bottom);
	}
	public static Rectangle FrameBounds(IntPtr handle) {
		RECT rect;
		if (DwmGetWindowAttribute(handle, 9, out rect, Marshal.SizeOf(typeof(RECT))) != 0 &&
				!GetWindowRect(handle, out rect)) { throw new InvalidOperationException("Cannot read target frame bounds."); }
		return RectangleOf(rect);
	}
	public static bool Visible(IntPtr handle) {
		int cloaked;
		return IsWindow(handle) && IsWindowVisible(handle) && !IsIconic(handle) &&
			(DwmGetWindowAttribute(handle, 14, out cloaked, 4) != 0 || cloaked == 0);
	}
	public static bool Active(IntPtr handle) {
		var foreground = GetForegroundWindow();
		return foreground != IntPtr.Zero && GetAncestor(handle, 3) == GetAncestor(foreground, 3);
	}
	public static IntPtr[] Windows(uint process) {
		var result = new List<IntPtr>();
		EnumWindows(delegate(IntPtr handle, IntPtr data) {
			uint owner;
			GetWindowThreadProcessId(handle, out owner);
			if (owner == process && Visible(handle)) {
				var bounds = ClientBounds(handle);
				if (bounds.Width > 0 && bounds.Height > 0) { result.Add(handle); }
			}
			return true;
		}, IntPtr.Zero);
		return result.ToArray();
	}
	public static void RequireExposed(IntPtr handle, Rectangle bounds) {
		if (!Visible(handle) || !Screen.FromHandle(handle).Bounds.Contains(bounds)) {
			throw new InvalidOperationException("Keep the visible capture on one monitor.");
		}
		for (var above = GetWindow(handle, 3); above != IntPtr.Zero; above = GetWindow(above, 3)) {
			if (Visible(above) && FrameBounds(above).IntersectsWith(bounds)) {
				throw new InvalidOperationException("Target is covered by another window. Bring it to the front and retry.");
			}
		}
	}
	public static IntPtr SelectWindow(uint process, bool popup) {
		var windows = Windows(process);
		var main = IntPtr.Zero;
		long largest = 0;
		foreach (var window in windows) {
			var r = ClientBounds(window);
			var area = (long)r.Width * r.Height;
			if (area > largest) { largest = area; main = window; }
		}
		if (!popup) { return main; }
		foreach (var window in windows) { if (window != main) { return window; } }
		return IntPtr.Zero;
	}
	public static Bitmap Capture(Rectangle bounds) {
		var bitmap = new Bitmap(bounds.Width, bounds.Height, PixelFormat.Format24bppRgb);
		try {
			using (var graphics = Graphics.FromImage(bitmap)) {
				var screen = GetDC(IntPtr.Zero);
				if (screen == IntPtr.Zero) { throw new InvalidOperationException("Cannot access the interactive desktop."); }
				try {
					var destination = graphics.GetHdc();
					try {
						if (!BitBlt(destination, 0, 0, bounds.Width, bounds.Height, screen, bounds.X, bounds.Y, 0x40CC0020)) {
							throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
						}
					} finally { graphics.ReleaseHdc(destination); }
				} finally { ReleaseDC(IntPtr.Zero, screen); }
			}
			return bitmap;
		} catch { bitmap.Dispose(); throw; }
	}
}

public sealed class SampleResult {
	public string Background, Text;
	public double Ratio;
}

public sealed class Difference {
	public long Changed, SampledChanged, SampledTotal;
	public Rectangle Bounds;
}

public static class Pixels {
	public static double Luminance(Color color) {
		return 0.2126 * Linear(color.R) + 0.7152 * Linear(color.G) + 0.0722 * Linear(color.B);
	}
	private static double Linear(byte value) {
		var c = value / 255.0;
		return c <= 0.04045 ? c / 12.92 : Math.Pow((c + 0.055) / 1.055, 2.4);
	}
	public static double Contrast(Color a, Color b) {
		var x = Luminance(a);
		var y = Luminance(b);
		return (Math.Max(x, y) + 0.05) / (Math.Min(x, y) + 0.05);
	}
	public static Color[] Read(Bitmap source, Rectangle region) {
		using (var bitmap = source.Clone(region, PixelFormat.Format32bppArgb)) {
			var data = bitmap.LockBits(new Rectangle(Point.Empty, bitmap.Size), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
			try {
				var pixels = new Color[bitmap.Width * bitmap.Height];
				var row = new byte[bitmap.Width * 4];
				for (var y = 0; y < bitmap.Height; ++y) {
					Marshal.Copy(IntPtr.Add(data.Scan0, y * data.Stride), row, 0, row.Length);
					for (var x = 0; x < bitmap.Width; ++x) {
						pixels[y * bitmap.Width + x] = Color.FromArgb(row[x * 4 + 3], row[x * 4 + 2], row[x * 4 + 1], row[x * 4]);
					}
				}
				return pixels;
			} finally { bitmap.UnlockBits(data); }
		}
	}
	private static Color Background(Color[] pixels, Color? text) {
		var counts = new int[4096];
		var reds = new long[4096];
		var greens = new long[4096];
		var blues = new long[4096];
		var best = -1;
		foreach (var pixel in pixels) {
			if (text.HasValue && Contrast(pixel, text.Value) <= 1.5) { continue; }
			var bin = ((pixel.R >> 4) << 8) | ((pixel.G >> 4) << 4) | (pixel.B >> 4);
			++counts[bin]; reds[bin] += pixel.R; greens[bin] += pixel.G; blues[bin] += pixel.B;
			if (best < 0 || counts[bin] > counts[best]) { best = bin; }
		}
		if (best < 0) { throw new ArgumentException("No background remains after excluding foreground pixels."); }
		return Color.FromArgb((int)(reds[best] / counts[best]), (int)(greens[best] / counts[best]), (int)(blues[best] / counts[best]));
	}
	private static Color Foreground(Color[] pixels, Color background) {
		var result = pixels[0];
		var best = 0.0;
		foreach (var pixel in pixels) {
			var ratio = Contrast(pixel, background);
			if (ratio > best) { result = pixel; best = ratio; }
		}
		return result;
	}
	private static string Hex(Color color) { return String.Format("#{0:x2}{1:x2}{2:x2}", color.R, color.G, color.B); }
	public static SampleResult Measure(Bitmap bitmap, Rectangle region, string foreground) {
		var pixels = Read(bitmap, region);
		var text = String.IsNullOrEmpty(foreground) ? Foreground(pixels, Background(pixels, null)) : ColorTranslator.FromHtml(foreground);
		var background = Background(pixels, text);
		if (String.IsNullOrEmpty(foreground)) { text = Foreground(pixels, background); }
		return new SampleResult { Background = Hex(background), Text = Hex(text), Ratio = Contrast(text, background) };
	}
	public static Difference Compare(Bitmap before, Bitmap after, int tolerance, int step, bool sum) {
		if (before.Size != after.Size) { throw new ArgumentException("Image sizes differ."); }
		var a = Read(before, new Rectangle(Point.Empty, before.Size));
		var b = Read(after, new Rectangle(Point.Empty, after.Size));
		var result = new Difference();
		var left = before.Width; var top = before.Height; var right = -1; var bottom = -1;
		for (var y = 0; y < before.Height; ++y) {
			for (var x = 0; x < before.Width; ++x) {
				var i = y * before.Width + x;
				var red = Math.Abs(a[i].R - b[i].R);
				var green = Math.Abs(a[i].G - b[i].G);
				var blue = Math.Abs(a[i].B - b[i].B);
				var alpha = Math.Abs(a[i].A - b[i].A);
				var changed = (sum ? red + green + blue : Math.Max(alpha, Math.Max(red, Math.Max(green, blue)))) > tolerance;
				if (changed) {
					++result.Changed;
					left = Math.Min(left, x); top = Math.Min(top, y); right = Math.Max(right, x); bottom = Math.Max(bottom, y);
				}
				if (x % step == 0 && y % step == 0) {
					++result.SampledTotal;
					if (changed) { ++result.SampledChanged; }
				}
			}
		}
		result.Bounds = result.Changed == 0 ? Rectangle.Empty : Rectangle.FromLTRB(left, top, right + 1, bottom + 1);
		return result;
	}
}
}
