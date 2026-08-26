# Device Command Library · Mock Server

A minimal implementation of the two endpoints described in `docs/device-library-update-protocol.md`, for local dev/demo purposes only — **no authentication**. Data is read directly from `data/devices.json` + `data/meta.json` (re-read from disk on every request — after editing the files, just click "Check for updates" on the client to see the effect, no need to restart this server).

## Run

```bash
cd server
go run . -addr :8980
```

(`-addr`/`-data` are both optional flags, defaulting to `:8980` and `./data`.)

## Connecting the client

Create a `.env` in the repo root (gitignored, won't be committed — see [.env.example](../.env.example)):

```
DEVICE_LIBRARY_API_BASE_URL=http://127.0.0.1:8980/api/device-library
DEVICE_LIBRARY_API_KEY=
```

Put it next to the packaged build (`deploy/UbiBotSerialAssistant/.env`) or in the project's current working directory (during development, just run it from the build directory) — the next "Check for updates" will hit this mock server.

The `/api/device-library` path is deliberately aligned with the real backend (`App\Http\Controllers\DeviceLibraryController`, registered in `server/routes/api.php` in the `ubibot-appcenter` repo) — the two only differ in host:port, the path structure is identical, so switching `.env` doesn't require changing anything else.

## Data files

- `data/devices.json` — same schema as [resources/devices.json](../resources/devices.json) (see [docs/device-json-protocol-schema.md](../docs/device-json-protocol-schema.md)); `/latest` passes `models` through as-is. The copy bundled in this repo has `version` bumped to `lib-2026.08.25` (newer than the `lib-2026.08.12` baked into the client at build time), plus an extra `reboot` command, so you can see the "update available" effect right after starting the server.
- `data/meta.json` — the fields on the `/version` endpoint that `devices.json` itself doesn't carry: `publishedAt`, `minAppVersion`, `changelog` (localized release notes). Missing fields just come back empty — it doesn't break the response.

## The two endpoints

- `GET /api/device-library/version` — metadata (`version`/`publishedAt`/`minAppVersion`/`modelCount`/`commandCount`/`changelog`).
- `GET /api/device-library/latest` — full data (`{ok, version, publishedAt, models}`), response carries an `X-Content-SHA256` header (SHA-256 of the entire response body), which the client verifies.

For the full field reference and error envelope format for both endpoints, see [docs/device-library-update-protocol.md](../docs/device-library-update-protocol.md).
