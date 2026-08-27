# UbiBot Serial Assistant

A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS,
SP1, …), built with Qt 6.11 / QML / C++17 / CMake.

**→ [Features & User Interface](docs/features.md)** — the full tour: what
the app looks like, everything it does, and why you'd use it.
**→ [Architecture](docs/architecture.md)** — how it's built: project
layout, the C++/QML split, internationalization, and design notes.

This project was scaffolded from a design mockup; see
[Architecture's Design notes](docs/architecture.md#design-notes) for what's
real vs. placeholder.

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
by every new release (this is what the stable download links below rely
on): `UbiBotSerialAssistant-X.Y.Z.W-<platform>.<ext>` and
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

#### Stable "latest" download links

GitHub redirects `.../releases/latest/download/<filename>` to the asset
with that exact filename on whichever release is currently marked
"Latest" — so as long as every release keeps attaching an asset under the
same fixed name per platform (per the workflow above, with `make_latest:
true` making sure each new tagged release actually becomes "Latest"), these
URLs always resolve to the newest build without needing to know its version
number:

```
https://github.com/ubibot-open/ubibot-serial-sync/releases/latest/download/UbiBotSerialAssistant-windows-x64.zip
https://github.com/ubibot-open/ubibot-serial-sync/releases/latest/download/UbiBotSerialAssistant-macos.dmg
https://github.com/ubibot-open/ubibot-serial-sync/releases/latest/download/UbiBotSerialAssistant-linux-x86_64.AppImage
```

(The release notes page itself is the same idea, one level up:
`https://github.com/ubibot-open/ubibot-serial-sync/releases/latest`.)

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
