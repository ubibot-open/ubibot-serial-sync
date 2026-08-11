# UbiBot Serial Assistant / UbiBot 串口助手

A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS,
SP1, …), built with Qt 6.11 / C++17 / CMake. Ships with English and
Simplified Chinese today, picked from a dropdown in Settings & About (built
to scale past a dozen languages), and is structured so adding more later is
a matter of dropping in one more `.ts` file.

This project was scaffolded from a design mockup; see
[Project notes](#project-notes) below for what's real vs. placeholder.

## Features

- **Serial I/O** over `QSerialPort` — port/baud/data bits/parity/stop
  bits/flow control, ASCII/HEX receive & transmit, timestamps, repeat send.
- **Device command library** (`resources/devices.json`) — per-model AT
  command sets, grouped, searchable, favoritable, with a parameter panel for
  commands that need arguments (e.g. `AT+INTERVAL=<sec>`). Add a model or
  command by editing the JSON; no recompile needed for content changes.
- **Data monitor** — color-coded TX/RX/SYS/ERR log with byte counters,
  pause/clear, and one-shot export to plain text / CSV / HEX dump, plus
  optional continuous, daily-rotating file logging.
- **Connection wizard** — pick a detected port, pick a device model, connect
  at 115200 8-N-1.
- **Settings & About** — switch the interface language at runtime from a
  dropdown, see the bundled command-library version.
- **Remote support panel** — UI only for now; see
  [Project notes](#project-notes).

## Requirements

- Qt **6.11** (or any 6.7+ toolchain — that's the floor `qt_add_translations`
  needs; 6.11 is what this project has been validated against), with the
  **Widgets**, **SerialPort**, and **Network** modules, plus the
  **Qt Linguist tools** (`lupdate`/`lrelease`, installed by default with the
  Qt Creator/CMake components).
- CMake ≥ 3.21.
- A C++17 compiler: MSVC 2019+/2022, MinGW, GCC, or Clang.

## Building

### Qt Creator

Open `CMakeLists.txt` as a project, pick a Qt 6.11 kit, and build/run — no
extra configuration needed.

### Command line (Windows, MSVC + Ninja)

Run from a "Developer PowerShell/Command Prompt for VS" so `cl.exe` is on
`PATH`, and point `CMAKE_PREFIX_PATH` at your Qt install:

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable lands in `build/UbiBotSerialAssistant.exe`. For a
double-clickable, standalone build, run Qt's deploy tool afterwards:

```bash
C:/Qt/6.11.0/msvc2022_64/bin/windeployqt.exe build/UbiBotSerialAssistant.exe
```

### macOS / Linux

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64
cmake --build build
```

## Project layout

```
CMakeLists.txt
resources/
  devices.json        # device/command catalog (bilingual labels, see below)
  icons/*.svg          # toolbar icons
  resources.qrc
translations/
  ubibot_zh_CN.ts      # Simplified Chinese catalog (hand-authored; see below)
src/
  main.cpp
  core/                # no Qt Widgets dependency — I/O, data, settings, i18n
    language_manager.*
    device_library.*
    log_entry.h
    log_manager.*
    serial_manager.*
    settings_store.*
  ui/                  # all the widgets
    mainwindow.*
    serial_settings_panel.*
    command_library_panel.*
    remote_assist_panel.*
    command_params_panel.*
    data_monitor_view.*
    connection_wizard.*
    save_log_dialog.*
    settings_about_dialog.*
    flow_layout.*        # wrapping layout for the filter-chip row
    styles.*
```

## Internationalization

Every UI string in the C++ code is written in English and wrapped in
`tr(...)`. Simplified Chinese is a plain Qt translation catalog
(`translations/ubibot_zh_CN.ts`). Only one language is ever active at a time
— no mixed/bilingual display — chosen from a dropdown in Settings & About
that's populated at runtime from `LanguageManager::availableLanguages()`
(`core/language_manager.cpp`); a dropdown rather than radio buttons because
this list is meant to grow past a dozen entries. Each entry's display text
is that language's own native name (e.g. "简体中文", "English"), so the
list itself never needs retranslating.

Switching languages at runtime (Settings & About dialog) works by
installing/removing a `QTranslator` on `QApplication`, which makes Qt post a
`QEvent::LanguageChange` to every top-level widget; each custom widget/dialog
in this project overrides `changeEvent()` and re-runs its `retranslateUi()`
in response, so the whole UI updates live with no restart.

**Adding a language:** add one more `TS_FILES` entry to the
`qt_add_translations()` call in `CMakeLists.txt`, add a matching
`{code, nativeName}` to `kLanguages` in `core/language_manager.cpp`, and fill
in the `.ts` once `lupdate` has generated its `<message>` entries. The
Settings & About dropdown picks it up automatically — nothing else changes.

**Editing translations:** after adding/changing `tr()` calls, regenerate the
`.ts` file's `<message>` entries with Qt's `lupdate` (the CMake target this
project's `qt_add_translations()` sets up handles this — check your
generator's target list, e.g. `update_translations`), fill in
`<translation>` for the new strings, then rebuild so `lrelease` regenerates
the embedded `.qm`.

**Device command text** (model descriptions, command names, parameter
labels) lives in `resources/devices.json` as `{ "zh": "…", "en": "…" }`
pairs rather than going through `tr()`, since it's data, not source code —
see `LocalizedText` in `core/device_library.h`. `LanguageManager::pick()`
resolves it to whichever single language is currently active.

## Project notes

A few implementation choices worth knowing about before you dig in:

- **Window chrome is a standard `QMainWindow`** (native title bar, menu bar,
  toolbar), not the frameless custom-drawn window in the original design
  mockup — far less platform-specific edge-case code to maintain, at the
  cost of not being a pixel-perfect match.
- **Remote support is a UI placeholder.** Generating a session code/OTP and
  setting permissions all work, but there's no signaling/relay server or
  peer-to-peer transport behind "Start session" yet — it explains that
  plainly instead of pretending to connect. Wiring up a real transport is
  future work.
- **"Recommended" port detection** in the connection wizard is a heuristic:
  it flags ports whose USB vendor ID matches the WCH CH340 (`0x1A86`) or
  Silicon Labs CP210x (`0x10C4`) bridge chips UbiBot's own USB-serial
  adapters use — not a guarantee that a given port is actually a UbiBot
  device.
- **Command-library "check for updates"** only reports the bundled JSON's
  version string; there's no update server in this build.
