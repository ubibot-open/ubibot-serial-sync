# Building & Architecture

Pre-built releases are published for Windows/macOS/Linux — see
[README.md](README.md) for the download links. **You don't need any of this
to use the app.** This document is for building from source and for the
technical details behind how it's put together.

## Requirements

- Qt **6.11** (or any 6.7+ toolchain — that's the floor `qt_add_translations`
  and `QQmlApplicationEngine::loadFromModule` need; 6.11 is what this
  project has been validated against), with the **Quick**,
  **QuickControls2**, and **SerialPort** modules, plus the **Qt Linguist
  tools** (`lupdate`/`lrelease`, installed by default with the Qt
  Creator/CMake components). Qt Serial Port in particular is an opt-in
  component in the Qt online installer's "Additional Libraries" section —
  check it's installed if CMake can't find `Qt6SerialPort`.
- CMake ≥ 3.21.
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
DLL/plugin/QML module it copies into whatever directory the .exe it's
pointed at lives in, and `build\Release` is full of unrelated CMake/Ninja
build files you don't want to hand someone along with the app:

```powershell
mkdir deploy\UbiBotSerialAssistant
copy build\Release\UbiBotSerialAssistant.exe deploy\UbiBotSerialAssistant\
```

**4. Run `windeployqt` on that copy** — this is what actually gathers every
Qt DLL, platform plugin, image format, and QML module (including this
app's own `UbiBot` module) the .exe needs to run on a machine with no Qt
installed at all:

```powershell
E:\Qt\6.11.1\mingw_64\bin\windeployqt.exe deploy\UbiBotSerialAssistant\UbiBotSerialAssistant.exe --qmldir qml --release
```

`deploy\UbiBotSerialAssistant\` is now a complete, self-contained ~110 MB
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
C:/Qt/6.11.0/msvc2022_64/bin/windeployqt.exe deploy/UbiBotSerialAssistant/UbiBotSerialAssistant.exe --qmldir qml --release
```

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
    VERSION 1.1.1.1
    ...
)
```

Bump it (e.g. to `1.1.1.2`), save, and rebuild — that single number then
flows automatically to everywhere the app shows its own version:

- the custom title bar (`v1.1.1.1`)
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
| Windows | `windows-latest` (MSVC 2022) | Qt 6.10.3 | `windeployqt` | `.zip` (a folder of the .exe + its Qt DLLs, per [Releasing a Windows build](#releasing-a-windows-build-mingw) above) |
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
  devices.json          # device/command catalog (bilingual labels, see below)
  icons/*.svg, *.png     # toolbar/app icons
  linux/*.desktop        # Linux packaging metadata (CI-only)
  resources.qrc
translations/
  ubibot_zh_CN.ts        # Simplified Chinese catalog (hand-authored; see below)
  ubibot_zh_TW.ts        # Traditional Chinese
  ubibot_ja.ts           # Japanese
  ubibot_ko.ts           # Korean
  ubibot_ru.ts           # Russian
  ubibot_fr.ts           # French
  ubibot_it.ts           # Italian
qml/
  Main.qml               # window shell: menu bar, toolbar, mode toggle, status bar
  Theme.qml              # shared colors/fonts (pragma Singleton)
  SerialSettingsPanel.qml
  CommandLibraryPanel.qml
  RemoteAssistPanel.qml
  CommandParamsPanel.qml
  DataMonitorView.qml
  SaveLogDialog.qml
  SettingsAboutDialog.qml
  AboutDialog.qml
  SoftwareUpdateDialog.qml
src/
  main.cpp
  core/                  # no Qt GUI dependency at all -- I/O, data, settings, i18n, updates
    language_manager.*
    device_library.*
    device_library_update_client.*   # device-library remote update (see docs/device-library-update-protocol.md)
    software_update_client.*         # app self-update networking (see docs/app-self-update.md)
    self_update_installer.*          # app self-update local install/swap logic
    env_config.*                     # runtime ".env" reader (update-server URLs, not committed)
    log_entry.h
    log_manager.*
    serial_manager.*
    settings_store.*
  models/                # QAbstractListModel adapters QML binds to
    log_list_model.*         # wraps LogManager for the data monitor ListView
    command_list_model.*     # filtered/searched device command list
    command_history_model.*  # manual-send history
    batch_command_model.*    # saved "Batch commands" sequences
    port_list_model.*        # detected serial ports
  app/
    app_controller.*     # the QML_SINGLETON every .qml file talks to
    serial_options.*     # QML_SINGLETON: translated dropdown option lists
```

