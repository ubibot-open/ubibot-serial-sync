# Features & User Interface

What UbiBot Serial Assistant actually does and what it looks like — the
short version lives in [README.md](../README.md); this is the full tour.
For "how it's built" rather than "what it does", see
[architecture.md](architecture.md).

## User interface

The window is a single frameless `ApplicationWindow` with its own
custom-drawn title bar (app icon, title, version, minimize/maximize/close)
rather than the OS-native frame — the whole window follows the app's
light/dark theme this way, not just its contents. Below the title bar:

- **A real menu bar** — `File` (save log, exit), `Edit` (clear the log),
  `View` (pause auto-scrolling), `Tools` (open Settings & About), `Help`
  (About; a "Check for software update" item exists in the code but is
  currently commented out — see [Software self-update](#software-self-update-currently-disabled)
  below).
- **A toolbar** — save log, send, pause, clear, a settings gear, and the
  open/close-port button, plus a badge showing the currently selected
  device model.
- **A two-way toggle** switching the left-hand panel between:
  - **Serial** — port/baud/data bits/parity/stop bits/flow control, plus
    receive options (timestamps, line wrap, echo sent data) and transmit
    options (ASCII/HEX mode, CRC16/MODBUS append, repeat-send interval).
  - **Device commands** — the searchable device command library (see
    below).
- **The data monitor** (right-hand pane, always visible regardless of which
  left-hand panel is active) — a color-coded TX/RX/SYS/ERR log with line
  count and byte counters, pause/clear, and a manual-send box (with send
  history) at the bottom.
- **Settings & About** (`Tools` menu or the toolbar gear) — language
  picker, data-monitor/system font pickers, theme toggle, and the device
  command library's own version/update status.
- **About** (`Help` menu) — version, build time, licensing.

There's also a `RemoteAssistPanel.qml` component (session code/OTP
generation, permission toggles) built for a future "Remote support" mode —
see [Remote support (early preview)](#remote-support-early-preview) below
for its current state.

## Features

### Serial I/O (`QSerialPort`)
- Full port configuration: baud rate, data bits, parity, stop bits, flow
  control.
- ASCII or HEX transmit/receive.
- Optional CRC16/MODBUS checksum, appended automatically to every outgoing
  send (manual or device-library) when enabled.
- Repeat-send: resend the current manual-send box contents on a fixed
  interval.
- Echo sent data into the log, with per-line timestamps and line-wrap
  toggles.
- Manual-send history — every sent line is kept, double-click to reuse.

### Device command library (`resources/devices.json`)
- Per-model command sets, grouped and searchable, for both plain-text AT
  commands (`AT+INTERVAL=<sec>`) and JSON-payload devices (base64-encoded
  command bodies) — see
  [device-json-protocol-schema.md](device-json-protocol-schema.md) for the
  exact schema.
- A parameter panel pops up for any command that takes arguments; picking a
  model that ships recommended serial settings offers to apply them.
- Drag-to-reorder the command list; "My templates" for your own quick-send
  snippets (add/edit/delete, unrelated to any specific device); "Batch
  commands" for a saved, named sequence of steps sent one at a time on a
  fixed interval (each step independently ASCII/HEX and CRC-toggle-able).
- Add a model or command by editing the JSON — no recompile needed for
  content changes.

### Device command library remote updates
- The device/command data above doesn't have to be baked into the binary:
  the app can check a configured backend for a newer version and download
  it in place (no restart required) — see
  [device-library-update-protocol.md](device-library-update-protocol.md)
  for the full protocol and how it's wired up.

### Software self-update (currently disabled)
- A full "check for a new app version → download → verify → swap →
  relaunch" flow exists (Windows-only) and works end to end, but its menu
  entry is commented out for now because the backend product/version
  registration it depends on hasn't been deployed yet — see
  [app-self-update.md](app-self-update.md) for the mechanism and
  `qml/Main.qml`'s `&Help` menu for the (commented-out) entry point.

### Data monitor
- Color-coded TX/RX/SYS/ERR log, byte counters, pause/clear.
- One-shot export to plain text / CSV / HEX dump.
- Optional continuous, daily-rotating file logging.

### Remote support (early preview)
- Session-code/OTP generation and permission toggles are fully built in
  QML, but there's no signaling/relay server or peer-to-peer transport
  behind them yet, and (see [architecture.md](architecture.md#design-notes))
  the panel isn't currently reachable from the main window's mode toggle at
  all. Wiring up a real transport — and exposing the panel — is future
  work.

### Internationalization
- English, Simplified & Traditional Chinese, Japanese, Korean, Russian,
  French, and Italian today, switchable at runtime from Settings & About,
  each language shown by its own native name. See
  [architecture.md](architecture.md#internationalization) for how to add
  another.

### Light/dark theme
Switchable at runtime, applied to the whole window including its custom
title bar.

### Cross-platform
One Qt/QML/C++17 codebase builds standalone packages for Windows (`.zip`),
macOS (`.dmg`), and Linux (`.AppImage`), all produced automatically by
[.github/workflows/release.yml](../.github/workflows/release.yml) on every
tagged release — see README.md's
[Publishing a release build via GitHub Actions](../README.md#publishing-a-release-build-via-github-actions).

## Advantages

- **Native and lightweight** — Qt Quick/QML, not a bundled browser runtime;
  a real desktop app with a small footprint and fast startup, not an
  Electron wrapper.
- **Data-driven device support** — adding a new UbiBot model or command is
  a JSON edit, not a code change or a recompile, and (via the remote-update
  mechanism) doesn't even require shipping a new app build to reach users
  who already have it installed.
- **One codebase, three platforms** — Windows/macOS/Linux builds come from
  the same source tree and the same CI pipeline, not three separately
  maintained ports.
- **Fully localized**, not just translated strings bolted on after the
  fact — every UI surface (including the frameless title bar) respects the
  active language and theme.
- **Free for commercial use** — LGPLv3, dynamically linked against Qt (see
  README.md's [Qt 6 module licensing](../README.md#qt-6-module-licensing-commercial-use-check)
  section) — no Qt commercial license needed to use, modify, resell, or
  embed this software.
- **Reproducible, automated releases** — every tagged version is built and
  published by CI from a clean checkout, not hand-assembled on someone's
  laptop.
