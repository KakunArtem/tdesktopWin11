# Window material state checks

Date: 2026-11-04. Code: `8dc1a436d6`, Debug. Windows: build 26300.9711.
Qt: 6.11.2. GPUs reported by the machine: AMD Radeon(TM) Graphics and
NVIDIA GeForce RTX 5080; QRhi selected Direct3D. Interface scale: 100%.

The app scenarios used an isolated, disposable profile with no authorized
account. Observers and fixture setup stayed in the disposable test overlay;
production files contain no test instrumentation. Evidence is held locally
under the C2 execution workspace, not committed with account data or captures.

## Automated checks

| Row | How | Result | Evidence |
| --- | --- | --- | --- |
| Policy and UI mirror | Run `test_window_material.exe` | Pass | 21 checks, 0 failures; current-value replay, activation, fallback and duplicate suppression included. |
| Settings placement and save | Build the real controller-free window-title content and toggle its checkbox both ways | Pass | Two rows; transparency immediately follows native frame; both KV writes observed. |
| Unsupported build | Temporarily make `Supported()` false, rebuild and inspect the same content; restore the real build check afterward | Pass | Transparency absent; native-frame row retained. No override retained. |
| Preference across restart | Save true in one process, start another with no authorized account | Pass after correction | The first run exposed `resetOnLastLogout()` clearing the KV preference; the fixed run retains true, like the native-frame preference. |
| Setting off | Restart with the preference false and construct another `MainWindow` | Pass for native state | Both windows have `WA_TranslucentBackground` clear; active state and mirror are false; changing the preference alone cannot change capability. |
| Setting on | Restart with the preference true, transparency effects on, high contrast off and native frame off | Pass | Capability, active state and UI mirror are true. |
| Additional main window | Construct another real `Window::Controller` and its native main window | Pass | Exactly one startup policy decision and two successful backdrop applications for two windows. This covers the shared window path without accessing a signed-in chat. |
| Producer consistency | Observe both state producers and repeat `Refresh()` | Pass | Subscribers observe matching state; repeated refresh emits no duplicate value. |
| Native frame live | Enable and disable the existing native-frame setting without restarting | Pass | Active state and mirror change true → false → true; startup capability stays true. |
| Native frame at startup | Save native frame on and restart | Pass | `refusal: native frame`, no backdrop application, ordinary opaque window creation. |
| Backdrop failure | Call the production `ReportBackdropFailure()` entry point twice | Pass | One false transition, capability remains false after refresh, and a later main window is created opaque. This is injected failure reporting, not a simulated failing DWM HRESULT. |

## Manual and screen checks

| Row | How | Result | Evidence |
| --- | --- | --- | --- |
| Transparency effects live | User turns Windows Transparency effects off, then on while the observer runs | Pending | Observer started capable and active, with transparency on and high contrast off. |
| High contrast live | User turns high contrast on, then off in the same process | Pending | No system settings are changed by a script. |
| Screen-level solid parity | `Test-Backdrop.ps1 -Expect Opaque` on an exposed setting-off window | Pending | Native window flags and state passed; a screen measurement is a separate check. |
| Signed-in transparency control | Enable the new Telegram setting and restart | Pass for visible material | The user confirmed transparency is visible; the normal client logged `refusal: none` and successful DWM calls after restart. The dialog's individual button gesture was not separately observed. |
| Separate-chat visual smoke | Open a chat in another window | Pending | The additional native `MainWindow` path passed; the actual signed-in gesture and visual result are not claimed. |

## Execution notes

- The native-frame checkbox applies live. C2 therefore guards that state
  immediately instead of assuming the existing setting requests a restart.
- The remaining prototype strip follows active state and existing windows
  repaint after a change, including a reported native-backdrop failure.
- The `window_material` KV preference survives the account-reset path, like
  the existing native-frame preference. No binary-stream field was added.
- C2 remains in progress until the pending rows are resolved or explicitly
  deferred by the user. C3 and later component work has not started.
- A fresh whole-branch review found no code issues. The original no-op test
  scenario was restored, the normal Debug executable rebuilt successfully,
  and the 21-check standalone suite passed again.
- The first live observer timed out without any OS-setting cycles while the
  visible region and the separate Telegram switch were clarified. No passing
  live OS result is inferred from that run.
