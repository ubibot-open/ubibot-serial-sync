# Building & Architecture

Pre-built releases are published for Windows/macOS/Linux — see
[README.md](README.md) for the download links. **You don't need any of this
to use the app.** This document is for building from source and for the
technical details behind how it's put together.

## Requirements

- Qt **6.11** (or any 6.7+ toolchain — that's the floor `qt_add_translations`
  and the vendored QLight widget library need; 6.11 is what this project has
  been validated against), with the **Widgets**, **Svg**, and **SerialPort**
  modules, plus the **Qt Linguist tools** (`lupdate`/`lrelease`, installed by default with the Qt
  Creator/CMake components). Qt Serial Port in particular is an opt-in
  component in the Qt online installer's "Additional Libraries" section —
  check it's installed if CMake can't find `Qt6SerialPort`.
- CMake ≥ 3.21.
- Network access the first time you configure — CMake
  fetches [QWindowKit](https://github.com/stdware/qwindowkit) 1.5.0
  (Apache-2.0, with its submodules) for the custom title bar. To build
  offline, pass `-DFETCHCONTENT_SOURCE_DIR_QWINDOWKIT=<path>` pointing at an
  existing checkout.
- A C++17 compiler: MSVC 2019+/2022, MinGW, GCC, or Clang.

## Building

### Qt Creator

Open `CMakeLists.txt` as a project, pick a Qt 6.11 kit, and build/run — no
extra configuration needed.

### Releasing a Windows build (MinGW)

This is the actual, verified command-line process for producing a
double-clickable, standalone Windows build to hand to someone else —
tested against a Qt 6.11.1 **MinGW 64-bit** install (`E:\Qt\6.11.1\mingw_64`
plus its bundled `E:\Qt\Tools\mingw1310_64` toolchain in the paths below;
adjust both to wherever your own Qt install lives). If you installed Qt via
the MSVC kit instead, see the next section. (This is also exactly what
[.github/workflows/release.yml](.github/workflows/release.yml) automates —
see [Publishing a release build via GitHub Actions](#publishing-a-release-build-via-github-actions)
below if you'd rather not do this by hand.)

**1. Bump the version number first** (skip this if you're just doing a test
build) — see [Releasing a new version](#releasing-a-new-version) below.

**2. Configure a Release build** in its own directory, separate from
whatever Debug build you already have (they can coexist):

```powershell
$env:PATH = "E:\Qt\6.11.1\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
cmake -S . -B build\Release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="E:\Qt\6.11.1\mingw_64"
cmake --build build\Release
```

**3. Copy the .exe into a clean, empty folder** — `windeployqt` dumps every
DLL/plugin it copies into whatever directory the .exe it's
pointed at lives in, and `build\Release` is full of unrelated CMake/Ninja
build files you don't want to hand someone along with the app:

```powershell
mkdir deploy\UbiBotSerialAssistant
copy build\Release\UbiBotSerialAssistant.exe deploy\UbiBotSerialAssistant\
```

**4. Run `windeployqt` on that copy** — this is what actually gathers every
Qt DLL, platform plugin, style and image format the .exe needs to run on a machine with no Qt
installed at all:

```powershell
E:\Qt\6.11.1\mingw_64\bin\windeployqt.exe deploy\UbiBotSerialAssistant\UbiBotSerialAssistant.exe --release
```

`deploy\UbiBotSerialAssistant\` is now a complete, self-contained
folder — zip it up (or hand the folder over directly) and
`UbiBotSerialAssistant.exe` inside it runs standalone on any Windows machine,
no Qt install, no PATH changes, nothing else needed. (Verified by actually
launching that copy on a machine with Qt removed from `PATH` entirely.)

### Windows (MSVC + Ninja)

Same shape as above, from a "Developer PowerShell/Command Prompt for VS" so
`cl.exe` is on `PATH`:

```bash
cmake -S . -B build\Release -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build\Release
mkdir deploy\UbiBotSerialAssistant
copy build\Release\UbiBotSerialAssistant.exe deploy\UbiBotSerialAssistant\
C:/Qt/6.11.0/msvc2022_64/bin/windeployqt.exe deploy/UbiBotSerialAssistant/UbiBotSerialAssistant.exe --release
```

`windeployqt` only deploys the MSVC runtime when it can find it through a
Visual Studio developer environment (`VCINSTALLDIR`). This shell is one, so it
usually does — but don't rely on it: a package without `msvcp140*.dll` /
`vcruntime140*.dll` fails on any machine that doesn't already have the VC++
redistributable installed. Copy them app-locally (Microsoft supports this)
either way:

```powershell
$crt = Get-ChildItem "$env:ProgramFiles\Microsoft Visual Studio\*\*\VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT" -Directory |
       Sort-Object { $_.Parent.Name } -Descending | Select-Object -First 1
Copy-Item "$($crt.FullName)\*.dll" deploy\UbiBotSerialAssistant\ -Force
```

The CI build does exactly this (see the workflow's "Add MSVC runtime
(app-local)" step) and fails if the DLLs are missing from the folder.

### Windows installer (Inno Setup)

The Release also ships a `-setup.exe` built from the same
`deploy\UbiBotSerialAssistant` folder as the `.zip` — the script is
[packaging/windows/installer.iss](packaging/windows/installer.iss). It installs per
user into `%LOCALAPPDATA%\Programs\UbiBot Serial Assistant` (no UAC prompt),
adds a Start Menu shortcut and an uninstaller entry. Per-user is deliberate:
the install directory has to stay writable for the in-app updater's file swap
to work without elevation.

Install [Inno Setup 6](https://jrsoftware.org/isinfo.php), then from the repo
root, after the deploy folder above is ready:

```powershell
& "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" `
  "/DAppVersion=0.9.1.2" `
  "/DSourceDir=$PWD\deploy\UbiBotSerialAssistant" `
  "/DOutputDir=$PWD" `
  "/DOutputBaseFilename=UbiBotSerialAssistant-0.9.1.2-windows-x64-setup" `
  "packaging\windows\installer.iss"
```

All four defines are optional for a local sanity check: with none passed, the
script uses `0.0.0.0`, the repo's `deploy\UbiBotSerialAssistant`, the current
directory, and the version-free output name. CI passes all four (see the
workflow's "Build installer (Inno Setup)" step) and uploads both the versioned
and the fixed `UbiBotSerialAssistant-windows-x64-setup.exe` name, matching the
zip's stable-link convention.

Things worth knowing about the script:

- `AppId` is a fixed GUID — never change it, or upgrades become a second
  install and uninstall entries multiply.
- The app holds a named mutex (`src/main.cpp`) that the script's `AppMutex`
  points at, so Setup asks the user to close a running instance instead of
  failing on locked files.
- After an in-app self-update, `DisplayVersion` in "Apps & features" still
  shows the version from the last installer run; the uninstaller itself keeps
  working because the updater preserves Inno's `unins*` files (see
  `SelfUpdateInstaller`).
- `InitializeSetup()` refuses to install when `%SystemRoot%\System32\icu.dll`
  is missing (Windows older than 10 1703, or a stripped image): Qt's
  `icuuc.dll` is a pure forwarder to that system DLL, so the app couldn't load
  at all and the user would only see a bare "icu.dll was not found". The
  message is bilingual because the wizard itself is English-only.
- The wizard is English-only (Inno doesn't ship a Chinese translation; adding
  one would mean vendoring a `ChineseSimplified.isl`).

### macOS / Linux

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64
cmake --build build
```

## Releasing a new version

There is exactly one place to edit: the `VERSION` in `CMakeLists.txt`'s
`project(...)` call, right at the top of the file:

```cmake
project(UbiBotSerialAssistant
    VERSION 0.8.1.1
    ...
)
```

Bump it (e.g. to `0.8.1.2`), save, and rebuild — that single number then
flows automatically to everywhere the app shows its own version:

- the status bar (`v0.8.1.1`)
- the "Version" row in Settings & About
- the built `.exe`'s own Windows file-version resource (right-click the
  .exe → Properties → Details → "File version"/"Product version")

Nothing else needs editing; there's no second copy of the version number
hardcoded anywhere else to keep in sync.

### Publishing a release build via GitHub Actions

[.github/workflows/release.yml](.github/workflows/release.yml) builds
**Windows, macOS, and Linux** packages in parallel and publishes all three
together as one GitHub Release:

1. Bump `VERSION` as above, commit it.
2. Tag that commit `vX.Y.Z.W` — the tag's version (after the `v`) **must
   match `CMakeLists.txt`'s `project(VERSION ...)` exactly**, or the
   workflow's `prepare` job fails fast before any of the (expensive)
   platform builds even start (see its "Verify version matches
   CMakeLists.txt" step).
3. `git push --tags`.

Each platform builds on its own runner and packages the result the way
that platform's own tooling expects, then a final `publish` job collects all
of them onto one Release once every build has finished:

| Platform | Runner | Qt install | Packaging tool | Output |
|---|---|---|---|---|
| Windows | `windows-latest` (MSVC 2022) | Qt 6.10.3 | `windeployqt` + [Inno Setup](#windows-installer-inno-setup) | `.zip` (a folder of the .exe + its Qt DLLs, per [Releasing a Windows build](#releasing-a-windows-build-mingw) above) **and** a `-setup.exe` installer |
| macOS | `macos-latest` | Qt 6.10.3 | `macdeployqt -dmg` | `.dmg` |
| Linux | `ubuntu-22.04` | Qt 6.10.3 | [linuxdeployqt](https://github.com/probonopd/linuxdeployqt) | `.AppImage` (self-contained, no install needed — `chmod +x`, run) |

(Qt 6.10.3, not the 6.11.1 the manual Windows process above was verified
against, on all three platforms for the same reason: see the workflow's own
`QT_VERSION` comment — aqtinstall, which `jurplel/install-qt-action` uses to
fetch Qt, doesn't yet support Qt's restructured download layout for the
whole 6.11.x line, on any platform.)

Each platform job attaches two identical copies of its package to the
Release — one with the version baked into the filename (kept forever, one
per release) and one under a fixed, version-free name that gets overwritten
by every new release (this is what the stable download links in README.md
rely on): `UbiBotSerialAssistant-X.Y.Z.W-<platform>.<ext>` and
`UbiBotSerialAssistant-<platform>.<ext>`.

To test all three builds without publishing anything public, run the
workflow manually from the Actions tab (`workflow_dispatch`, supplying a
`version` input) — every platform still builds and uploads its package as a
plain workflow artifact, but the final `publish` job is skipped entirely
(gated on the run actually being a tag push), so nothing goes out.

**Known caveats**, none of which block a release, but worth knowing about:

- **Neither the Windows .exe nor the macOS .app/.dmg is code-signed** (no
  certificate/Apple Developer account wired into this workflow) — Windows
  SmartScreen and macOS Gatekeeper will both warn first-run users that the
  app is from an "unknown publisher"/"unidentified developer". Users click
  through ("More info → Run anyway" / right-click → Open, or
  `xattr -cr UbiBotSerialAssistant.app` on macOS) same as any other unsigned
  open-source binary.
- **The Linux AppImage needs FUSE to run directly** (most desktop distros
  ship it; some newer/minimal ones don't) — if double-clicking or running it
  does nothing, `./UbiBotSerialAssistant-*.AppImage --appimage-extract-and-run`
  works everywhere regardless of FUSE.
- **The macOS and Linux packaging steps are less battle-tested than the
  Windows one** — the Windows workflow has actually been run successfully;
  macdeployqt/linuxdeployqt's exact CLI behavior here was verified against
  their documentation and (for linuxdeployqt's download URL/module
  availability) live, but not against an actual run of this workflow yet.
  Worth a `workflow_dispatch` test run before trusting them for a real
  release.

## Project layout

```
CMakeLists.txt
resources/
  devices.json          # bundled device/command catalog (zh/en labels)
  icons/*.svg, *.png    # app/window icons
  linux/*.desktop       # Linux packaging metadata (CI-only)
  app_version.rc.in     # Windows file-version resource template
  resources.qrc
translations/
  ubibot_zh_CN.ts       # Simplified Chinese catalog (hand-authored; see below)
  ubibot_zh_TW.ts       # Traditional Chinese
  ubibot_ja.ts          # Japanese
  ubibot_ko.ts          # Korean
  ubibot_ru.ts          # Russian
  ubibot_fr.ts          # French
  ubibot_it.ts          # Italian
third_party/
  qlight/               # vendored Fluent-style Qt Widgets library (MIT):
                        # controls, dialogs, frameless window (+ QWindowKit)
src/
  main.cpp
  core/                 # no Qt GUI dependency at all -- I/O, data, settings, i18n, updates
    ansi_text.*                     # ANSI escape handling for received text
    batch_command.h                 # saved multi-step command value type
    crc16.*                         # CRC16/MODBUS helper
    device_library.*                # devices.json loading/model/command types
    device_library_update_client.*  # device-library remote update (see its header)
    driver_check.*                  # CH340 driver detection + download links
    env_config.*                    # runtime ".env" reader (update-server URLs)
    hex_text.*                      # strict HEX <-> text conversion/validation
    language_manager.*
    log_entry.h
    log_manager.*
    self_update_installer.*         # app self-update local install/swap logic
    serial_manager.*
    settings_store.*
    software_update_client.*        # app self-update networking (see its header)
  models/                # QAbstractListModel adapters the UI reads
    log_list_model.*         # wraps LogManager for the data monitor
    command_list_model.*     # filtered/searched merged command list
    command_history_model.*  # manual-send history
    batch_command_model.*    # saved "Batch commands" sequences
    port_list_model.*        # detected serial ports
  app/
    app_controller.*     # the facade every UI class talks to
    serial_options.*     # translated dropdown option lists
  ui/                    # Qt Widgets UI (QLight, Fluent-style design)
    main_window.*        # top-level window (QLight::FramelessWindow)
    main_view.*          # menus, command bar, driver bar, panels, send box, status bar
    app_dialog.*         # shared QLight dialog base (card, shadow, buttons)
    serial_settings_panel.*, command_library_panel.*, command_params_panel.*
    data_monitor_view.*  # coloured TX/RX/SYS/ERR log view
    batch_dialogs.*, misc_dialogs.*, settings_dialog.*, template_dialog.*
    ui_common.*          # theme glue + small widgets (cards, segmented control, status pill)
server/                  # mock device-library update server (Go, dev/demo only)
  main.go
  data/devices.json, meta.json
docs/screenshots/        # screenshots used by README.md
.github/workflows/release.yml
```

The UI is Qt Widgets on top of the vendored QLight library; the earlier QML
presentation layer is gone. `core/`, `models/` and `app/` stayed (with new
helpers and models added), and all user-visible UI now lives in `ui/` (see
[Architecture](#architecture) below).

## Architecture

**All logic lives in `core/` and `app/`; the widgets only render state and
forward user actions.** Concretely:

- `core/*`: `SerialManager` wraps `QSerialPort`, `LogManager` owns the
  scrollback + file export, `DeviceLibrary` loads `devices.json`,
  `SettingsStore` wraps `QSettings`, `LanguageManager` owns the active
  `QTranslator`; `AnsiText`, `HexText` and `Crc16` are small pure helpers for
  received-text cleanup, strict HEX parsing/conversion and the MODBUS
  checksum. `DeviceLibraryUpdateClient`/`SoftwareUpdateClient`/
  `SelfUpdateInstaller` back the two remote-update features — their class
  headers document the endpoints, verification and install behaviour.
  `DriverCheck` handles CH340 driver detection and download links. None of
  these classes know about the UI.
- `AppController` (`src/app/app_controller.*`) owns one instance of each
  `core/` class and is the *only* thing the UI talks to for actions: opening
  and closing the port, sending data, resolving an AT/JSON command's
  parameters, saving a log, checking for either kind of update. State is
  exposed as getters plus change signals.
- `models/*` are thin `QAbstractListModel` adapters (`LogListModel`,
  `CommandListModel`, `CommandHistoryModel`, `BatchCommandModel`,
  `PortListModel`); the command list and the send history are `QListView`s
  on them directly.
- `ui/*` is purely presentational: layout, painting, and wiring user
  gestures to `AppController` calls. Controls come from QLight
  (`third_party/qlight`, MIT, vendored and built as a static library);
  `ui_common.*` adds the app's colours on top of QLight's palette (UbiBot
  blue `#0068B7` as the accent) and the few widgets the design needs that
  QLight doesn't ship. Every UI class implements `retranslateUi()` and calls
  it on `QEvent::LanguageChange`, so switching language updates the open
  window live.

## Internationalization

Every UI string is written in English and wrapped in `tr(...)`. Every
other language is a plain Qt translation catalog under `translations/`
(`ubibot_zh_CN.ts`, `ubibot_zh_TW.ts`, `ubibot_ja.ts`, `ubibot_ko.ts`,
`ubibot_ru.ts`, `ubibot_fr.ts`, `ubibot_it.ts`). Only one language is ever
active at a time — no mixed/bilingual display — chosen from a dropdown in
Settings & About that's populated at runtime from
`AppController.availableLanguages()` (backed by
`LanguageManager::availableLanguages()` in `core/language_manager.cpp`); a
dropdown rather than radio buttons because this list is meant to grow past
a dozen entries. Each entry's display text is that language's own native
name (e.g. "简体中文", "English"), so the list itself never needs
retranslating.

Switching languages at runtime (Settings & About dialog) calls
`LanguageManager::setLanguage()`, which installs/removes a `QTranslator` on
the application; Qt then sends `QEvent::LanguageChange` to every widget,
and each UI class re-applies its texts in `retranslateUi()`.

**Adding a language:** add one more `TS_FILES` entry to the
`qt_add_translations()` call in `CMakeLists.txt`, add a matching
`{code, nativeName}` to `kLanguages` in `core/language_manager.cpp`, and
fill in the `.ts` once `lupdate` has generated its `<message>` entries. The
Settings & About dropdown picks it up automatically — nothing else changes.

**Editing translations:** after adding/changing `tr()` calls, regenerate
the `.ts` files' `<message>` entries with the `UbiBotSerialAssistant_lupdate`
CMake target (set up by `qt_add_translations()`), fill in `<translation>`
for the new strings, then rebuild so `lrelease` regenerates the embedded
`.qm`. Message contexts are the C++ class names (e.g.
`ui::SerialSettingsPanel`).

**Device command text** (model descriptions, command names, parameter
labels) lives in `resources/devices.json` as `{ "zh": "…", "en": "…" }`
pairs rather than going through `tr()`, since it's data, not source code —
see `LocalizedText` in `core/device_library.h`. `LanguageManager::pick()`
resolves it to whichever single language is currently active. Same idea
applies to the device-library remote-update payload — see
`DeviceLibraryUpdateClient` in `src/core/device_library_update_client.h`.

## Design notes

A few implementation choices worth knowing about before you dig in:

- **Window chrome.** On all three platforms the main window is a
  `QLight::FramelessWindow`: a title bar drawn by the app (icon, title,
  light/dark theme) on top of [QWindowKit](https://github.com/stdware/qwindowkit),
  which does the native window work. On Windows that means Aero Snap,
  resize borders, the system menu, Windows 11 snap layouts and the system
  shadow, with the app drawing the minimize/maximize/close buttons. On macOS
  the native close/minimize/zoom buttons stay (QWindowKit's
  `no-system-buttons` is left off) at the left edge of the title bar, which
  leaves room for them. On Linux QWindowKit uses its Qt-based window
  context (system move/resize via Qt), with the same drawn buttons as
  Windows; snapping behaviour depends on the window manager.
- **Dialogs** are QLight content dialogs: a rounded card centred over the
  main window, which is dimmed behind it, with the command buttons in a
  shaded strip at the bottom.
- **Remote support is not part of the Widgets UI.** The QML version had a
  placeholder Remote Support panel (session code/OTP with no transport
  behind it) that was not reachable from the running app; it was not
  carried over.
- **Port chip detection** in the port picker is a heuristic: ports whose USB
  vendor ID matches the WCH CH340 (`0x1A86`) or Silicon Labs CP210x
  (`0x10C4`) bridge chips are labelled with that chip, because those are what
  UbiBot's own USB-serial adapters use. The label does not guarantee that a
  given port is actually a UbiBot device. A missing CH340 driver on Windows
  or macOS is detected separately and shown as a hint bar.
- **Two independent remote-update mechanisms, not one.** "Check for
  updates" in Settings & About's "Command library" section updates the
  *device command data* (`resources/devices.json`'s contents) from a
  configured backend — see `DeviceLibraryUpdateClient` in
  `src/core/device_library_update_client.h`. "Check for software update" in
  the `&Help` menu updates *the app itself* and is currently commented out
  (its backend registration hasn't been deployed yet) — see
  `SoftwareUpdateClient` in `src/core/software_update_client.h`. Don't
  confuse the two; they're deliberately named/coded differently to avoid
  exactly that.

## License

This project is licensed under the **GNU Lesser General Public License v3.0
(LGPLv3)** — see [LICENSE](LICENSE). Because LGPLv3 incorporates GPLv3 by
reference, the GPLv3 text it points to is also included verbatim in
[COPYING](COPYING) (with [COPYING.LESSER](COPYING.LESSER) holding the same
LGPLv3 terms as `LICENSE`, per the FSF's own convention for projects that
use this license pair).

### Qt 6 module licensing (commercial-use check)

This project links against Qt 6 (`Widgets`, `Svg`, `SerialPort`, `Network` at
runtime; `LinguistTools` is a build-time-only dependency — `lupdate`/
`lrelease` are never linked into or shipped with the built app). Qt is
dual-licensed; a handful of *add-on* modules (e.g. Qt Quick 3D, Qt Quick
Timeline, Qt Virtual Keyboard, Qt Wayland Compositor, Qt MQTT, Qt CoAP, Qt
HTTP Server, Qt Network Authorization, Qt Lottie Animation, Qt Graphs, Qt
GRPC, Qt Canvas Painter, and the Qt Qml ahead-of-time type compiler) are
**GPLv3-only** for open-source users — combining those into a project under
any license other than GPL would not be permitted. None of those are used
here. Checked against Qt's own module documentation (Qt 6.11):

| Module | Open-source license(s) | GPL-only? |
|---|---|---|
| Qt Widgets (`Widgets`) | LGPLv3 or GPLv2 | No |
| Qt SVG (`Svg`) | LGPLv3 or GPLv2 | No |
| Qt Network (`Network`) | LGPLv3 or GPLv2 | No |
| Qt Serial Port (`SerialPort`) | LGPLv3 or GPLv2 | No |
| Qt Linguist Tools (`LinguistTools`) | build-time tool only, not linked/shipped | N/A |

Every runtime Qt module this app uses is available under LGPLv3, the same
license this project itself is released under, so the combination is
license-compatible — **including commercial use**: a business may use,
modify, resell, or embed this software (or ship it internally) without
needing a Qt commercial license, provided the LGPLv3 obligations toward Qt
are met. This project already satisfies them the standard, low-friction
way:

- **Dynamic linking, not static.** The release process
  ([Releasing a Windows build](#releasing-a-windows-build-mingw) above) runs
  `windeployqt`, which drops Qt's own `.dll`s next to the `.exe` rather than
  statically linking them in. End users can replace any of those DLLs with
  a compatible build of Qt, which is what LGPLv3 §4(d) ("Combined Works")
  requires in exchange for not having to LGPL/GPL the whole application.
- **Unmodified Qt.** This project makes no source changes to Qt itself, so
  the LGPLv3-licensed Qt source anyone would need is simply upstream Qt —
  freely available from <https://www.qt.io/download-open-source> or
  <https://code.qt.io> — no separate written offer is needed.
- **This section** documents the Qt version floor (6.7+, validated against
  6.11) and license (LGPLv3) in use, satisfying LGPLv3's "give prominent
  notice" expectation.

If this project (or a fork of it) is ever changed to **statically** link
Qt, or to ship a build where end users cannot replace the Qt libraries,
these LGPLv3 terms would no longer be satisfied automatically and either a
switch back to dynamic linking or a Qt commercial license would be needed
instead.

Two third-party libraries are compiled *into* the app (statically): QLight
Widgets (MIT, see `third_party/qlight/LICENSE`, including a subset of
Microsoft's Fluent UI System Icons, MIT) and QWindowKit (Apache-2.0). Both
licenses are compatible with LGPLv3; their notices are in
`third_party/qlight/THIRD_PARTY_NOTICES.md` and should ship with binary
releases. QWindowKit uses Qt's private headers, so a build is tied to the
Qt 6 minor version it was compiled against — replacement Qt libraries must
be the same minor version.

This is an engineering-level license/compatibility check, not legal advice
— for a business-critical determination, run it past a licensed attorney.
