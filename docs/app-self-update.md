# App self-update (GitHub Releases)

How UbiBot Serial Assistant updates itself. There is **no separate update
server**: the GitHub Release that `.github/workflows/release.yml` publishes is
the source of truth for "what version exists", "what changed", and "what to
download".

## Where the feature lives

| Piece | File | Job |
| --- | --- | --- |
| Check + download | `src/core/software_update_client.{h,cpp}` | GitHub API, asset selection, progress |
| State machine | `src/app/app_controller.cpp` | `appUpdateState_` / `appUpdateMessage_` / `appUpdateProgress_` |
| Dialog | `src/ui/settings_dialog.cpp` (`SoftwareUpdateDialog`) | what the user sees |
| Swap + relaunch | `src/core/self_update_installer.{h,cpp}` | extract, validate, replace, restart (Windows only) |

Entry points:

- **Help → Check for software update** (manual, always available).
- **Automatic check at startup**, at most once a day
  (`AppController::maybeCheckForAppUpdate()`, timestamp in
  `SettingsStore::lastAppUpdateCheckAt()`). It never opens UI by itself;
  `MainView` opens the dialog only if the result is `updateAvailable`.

## Source: the GitHub Release

`release.yml` publishes one Release per `vX.Y.Z.W` tag containing one package
per platform, each uploaded twice:

```
UbiBotSerialAssistant-<version>-windows-x64.zip   versioned copy
UbiBotSerialAssistant-windows-x64.zip             fixed name (stable link)
UbiBotSerialAssistant-<version>-macos.dmg
UbiBotSerialAssistant-macos.dmg
UbiBotSerialAssistant-<version>-linux-x86_64.AppImage
UbiBotSerialAssistant-linux-x86_64.AppImage
```

This naming is a contract with the client — see `platformAssetSuffix()`.

A check is one request:

```
GET https://api.github.com/repos/{owner}/{repo}/releases/latest
Accept: application/vnd.github+json
X-GitHub-Api-Version: 2022-11-28
User-Agent: UbiBotSerialAssistant/<version>
```

`/releases/latest` returns the newest **published, non-prerelease, non-draft**
release, so pre-releases can be shipped for testing without every client
offering them. From the response:

- **version** — `tag_name` minus the leading `v`, compared with
  `QVersionNumber` (four-component versions like `0.8.2.1` work; `0.8.2.1 >
  0.8.2`).
- **changelog** — the release body, trimmed to 2000 characters.
- **package** — the first asset whose name ends in the platform suffix,
  preferring the versioned name, falling back to the fixed one.
- **sha256** — the asset's `digest` field (`sha256:...`). GitHub computes this
  at upload time; assets from older releases may not have one, in which case
  the download is applied unverified.

The repository can be overridden with `.env`'s `APP_UPDATE_GITHUB_REPO=owner/name`
(for forks and for testing against a throwaway release). Unset/blank means
`SoftwareUpdateClient::kDefaultRepo`.

## Windows: in-place replacement

1. Download the zip to
   `%TEMP%/UbiBotSerialAssistant-update/update.zip`, with progress in the
   dialog (`Cancel` aborts).
2. Verify the SHA-256 when the release provided a digest. A mismatch ends the
   flow with an error; nothing is extracted.
3. Extract with the `tar.exe` bundled with Windows 10 1803+ (no third-party zip
   library) into a fresh temp directory, and require
   `UbiBotSerialAssistant.exe` either at the archive root or one level down —
   that rejects an HTML error page saved as `.zip` and both plausible zip
   layouts.
4. Write `apply-update.bat` and launch it detached (`cmd.exe /C`). It:
   - waits until this process's PID is gone,
   - `robocopy /MIR`s the extracted files over the installation directory,
     excluding a user-owned `.env`, retrying locked files for ~20s,
   - relaunches the app from the installation directory,
   - deletes the staging directory, the zip, and itself.
5. The app calls `QCoreApplication::quit()` so the script can proceed.

Installation directory must be writable by the user. A build unpacked into
`Program Files` needs elevation that this flow does not request, so the
`robocopy` step fails and the dialog reports the error; the installation is
left untouched.

## macOS and Linux

`SelfUpdateInstaller` is Windows-only, so on those platforms "Update now" opens
the release's `html_url` (the browser's normal download page) instead of
downloading. The dialog text says automatic installation isn't available
there.

## Constraints and known limitations

- **Public repository required.** The API is queried anonymously (60
  requests/hour per IP); no token is embedded, because anything shipped in the
  binary can be extracted. The once-a-day throttle keeps normal use far inside
  that budget, and a manual check is one request.
- **Unsigned builds.** The packages are not code-signed, so the first launch
  after an update still triggers SmartScreen. That is a release-signing
  problem, not an update-logic one.
- **No forced updates.** GitHub Releases have no "force" flag, so every update
  can be dismissed with "Later". If forced updates are ever needed, a tag
  convention or a marker line in the release body would be the place to add
  it.
- **`/releases/latest` is by creation time**, not by version number: publishing
  an older tag later would make it "latest". Always publish in increasing
  version order.
- **Network reachability.** `api.github.com` and the asset host
  (`objects.githubusercontent.com`) are not reachable from every network. A
  failed check surfaces as an error message with "Retry" in the dialog; it
  never changes anything locally.
- **Version consistency.** `release.yml` refuses to publish when the pushed tag
  doesn't match `project(VERSION ...)` in `CMakeLists.txt`, so the version the
  client compares against is always the one that was packaged.

## Trying the flow without publishing a real release

1. Fork, or use any throwaway public repo, and create a release whose tag is
   higher than the running build's version, with an asset named
   `UbiBotSerialAssistant-<tag-without-v>-windows-x64.zip` containing the
   deploy folder's contents (the `.exe` at the zip root).
2. Point `.env` at it:

   ```
   APP_UPDATE_GITHUB_REPO=your-name/your-test-repo
   ```

3. Run the app from a **scratch copy** of the install directory — the update
   really does replace whatever directory the running `.exe` lives in.
4. Help → Check for software update → Update now. The app downloads, swaps the
   scratch directory's contents, and restarts itself.

Remember that a successful check is cached for a day for the automatic
startup check; the manual menu entry always checks.
