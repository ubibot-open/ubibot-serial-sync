# App Self-Update · Design Notes

Status: **implemented**. This document records the design tradeoffs and known
limitations, for future maintenance/troubleshooting reference — it is a
separate feature from
[device-library-update-protocol.md](device-library-update-protocol.md) (the
remote update mechanism for the device command library): this update targets
**the App itself** (`UbiBotSerialAssistant.exe`), not device command data.

## 1. Overview

The menu item `Help (&Help) → Check for software update` triggers
`AppController.checkForAppUpdate()`, which pops up `SoftwareUpdateDialog.qml`.
When a new version is available and the user clicks "Update now", it runs the
full download → verify → extract → quit App → overwrite the install directory
via an internally-generated batch script → relaunch App flow — the user never
has to open a browser and download anything manually.

Two new classes with a clean split of responsibilities:
- [SoftwareUpdateClient](../src/core/software_update_client.h) — handles
  networking only: checking the version, downloading the zip.
- [SelfUpdateInstaller](../src/core/self_update_installer.h) — handles the
  local filesystem only: extraction, verification, generating and launching
  the batch script responsible for "overwriting the install directory". Pure
  static functions, no dependency on Qt's signal/slot mechanism, because every
  step is a synchronous local operation.

`AppController` wires the two together, exposing `appUpdateState`/
`appUpdateMessage`/`remoteAppVersion`/`appUpdateAvailable`/`appUpdateForced`/
`appUpdateProgress` as properties for `SoftwareUpdateDialog.qml` to bind to —
structurally identical to the `libraryUpdateState` family of properties used
by the device command library update (deliberately using different property
names/menu text, to avoid confusion with "check for command library update").

## 2. Reusing the existing Software/Version system instead of writing a new protocol

The `ubibot-appcenter` backend already has a generic "software/version"
distribution system (`softwares`/`versions` tables,
`App\Http\Controllers\Admin\SoftwareController::api_list()`, public route
`GET /api/software/list`), originally built for other UbiBot products —
`admin-react` already has a ready-made upload UI for it too (version number,
release notes, platform, and a force-update toggle can all be filled in;
uploading a zip/exe gets you a download link directly). This update reuses it
directly, rather than standing up a separate protocol the way the device
command library did. Reasons:

- The backend admin UI already exists — ops just goes to the "Software
  Version Management" page in `admin-react` to upload a file and fill in the
  version number; there's no need to build a separate upload/management UI
  just for this feature.
- The only one-time integration work needed is the migration added under
  `ubibot-appcenter` (a separate repository, not part of this one's directory
  tree) at `server/database/migrations/`:
  `2026_08_25_010000_register_ubibot_serial_assistant_software.php`, which
  registers this App as the row with `slug = "ubibot-serial-assistant"` in
  the `softwares` table — **this migration has to be run manually via
  `php artisan migrate`** (nothing runs it automatically, because it's a
  migration against a shared remote database, which a script shouldn't be
  doing on its own).

**The response is not a bare array**: `SoftwareController` uses the
`larke-admin` framework's built-in `ResponseJson` trait
(`$this->success('success', $software)`), so the actual response is wrapped
in that framework's standard envelope format
`{"success": bool, "code": int, "message": string, "data": [...]}` — the
product list itself lives in the `data` field, not as a top-level array. This
was confirmed by actually `curl`-ing the endpoint (the initial implementation
assumed a bare array based on the exploration report's description of "returns
an array"; it compiled fine, and it wasn't until testing against a locally
running Laravel instance that the parsing bug was caught and fixed).

Reuse also brought along two known limitations, both handled defensively on
the client side:

1. **`/software/list`'s `product`/`serial` query parameters are validated but
   not actually used to filter** — the endpoint returns *every* registered
   software product (each with its latest version filtered by `os`), not just
   ours. `SoftwareUpdateClient::checkForUpdate()` finds its own entry in the
   returned array by matching `slug == "ubibot-serial-assistant"`; if it can't
   find one, it reports "this App is not registered on the update server" —
   it will never mistake another product's version info for its own.
2. **The `versions.sha256` field exists, but nothing anywhere in the pipeline
   (`SoftwareVersionController`, the `admin-react` upload form) ever writes to
   it** — in practice it's currently always empty. `SoftwareUpdateClient`/
   `SelfUpdateInstaller` still implement the verification logic properly
   (verify only when `sha256` is non-empty; abort immediately on a mismatch —
   no extraction, no overwrite), on a strict "verify if present, skip if
   absent" basis — whenever the backend actually starts populating this
   field, the client will pick it up automatically with no code changes
   needed.

## 3. Version comparison / forced update / minimum version

Same approach as the `minAppVersion` handling in the device command library
update:

- `version.version` (server-side) and `AppController.appVersion` (local,
  sourced from `PROJECT_VERSION` in `CMakeLists.txt`) are both compared as
  `QVersionNumber`, not as a plain string inequality — this avoids issues
  like "1.10.0" being incorrectly judged older than "1.9.0" under
  lexicographic ordering.
- `version.min_required_version`: if the server's new version requires that
  the App can only be upgraded directly from some minimum version onward, and
  the local version is older than that, "Update now" is not offered — instead
  the user is told to upgrade to some intermediate version first (this mirrors
  the `minAppVersion` threshold check in the device command library update,
  but with the semantics flipped — `minAppVersion` is "the minimum App
  version this data requires", while `min_required_version` is "the minimum
  starting version that can jump directly to this new version"; the two field
  names are easy to confuse with each other, so this has been spelled out in
  the comments).