There is no `src/ui/` anymore — the Qt Widgets version of this UI has been
fully replaced by the QML one above. `core/` is unchanged and has zero Qt
GUI/Quick dependency; `models/` and `app/` are the adapter layer that makes
it consumable from QML (see [Architecture](#architecture) below).

## Architecture

**All logic is C++; QML only renders state and forwards user actions.**
Concretely:

- `core/*` is exactly what it was in the Widgets version: `SerialManager`
  wraps `QSerialPort`, `LogManager` owns the scrollback + file export,
  `DeviceLibrary` loads `devices.json`, `SettingsStore` wraps `QSettings`,
  `LanguageManager` owns the active `QTranslator`. `DeviceLibraryUpdateClient`/
  `SoftwareUpdateClient`/`SelfUpdateInstaller` are the same shape, added
  later for the two remote-update features (see
  [docs/device-library-update-protocol.md](docs/device-library-update-protocol.md)
  and [docs/app-self-update.md](docs/app-self-update.md)). None of these
  classes know QML exists.
- `AppController` (`src/app/app_controller.*`) is a `QML_SINGLETON` that
  owns one instance of each `core/` class and is the *only* thing QML talks
  to for actions: opening/closing the port, sending data, resolving an AT
  command's parameters, saving a log, checking for either kind of update.
  Every `Q_INVOKABLE` on it is a real operation implemented in C++; QML
  never contains business logic like "how do I format a HEX payload" or
  "what's today's default log directory".
- `models/*` are thin `QAbstractListModel` adapters (`LogListModel`,
  `CommandListModel`, `CommandHistoryModel`, `BatchCommandModel`,
  `PortListModel`) so QML `ListView`s/`ComboBox`es can bind directly to
  `LogManager`/`DeviceLibrary`/`SettingsStore`/`SerialManager::availablePorts()`
  data without either side knowing about the other's native types.
- `SerialOptions` is a stateless `QML_SINGLETON` exposing the
  parity/stop-bits/flow-control/baud-rate dropdown contents, so the
  `QSerialPort` enum values it hands back stay a C++-side implementation
  detail QML never has to know.
- The `.qml` files under `qml/` are purely presentational: layout, styling,
  and wiring user gestures to `AppController` calls or local, purely
  cosmetic UI state (e.g. the Remote Support panel's placeholder code
  generator).

C++ types are exposed to QML via the `QML_ELEMENT`/`QML_SINGLETON` macros
declared directly on the classes (`core/language_manager.h` is the
exception — it's never included from QML, only used internally by
`AppController` and `DeviceLibrary`) — there are no manual
`qmlRegisterType()` calls anywhere; `qt_add_qml_module()` in
`CMakeLists.txt` wires the registration up at build time via
`qmltyperegistrar`.

## Internationalization

Every UI string is written in English and wrapped in `tr(...)` (C++) or
`qsTr(...)` (QML) — Qt's translation tooling treats both uniformly. Every
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
`QGuiApplication`. `main.cpp` additionally calls
`QQmlApplicationEngine::retranslate()` on every language change — without
that, `tr()`-bound C++ properties would update live but `qsTr()` literals
baked into QML bindings would not.

**Adding a language:** add one more `TS_FILES` entry to the
`qt_add_translations()` call in `CMakeLists.txt`, add a matching
`{code, nativeName}` to `kLanguages` in `core/language_manager.cpp`, and
fill in the `.ts` once `lupdate` has generated its `<message>` entries. The
Settings & About dropdown picks it up automatically — nothing else changes.

**Editing translations:** after adding/changing `tr()`/`qsTr()` calls,
regenerate the `.ts` file's `<message>` entries with Qt's `lupdate` (the
CMake target this project's `qt_add_translations()` sets up handles this —
check your generator's target list, e.g. `update_translations`; `lupdate`
scans `.qml` files natively, same as `.cpp`), fill in `<translation>` for
the new strings, then rebuild so `lrelease` regenerates the embedded `.qm`.
Note that QML message contexts are the file's base name (e.g.
`SerialSettingsPanel.qml`'s strings appear under context
`SerialSettingsPanel` in the `.ts`).

**Device command text** (model descriptions, command names, parameter
labels) lives in `resources/devices.json` as `{ "zh": "…", "en": "…" }`
pairs rather than going through `tr()`, since it's data, not source code —
see `LocalizedText` in `core/device_library.h`. `LanguageManager::pick()`
resolves it to whichever single language is currently active. Same idea
applies to the device-library remote-update payload — see
[docs/device-library-update-protocol.md](docs/device-library-update-protocol.md).

## Design notes

A few implementation choices worth knowing about before you dig in:

- **Window chrome is a frameless, custom-drawn `ApplicationWindow`**
  (`Main.qml`'s `flags: Qt.Window | Qt.FramelessWindowHint`, plus its own
  `TitleBar` component standing in for the OS-native one) rather than a
  standard one with the platform's own title bar — matches the original
  design mockup, and means the whole window (not just its contents) follows
  the app's light/dark theme instead of the OS always drawing a plain white
  title bar regardless. Losing the native frame also loses the OS's own
  resize cursors/border and drop shadow; the resize-grip `MouseArea`s stand
  in for the former. The shadow is repainted from QML instead of via a
  native per-platform call, so it works the same on Windows/macOS/Linux:
  the window itself is oversized by `shadowMargin` on every side and made
  fully transparent (`color: "transparent"`), `frame` (a plain `Item`, not
  `ApplicationWindow`'s own `header:`/`contentItem` slots — those always sit
  flush against the window's *true* edges with no way to inset them) holds
  the actual visible UI inset from those true edges by that same margin, and
  a `MultiEffect` (see `DialogCard.qml`'s own comment for why MultiEffect
  over a stacked-rectangle approximation) paints a real blurred drop shadow
  into the resulting empty ring — all collapsing to zero while maximized,
  same as a native window's shadow would. `frame`'s own 1px outline border
  stands in for the lost native window border on top of that. The visual
  style of everything *inside* the window is Qt Quick Controls' "Fusion"
  style (set in `main.cpp` via `QQuickStyle::setStyle()`), chosen for a
  native-desktop
  look rather than the touch-oriented default.
- **Remote support is a UI placeholder, and currently unreachable besides.**
  Generating a session code/OTP and setting permissions all work (entirely
  in QML — there's no backing logic to speak of), but there's no
  signaling/relay server or peer-to-peer transport behind "Start session"
  yet — it explains that plainly instead of pretending to connect. On top
  of that, `Main.qml`'s mode toggle currently only offers two segments
  ("Serial", "Device commands") even though the `StackLayout` beneath it
  has a third page for `RemoteAssistPanel` — so the panel isn't reachable
  from the running app at all right now, not just incomplete. Wiring up a
  real transport, and exposing the panel, are both future work that would
  live in `AppController`/`Main.qml` like everything else that's real.
- **"Recommended" port detection** in the port picker is a heuristic:
  it flags ports whose USB vendor ID matches the WCH CH340 (`0x1A86`) or
  Silicon Labs CP210x (`0x10C4`) bridge chips UbiBot's own USB-serial
  adapters use — not a guarantee that a given port is actually a UbiBot
  device.
- **Two independent remote-update mechanisms, not one.** "Check for
  updates" in Settings & About's "Command library" section updates the
  *device command data* (`resources/devices.json`'s contents) from a
  configured backend — see
  [docs/device-library-update-protocol.md](docs/device-library-update-protocol.md).
  "Check for software update" in the `&Help` menu updates *the app itself*
  and is currently commented out (its backend registration hasn't been
  deployed yet) — see [docs/app-self-update.md](docs/app-self-update.md).
  Don't confuse the two; they're deliberately named/coded differently to
  avoid exactly that.

## License

This project is licensed under the **GNU Lesser General Public License v3.0
(LGPLv3)** — see [LICENSE](LICENSE). Because LGPLv3 incorporates GPLv3 by
reference, the GPLv3 text it points to is also included verbatim in
[COPYING](COPYING) (with [COPYING.LESSER](COPYING.LESSER) holding the same
LGPLv3 terms as `LICENSE`, per the FSF's own convention for projects that
use this license pair).

### Qt 6 module licensing (commercial-use check)

This project links against Qt 6 (`Quick`, `QuickControls2`, `SerialPort` at
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
| Qt Quick (`Quick`) | LGPLv3 or GPLv2 | No |
| Qt Quick Controls (`QuickControls2`) | LGPLv3 or GPLv2 | No |
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

This is an engineering-level license/compatibility check, not legal advice
— for a business-critical determination, run it past a licensed attorney.
