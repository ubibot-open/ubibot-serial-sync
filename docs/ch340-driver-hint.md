# CH340 Driver Install Hint · Design Notes

Status: **implemented, Windows + macOS**. Records why this exists and the
detection approach's known limitations, for future maintenance reference.

## 1. The problem

UbiBot devices ship with a CH340 (or CH341, the same chip family) USB-to-
serial bridge -- see `SerialManager::PortHint::Recommended` and
`PortComboBox.qml`'s "recognized chip" labeling. Both Windows and macOS need
a vendor driver installed for that chip to actually get a serial port at
all; without it, the device is completely invisible to `QSerialPortInfo`
(what `SerialManager::availablePorts()` is built on) -- not listed, not
"unavailable", just absent, indistinguishable from no device being plugged
in at all. Some users hit exactly this and don't realize a driver install
is the missing step, since nothing in the app said so.

## 2. Detecting the driver, not the device

`QSerialPortInfo` can't help here (see above -- a driver-less device never
reaches it). What can be checked directly is whether the *driver itself* is
installed, independent of whether any CH340 device happens to be plugged in
right now. `core/driver_check.h`'s `DriverCheck::isCh340DriverInstalled()`
checks for the driver's own installed file(s) directly:

- **Windows**: `CH341SER.SYS` or `CH341S64.SYS` under
  `%SystemRoot%\System32\drivers\` (different WCH driver package versions
  have shipped the same driver under either name, so both are checked).
  `SystemRoot` (an environment variable Windows sets for every process to
  wherever it's actually installed) is used rather than assuming `C:\Windows`
  outright, though that's what it resolves to on the overwhelming majority
  of systems.
- **macOS**: `/Library/Extensions/usbserial.kext`.

This intentionally mirrors the same file-existence check UbiBot's existing
Electron desktop app already uses in production (ported here rather than
re-derived from scratch), rather than the registry-service-key lookup this
started out with -- a real Windows machine without the driver installed
still had the `CH341SER` service key present from a previous install/an
unrelated driver package sharing that name, which the registry approach
couldn't tell apart from a genuinely working driver; checking for the
driver's own file(s) directly is what the already-shipping Electron app
actually relies on, and empirically doesn't have that problem.

Returns `true` unconditionally on Linux (the in-tree `ch341` kernel module
needs no separate install) and any other platform, so the hint (see below)
can never fire there.

## 3. Where the hint shows up

Checked once at app launch (`AppController.ch340DriverInstalled`, a
`CONSTANT` property -- driver install state realistically never changes
mid-session), then surfaced in three places, all ultimately reading that
same value:

1. **`Main.qml`'s own banner**, a thin row docked under the toolbar
   (`Layout.fillWidth`, part of `headerColumn` so it's visible above
   whichever sidebar tab -- Serial or Device commands -- is active), shown
   whenever `ch340DriverInstalled` is false and
   `AppController.ch340DriverHintDismissed` is false. This *used* to be a
   modal dialog popped up reactively from `AppController::openPort()` the
   moment "Open port" was clicked with nothing to select and the driver
   missing -- moved to a persistent, non-interrupting banner instead, per
   user feedback that only surfacing this after the user already hit a
   wall (and, worse, a dialog with no other way back to it once dismissed)
   read as unfriendly. Clicking the banner opens `DriverHintDialog.qml`; a
   "✕" on the banner itself calls `AppController.ch340DriverHintDismissed = true`.
2. **`DriverHintDialog.qml`** -- the explanation dialog, opened only from
   the banner above (not popped up on its own). Its own "Don't show this
   again" checkbox writes/reads the same `ch340DriverHintDismissed`
   property the banner's "✕" does, so whichever one the user reaches for
   first, the other stays in sync. `ch340DriverHintDismissed` is persisted
   via `SettingsStore::ch340DriverHintDismissed()`/`setCh340DriverHintDismissed()`
   so a dismissal survives a restart.
3. **The Help menu's "Driver downloads" item** (`Main.qml`), always present
   regardless of whether the banner has ever shown or been dismissed --
   added specifically so dismissing the banner doesn't lock someone out of
   coming back to this later if it turns out they did need the driver
   after all.

## 4. The driver list + download links

Both `DriverHintDialog.qml`'s "Download driver" button and the Help menu
item above open the same `DriverListDialog.qml` (a `signal
downloadRequested()` on the former, listened for in `Main.qml`, same
pattern `CommandLibraryPanel.qml`'s own `openParams(row)` uses to reach a
sibling panel) -- one dialog, not a URL opened directly from two different
places. It lists every entry from `AppController.availableDrivers()`
(`core/driver_check.h`'s `DriverCheck::knownDrivers()`), each with its
detected install state and a per-row "Download" button. Just one entry
exists today (CH340), but this was deliberately built as a list from the
start -- adding a future driver (CP210x, FTDI, ...) is "one more
`DriverInfo` entry in `knownDrivers()`", not a second dialog or a reworked
single-driver one.

Each driver's URL (`DriverInfo::url`, e.g. CH340's
`DriverCheck::downloadUrl()`) is a hardcoded, per-platform value --
`https://www.ubibot.io/download/3168` on Windows,
`https://www.ubibot.io/download/3166` on macOS, empty everywhere else. This
was originally a `.env` entry (`CH340_DRIVER_DOWNLOAD_URL`, same mechanism
as `DEVICE_LIBRARY_API_BASE_URL`/`APP_UPDATE_API_BASE_URL`), and was
deliberately moved off of that: `.env` is never committed (see `.gitignore`)
and nothing in `.github/workflows/release.yml` creates one, so a
GitHub-Actions-built release package would silently have shipped with this
value empty -- fine for the *server* URLs, which genuinely need to differ
per deployment and default to "feature disabled" when unset, but wrong here,
where the real links are unlikely to ever need be different for a
from-source build vs. an official release. Hardcoding them into the binary
means they're simply always present, in a CI-built release exactly the same
as a local dev build. `DriverListDialog.qml` hides a row's "Download" button
entirely when its URL is empty (e.g. a future platform/driver combination
with no known link yet) rather than showing a dead one.

## 5. Known limitations

1. **Doesn't detect *other* USB-serial chips' missing drivers** (CP210x,
   FTDI, ...) -- scoped to CH340 specifically, since that's the chip
   UbiBot's own hardware ships with. A user on a different USB-serial
   adapter with no driver installed gets the same generic "Select a serial
   port first" message as before, not a tailored hint.
2. **Doesn't verify the driver is actually *working***, only that its file
   is *present* -- a corrupted/disabled driver still reads as "installed"
   here, same tradeoff the Electron app this was ported from already
   accepts.
3. **macOS: checks for the older-style kext specifically, not a newer
   signed system-extension (dext)** -- some CH340 driver distributions have
   moved to the dext mechanism on recent macOS. Ported as-is from the app
   already shipping today rather than re-derived, so behavior matches what
   users of that app already experience; revisit if a dext-only driver
   package is ever what's actually distributed to UbiBot's users.
4. **Windows: no fallback beyond `%SystemRoot%`** -- unlike the Electron
   app this was ported from (which additionally guesses `C:\` if its own
   working-directory-derived drive letter looks wrong, a workaround for
   Node's `process.cwd()` having no reliable relationship to which drive
   Windows itself is installed on), `SystemRoot` is read directly here and
   Windows always sets it correctly regardless of which drive the running
   process itself lives on -- so no equivalent fallback guess was needed to
   match that app's real-world behavior.