- `version.is_force_update`: when true, `SoftwareUpdateDialog.qml` hides the
  "Later" button and disables closing via click-outside/Esc
  (`closePolicy: Popup.NoAutoClose`), forcing the user through the update
  flow. **The current version does not check for updates automatically on App
  launch** — a check only fires when the user manually opens the Help menu,
  so "forced update" currently only takes effect when the user actively
  checks; it won't silently block a user who's already in the middle of using
  the App. Adding an automatic check-on-launch is an obvious follow-up option,
  not implemented yet.

## 4. Package format: zip of the whole directory + `tar` to extract, no new toolchain introduced

Current state (true when this feature was built; see
[.github/workflows/release.yml](../.github/workflows/release.yml) for
whether CI packaging has since replaced the manual steps): client releases
were a fully manual process (see [BUILD.md](../BUILD.md)) — `windeployqt`
generates a `deploy\UbiBotSerialAssistant\` folder (exe + Qt shared
libraries + qml/translation resources, ~110MB), and ops compresses it into a
zip for distribution by hand. The repo has no installer-building toolchain
at all (no Inno Setup, no NSIS) — CI (added afterward, see the workflow
linked above) automates *producing* that same zip, but still doesn't turn it
into an installer. This update deliberately did **not** add an installer
toolchain just for this feature — instead it builds self-update directly on
top of the existing "zip the whole directory" approach:

- What gets downloaded is that same zip; `SelfUpdateInstaller::extractAndValidate()`
  extracts it to a temp directory using the `tar.exe` built into Windows 10
  1803+/11 (Microsoft has bundled a zip-capable bsdtar since that release),
  with no need for any third-party unzip library, and no need to pull in Qt's
  zip support module just to satisfy a need as small as "unzip one zip file".
- After extraction, a sanity check runs first: `UbiBotSerialAssistant.exe`
  must be found either at the top level of the temp directory or exactly one
  level down (to accommodate both packaging habits — "zipped the whole
  `deploy` folder" and "zipped the contents of the folder") — if it's not
  found, the process errors out immediately without proceeding, to guard
  against the server returning something that isn't actually a real release
  package (e.g. an HTML error page saved as a `.zip`).
- Only after verification passes does it: quit the App itself, launch an
  auto-generated `.bat` script, and the script takes over — waiting for the
  App to actually exit, `robocopy /MIR`-mirroring the temp directory over the
  install directory, relaunching the App, cleaning up temp files, and deleting
  itself.

**Why a batch script instead of a real installer**: a running App can't
overwrite/delete its own exe/dlls while they're in use (Windows file locking).
The standard approaches are either "download an installer, quit, run the
installer silently" (more robust, but requires adding a whole installer
toolchain and the associated changes to the release process), or "download a
zip, quit, use a separate small process to do the file swap" (no new
toolchain needed, but the mechanism itself is hackier, with weaker fallback
options when something goes wrong). This update went with the latter — see
the known limitations in the next section.

## 5. `.env` protection: `robocopy /XF ".env"`

If the install directory has a `.env` file in it (the one used by the device
command library update, see
[device-library-update-protocol.md](device-library-update-protocol.md)),
`robocopy /MIR` would by default treat it as "an extra file present in the
destination but not the source" and delete it outright — since the downloaded
release package naturally never contains this file. The script generated by
`SelfUpdateInstaller::writeUpdateScript()` explicitly adds `/XF ".env"` to
exclude specifically this one file, leaving the rest of the mirroring
behavior unchanged (including deleting old DLLs/files from the previous
version that no longer exist in the new one).

## 6. Known limitations (deliberately out of scope for this implementation, kept here for future reference)

1. **Assumes the install directory itself is writable; doesn't handle install
   locations that require administrator privileges** — the current
   deployment approach is "copy this folder anywhere you like", which
   normally isn't somewhere requiring elevation like `Program Files`. To
   properly support installing in such a location, the `robocopy` step would
   silently fail due to insufficient permissions (the batch script has no way
   to report failure back to the App, which has already exited by that
   point), and UAC elevation would need to be added.
2. **No failure rollback** — if `extractAndValidate()` fails, it errors out
   and aborts the flow while the App is still running and the install
   directory hasn't been touched at all, which is safe. But once the batch
   script actually starts running `robocopy /MIR`, if that step gets
   interrupted partway through (e.g. a sudden power loss), the install
   directory can be left in a half-old-half-new state, with no automatic
   detection/repair mechanism — the only recovery is for the user to run
   "Check for software update" again to overwrite it back to a complete
   version.
3. **The batch window flashes briefly on screen** — `cmd.exe /C script.bat`
   opens a console window that stays visible until the script deletes itself
   and exits; this is purely a cosmetic wart and doesn't affect functionality.
4. **No automatic check on App launch** — see the end of section 3, purely
   out of scope for now, not a technical limitation.
5. **`sha256` verification is currently a no-op in practice** — see section 2;
   nothing populates the field yet, so whether the client-side code actually
   behaves as intended will need to be verified once the backend starts
   filling it in.
