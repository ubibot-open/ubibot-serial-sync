# Architecture

How UbiBot Serial Assistant is put together — for "what it does" rather
than "how it's built", see [features.md](features.md).

## Project layout

```
CMakeLists.txt
resources/
  devices.json          # device/command catalog (bilingual labels, see below)
  icons/*.svg, *.png     # toolbar/app icons
  linux/*.desktop        # Linux packaging metadata (CI-only, see README.md)
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
  [device-library-update-protocol.md](device-library-update-protocol.md)
  and [app-self-update.md](app-self-update.md)). None of these classes know
  QML exists.
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
[device-library-update-protocol.md](device-library-update-protocol.md).

## Design notes

A few implementation choices worth knowing about before you dig in:

- **Window chrome is a frameless, custom-drawn `ApplicationWindow`**
  (`Main.qml`'s `flags: Qt.Window | Qt.FramelessWindowHint`, plus its own
  `TitleBar` component standing in for the OS-native one) rather than a
  standard one with the platform's own title bar — matches the original
  design mockup, and means the whole window (not just its contents) follows
  the app's light/dark theme instead of the OS always drawing a plain white
  title bar regardless. Losing the native frame also loses the OS's own
  resize cursors/border and (Windows 11) drop shadow/rounded corners; the
  1px outline + resize-grip `MouseArea`s in `Main.qml` stand in for the
  former, there's no attempt at the latter. The visual style of everything
  *inside* the window is Qt Quick Controls' "Fusion" style (set in
  `main.cpp` via `QQuickStyle::setStyle()`), chosen for a native-desktop
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
  [device-library-update-protocol.md](device-library-update-protocol.md).
  "Check for software update" in the `&Help` menu updates *the app itself*
  and is currently commented out (its backend registration hasn't been
  deployed yet) — see [app-self-update.md](app-self-update.md). Don't
  confuse the two; they're deliberately named/coded differently to avoid
  exactly that.
