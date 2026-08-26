# Device Command Library · Remote Update Protocol Design

Status: **Design draft, ready to implement against**. This document defines only the client ↔ server interface protocol and the client-side data-fetching logic; it does not cover the server's specific implementation stack (that's up to whichever backend maintains the device list).

## 1. Background and Goals

[resources/devices.json](../resources/devices.json) (corresponding to [DeviceLibrary](../src/core/device_library.h)) is currently a **Qt resource baked into the exe at compile time** — adding a new model or a new command requires editing this file and rebuilding/republishing before users can get it.

The current plan: move the **authoritative version** of "device list + list of commands supported by each device" to be maintained on the server. On startup the client uses the cache from its last download (falling back to the compile-time bundled version if there's no cache), and offers two actions — "check for update" / "download and apply the latest version" — both of which go through the two read-only JSON endpoints described below. **The data schema itself does not change** — the `models[]` the server returns matches exactly what's already described in [device-json-protocol-schema.md](device-json-protocol-schema.md); this only adds a layer for "where to fetch this data from" — once fetched, the parsing code path is identical.

## 2. Client Configuration: `.env`

The backend service address is not committed to the code repository (different environments/internal domains don't need to be public, and there's no need to rebuild every time it changes) — instead it's read at runtime from a `.env` file **not committed to git** (see [.env.example](../.env.example) and [.gitignore](../.gitignore)):

```
DEVICE_LIBRARY_API_BASE_URL=https://appcenter.ubibot.com/api/device-library
DEVICE_LIBRARY_API_KEY=
```

- `DEVICE_LIBRARY_API_BASE_URL`: common prefix for both endpoints (no trailing `/` — the client strips any extra slash automatically). **Empty, or the whole `.env` file missing** = remote updates disabled; the app just uses the compile-time bundled `resources/devices.json`, and the "check for update" button shows "update server not configured" directly, without erroring out. The protocol itself (sections 4 and 5 below) only defines the relative paths `/version`, `/latest` — the `/api/device-library` prefix is purely a config value, not something hardcoded in client code; the real backend (see section 3.1) just happens to register both endpoints under this path, which is why `DEVICE_LIBRARY_API_BASE_URL` looks like this in production.
- `DEVICE_LIBRARY_API_KEY`: optional. When non-empty, the client attaches an `X-Api-Key: <this value>` header on every request, for the backend to use for access control as needed; if the backend doesn't require auth, leave it empty. The current real backend (section 3.1) has no auth, so leave it empty.

The client looks for `.env` in this order (see [EnvConfig](../src/core/env_config.h)):
1. The directory containing the executable (`deploy/UbiBotSerialAssistant/.env`) — production/packaged environment.
2. The current working directory — for running directly from the build directory during development, for pointing at a test backend on the fly.
3. The source root of this checkout (a path baked in at compile time, see `APP_SOURCE_DIR` in
   [CMakeLists.txt](../CMakeLists.txt)) — purely a development convenience, so a `.env` placed at
   the repo root can still be found when running via Qt Creator (whose default working directory is
   the build directory, not the source directory); on a different machine / after a proper packaged
   build, this path simply doesn't exist and the lookup is silently skipped.

If none of the three locations has it, the feature is disabled. **When packaging for release, ops/the release process needs to place the real `.env` into the `deploy/UbiBotSerialAssistant/` directory** (alongside the `windeployqt` output) — this step is not done automatically by this repo's build scripts.

## 3. General Conventions

- Both endpoints are `GET`, returning `Content-Type: application/json`.
- The client appends `app=ubibot-serial-assistant` and `appVersion=<APP_VERSION>` (e.g. `0.2.2.1`) to the query string, for the backend's stats/rollout purposes — the server may ignore them.
- Request timeout is 8 seconds (fixed on the client side); a timeout is treated as a network error and is not retried automatically — whether to retry is up to the user manually clicking "check for update" again.
- Every response body carries a top-level `"ok"` boolean field; the client **treats it as `ok: true` by default** (a missing field is not treated as an error), but the server is encouraged to send it explicitly. When `ok: false`, the client only reads `error.message` to show the user — no other fields are required:

  ```json
  { "ok": false, "error": { "code": "SERVICE_UNAVAILABLE", "message": "..." } }
  ```

- Version numbers are all strings in the lexicographically-comparable form `lib-2026.08.20` (the same convention as the `version` field in the existing `resources/devices.json`); the server guarantees a newer version's string sorts lexicographically greater than an older one. The client only does string equal/not-equal comparison (not-equal = update available) — no semantic version comparison.

### 3.1 Real Backend Implementation

The real backend is `server/` in the `ubibot-appcenter` repository (a Laravel app): `app/Http/Controllers/DeviceLibraryController.php` implements the concrete logic for the two endpoints below, and `routes/api.php` registers the routes; since `routes/api.php` as a whole is already wrapped with an `api` prefix by `RouteServiceProvider`, the final external paths are `/api/device-library/version` and `/api/device-library/latest` (**no auth required** — same treatment as other public read-only endpoints in that project, like `/api/software/list`). Data comes from the two files `resources/device-library/{devices,meta}.json` in that repo — there's currently no accompanying admin CRUD UI; updating the device library means directly editing these two JSON files and running a normal deploy. Once a device-management page exists, the file-reading logic in the Controller can be swapped for reading from a database, without the paths/response formats of these two endpoints needing to change.

The Go mock server under this repo's `server/` directory ([server/README.md](../server/README.md)) has a path layout that fully matches the real backend, differing only in host:port — making it easy to switch back and forth during local integration testing.

## 4. Endpoint 1: Version Check `GET {baseUrl}/version`

Returns only metadata, no full command list — for low-cost "is there an update" polling, without pulling down tens of KB of full data every time.

**Request**: `GET {baseUrl}/version?app=ubibot-serial-assistant&appVersion=0.2.2.1`

**Response**:

```json
{
  "ok": true,
  "version": "lib-2026.08.20",
  "publishedAt": "2026-08-20T08:00:00Z",
  "minAppVersion": "0.2.0.0",
  "modelCount": 12,
  "commandCount": 340,
  "changelog": { "zh": "新增 GS1-4G2 网关支持", "en": "Added GS1-4G2 gateway support" }
}
```

| Field | Type | Required | Description |
|---|---|---|---|
| `version` | string | Yes | The server's current authoritative version number. |
| `publishedAt` | string (ISO 8601) | No | Publish time of this version, display-only. |
| `minAppVersion` | string | No | The minimum app version this data requires. If the client finds its own `AppController.appVersion` lower than this value, it does not show "download available" even when `version` differs — instead it shows "please upgrade the app itself first" (see section 6). Omitted means no restriction. |
| `modelCount` / `commandCount` | number | No | Display-only, for copy like "N models / M commands available". |
| `changelog` | LocalizedText (`{zh,en}`) | No | Update notes, shown in the "check for update" result in whichever language matches the current UI language. Omitted = no update notes shown. |

Client logic: `version` is compared character-by-character against the locally active version (`AppController.libraryVersion` — the last successfully applied version, or the compile-time bundled version if nothing has ever been downloaded); any difference is treated as "update available".

## 5. Endpoint 2: Fetch Latest Full Data `GET {baseUrl}/latest`

Returns the full device/command list. **The top-level structure is identical to `resources/devices.json`** (just wrapping in a few extra metadata fields; the internal schema of `models[]` is unchanged — see [device-json-protocol-schema.md](device-json-protocol-schema.md)):

```json
{
  "ok": true,
  "version": "lib-2026.08.20",
  "publishedAt": "2026-08-20T08:00:00Z",
  "models": [
    { "id": "WS1 Pro", "protocol": "at", "...": "..." },
    { "id": "GS1-JSON-01", "protocol": "json", "...": "..." }
  ]
}
```

Since `DeviceLibrary::loadFromJsonText()` already only reads the `version` and `models` fields by name, extra top-level fields like `ok`/`publishedAt` are simply ignored — the server can just return its maintained `devices.json` as-is, wrapped in an outer layer of `ok`/`publishedAt`, without needing to maintain a separate "client-only format" for this endpoint.

### Integrity check: `X-Content-SHA256` response header

The server adds a header line:

```
X-Content-SHA256: 9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08
```

The value is the SHA-256 hex digest of **the entire raw response body bytes** (not of some field, not of reformatted JSON). After receiving the response, the client computes the same SHA-256 over the raw bytes it received and compares it against this header; only on a match does it parse, cache, and apply the data. On a mismatch it's treated as "downloaded data may be corrupted" — it errors out and leaves the currently active version untouched. This header is **optional** — if the server hasn't implemented it yet, the client simply skips the check; it's not mandatory.

(Checking the whole raw byte stream rather than individual fields avoids the cross-language canonical-JSON headache of "do the server's and client's respective JSON serialization implementations byte-align" — both sides operate on the exact same bytes, so the SHA-256 naturally matches, and there's no need to worry about differences in key ordering, escaping, or float formatting.)

## 6. Client Application Flow

1. **On startup**: when `AppController` is constructed, it first loads
   `SettingsStore::cachedLibraryJson()` (the full JSON text from the last successful
   download-and-apply); if empty or unparseable, it falls back to the compile-time bundled
   `resources/devices.json` (`DeviceLibrary::loadFromResource()`). No network request is made
   automatically at startup.
2. **"Check for update"** (`AppController::checkForLibraryUpdate()`): calls endpoint 1,
   and reflects `version`/`changelog`/whether an update exists into the
   `libraryUpdateAvailable`, `remoteLibraryVersion`, `libraryUpdateMessage` properties, for
   `SettingsAboutDialog.qml` to bind and display. If `minAppVersion` is higher than the local
   `appVersion`, `libraryUpdateAvailable` is not set to `true` even when the version numbers
   differ — instead `libraryUpdateMessage` is set to something like "need to upgrade the app to
   X.X.X.X first".
3. **"Download and apply"** (`AppController::downloadLibraryUpdate()`, clickable in the UI
   only when `libraryUpdateAvailable` is true): calls endpoint 2; once the check passes:
   - `DeviceLibrary::loadFromJsonText(rawJson)` re-parses in place, replacing the entire
     in-memory model/command data;
   - `SettingsStore::setCachedLibraryJson(rawJson)` persists it for direct use on the next
     startup;
   - `CommandListModel::reload()` rebuilds the currently displayed command rows;
   - `AppController` emits `libraryChanged()`, and the `modelIds`/`libraryVersion`/
     `modelCount`/`commandCount` properties refresh automatically in QML as a result (no app
     restart needed).
4. No automatic retries and no silent background polling throughout — both actions are
   manually triggered by the user in the Settings & About panel; on failure the user just gets
   a readable error message and decides for themselves whether to click again.

## 7. Error Scenarios

| Scenario | Client behavior |
|---|---|
| `.env` not configured/empty | Both methods return "update server not configured" directly, without sending a request |
| Network timeout / unreachable | Shows `QNetworkReply::errorString()`, leaves the currently active data untouched |
| Response is not valid JSON / missing `models` | Shows "response format error", leaves the currently active data untouched |
| `X-Content-SHA256` check fails | Shows "data validation failed, may be corrupted", **does not apply** this download |
| `minAppVersion` higher than local app version | New version number exists but "download and apply" is not allowed; prompts to upgrade the app itself first |

## 8. Open Questions (does not block starting work)

1. Whether endpoint 2 should support `?since=<version>` for incremental returns (in the
   current design the server always returns the full `models[]`; the expected scale of
   models/commands is within a few hundred entries, so the size and parsing cost of a full
   transfer are negligible — incremental is not implemented for now).
2. Whether to add a local "roll back to the previous cached version" button to "download
   and apply" — in the current design `SettingsStore` only keeps one cache of "most recently
   successfully applied", with no version history; if something goes wrong, the user's only
   current recourse is to wait for the server to be fixed and then "download and apply" again
   to overwrite it.
