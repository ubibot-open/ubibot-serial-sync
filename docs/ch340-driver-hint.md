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

## 3. When the hint actually fires

Not proactively on launch (would nag someone who simply hasn't plugged
anything in yet, the overwhelmingly common case, and wrongly imply their
setup is broken). Instead, `AppController::openPort()` offers it only when
someone actually tries to use serial and hits the exact wall this is about:
"Open port" clicked with **no port name** (nothing to select from) **and**
`portListModel_->rowCount() == 0` (not just "nothing currently selected" --
genuinely no ports exist) **and** the driver file(s) above aren't found.
That combination is a much stronger signal than any one of them alone. When
it doesn't hold, the existing plain `portOpenFailed(tr("Select a serial port
first."))` behavior is unchanged.

`SettingsStore::suppressCh340DriverHint()` (persisted, `DriverHintDialog.qml`'s
own "Don't show this again" checkbox) is checked alongside the above --
scoped to just this dialog, not a general mute switch for other port errors.

## 4. The download link

`AppController.ch340DriverUrl` reads `DriverCheck::downloadUrl()`
(`core/driver_check.cpp`), which returns a hardcoded, per-platform URL --
`https://www.ubibot.io/download/3168` on Windows,
`https://www.ubibot.io/download/3166` on macOS, empty everywhere else. This
was originally a `.env` entry (`CH340_DRIVER_DOWNLOAD_URL`, same mechanism
as `DEVICE_LIBRARY_API_BASE_URL`/`APP_UPDATE_API_BASE_URL`), and was
deliberately moved off of that: `.env` is never committed (see `.gitignore`)
and nothing in `.github/workflows/release.yml` creates one, so a
GitHub-Actions-built release package would silently have shipped with this
value empty -- fine for the *server* URLs, which genuinely need to differ
per deployment and default to "feature disabled" when unset, but wrong here,
where the two real links are unlikely to ever need be different for a
from-source build vs. an official release. Hardcoding them into the binary
means they're simply always present, in a CI-built release exactly the same
as a local dev build. `DriverHintDialog.qml` hides its "Download driver"
button entirely if this is ever empty (e.g. a future platform this hasn't
been given a link for yet) rather than showing a dead one.

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
