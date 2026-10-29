"""Reproduces the C3 fill calculator and text adjustment for the reference sheet.

Follows the material fills plan: ComputeFill (Task 1) and ReadableOver
(Task 2, using the OKLab scan of EnsureContrast in chat_theme_readability.cpp,
without the olive hue nudge, which only affects yellow hues).
Run: python3 -I compute-fills.py
"""
import json
import math

FLOORS = {
    'Window': (0.72, 0.74), 'Navigation': (0.78, 0.82), 'Conversation': (0.79, 0.81),
    'Bar': (0.90, 0.90), 'Composer': (0.96, 0.96), 'Popup': (0.97, 0.97),
}
BLACK, WHITE = (0, 0, 0), (255, 255, 255)


def rgb(h):
    return tuple(int(h[i:i + 2], 16) for i in (1, 3, 5))


def hexa(c):
    return '#%02x%02x%02x' % c


def lum(c):
    m = lambda v: v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4
    r, g, b = (x / 255 for x in c)
    return 0.2126 * m(r) + 0.7152 * m(g) + 0.0722 * m(b)


def contrast(a, b):
    la, lb = lum(a), lum(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


def composite(over, alpha, under):
    return tuple(int(round(o * alpha + u * (1 - alpha))) for o, u in zip(over, under))


def floor(role, dark):
    return FLOORS[role][1 if dark else 0]


def compute_fill(role, surface, texts, dark):
    alpha = floor(role, dark)
    while alpha < 1.0 - 1e-9:
        ok = True
        for t in texts:
            need = min(4.5, contrast(t, surface))
            for back in (BLACK, WHITE):
                if contrast(t, composite(surface, alpha, back)) < need:
                    ok = False
        if ok:
            return round(alpha, 2)
        alpha += 0.01
    return 1.0


def to_linear(v):
    v /= 255
    return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def to_lab(c):
    r, g, b = (to_linear(x) for x in c)
    l = (0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b) ** (1 / 3)
    m = (0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b) ** (1 / 3)
    s = (0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b) ** (1 / 3)
    return (0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s,
            1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
            0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s)


def linear_from_lch(L, C, H):
    a, b = C * math.cos(math.radians(H)), C * math.sin(math.radians(H))
    l = (L + 0.3963377774 * a + 0.2158037573 * b) ** 3
    m = (L - 0.1055613458 * a - 0.0638541728 * b) ** 3
    s = (L - 0.0894841775 * a - 1.2914855480 * b) ** 3
    return (4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
            -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
            -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s)


def from_lch(L, C, H):
    in_gamut = lambda c: all(-1e-4 <= v <= 1 + 1e-4 for v in linear_from_lch(L, c, H))
    if not in_gamut(C):
        lo, hi = 0.0, C
        for _ in range(30):
            mid = (lo + hi) / 2
            lo, hi = (mid, hi) if in_gamut(mid) else (lo, mid)
        C = lo

    def ch(v):
        v = min(max(v, 0.0), 1.0)
        v = 12.92 * v if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055
        return int(round(v * 255))
    return tuple(ch(v) for v in linear_from_lch(L, C, H))


def ensure_contrast(color, backgrounds, need):
    ok = lambda c: all(contrast(c, b) >= need for b in backgrounds)
    if ok(color):
        return color
    L, a, b = to_lab(color)
    C, H = math.hypot(a, b), math.degrees(math.atan2(b, a)) % 360
    up = lum(color) >= lum(backgrounds[0])
    for direction in ((1, -1) if up else (-1, 1)):
        for step in range(1, 161):
            light = L + direction * step * 0.005
            if light < 0 or light > 1:
                break
            now = from_lch(light, C, H)
            if ok(now):
                return now
    return color


def readable_over(text, surface, dark):
    backs = []
    for role in ('Navigation', 'Conversation'):
        f = floor(role, dark)
        backs += [composite(surface, f, BLACK), composite(surface, f, WHITE)]
    return ensure_contrast(text, backs, 4.5)


THEMES = {
    'mockup-light': dict(dark=False, surface='#e9eef5', text='#132335', secondary='#3a4b5f', accent='#004a8f'),
    'mockup-dark': dict(dark=True, surface='#1b2838', text='#f2f6fb', secondary='#c2cfdd', accent='#9ad3ff'),
    'telegram-day': dict(dark=False, surface='#ffffff', text='#000000', secondary='#999999', accent='#168acd'),
    'telegram-night': dict(dark=True, surface='#17212b', text='#f5f5f5', secondary='#708499', accent='#6ab3f3'),
}

result = {}
for name, t in THEMES.items():
    surface, text = rgb(t['surface']), rgb(t['text'])
    secondary, accent = rgb(t['secondary']), rgb(t['accent'])
    adj_secondary = readable_over(secondary, surface, t['dark'])
    adj_accent = readable_over(accent, surface, t['dark'])
    entry = {'adjusted': {
        'secondary': [t['secondary'], hexa(adj_secondary)],
        'accent': [t['accent'], hexa(adj_accent)],
    }, 'fills': {}}
    for role in FLOORS:
        before = compute_fill(role, surface, (text, secondary), t['dark'])
        after = compute_fill(role, surface, (text, adj_secondary), t['dark'])
        worst = min(contrast(c, composite(surface, after, back))
                    for c in (text, adj_secondary) for back in (BLACK, WHITE))
        entry['fills'][role] = {'floor': floor(role, t['dark']), 'before': before,
                                'after': after, 'worst': round(worst, 2)}
    result[name] = entry
print(json.dumps(result, indent=1))
