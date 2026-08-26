# Device Command Library · JSON Protocol Extension Design

Status: **Design draft, most key questions have been settled** (see "Open Questions" at the end of the document — what remains is minor). This document only defines the data structures; it does not cover the code changes for this round.

> **v2 update**: Confirmed based on feedback — ① once a model is selected, the serial port parameters are **force-overridden** (not "only prefilled if unchanged");
> ② JSON commands **automatically append `\r\n`** when sent, identical to the existing AT command behavior;
> ③ JSON commands that need input **skip the parameter form** and instead put the command template directly into the manual-send box,
> for the user to edit and click "Send" themselves. Sections 5, 6, 9, and 10 have been updated accordingly.

## 1. Background

The commands in the current `resources/devices.json` ([devices.json](../resources/devices.json), corresponding to the C++-side
[DeviceLibrary](../src/core/device_library.h)) are all plain-text AT command templates, for example:

```json
{ "name": { "zh": "读取设备信息", "en": "Read device info" }, "cmd": "AT+DEVINFO?" }
```

We now need to onboard a batch of devices that communicate via **JSON messages**, where the command itself is a JSON blob, for example:

```json
{"Command":"ReadProduct"}
```

The command library needs to describe: device name, device description, the device's serial port parameters (baud rate, data bits, etc.), and the command list (command name, type, whether user input is required, base64-encoded command content).

## 2. Design Principles

1. **Coexist with the existing AT protocol — don't overturn the data for the existing 4 devices.** Add a
   `protocol` field at the model level to distinguish `"at"` / `"json"`; when old data omits this field it is treated as `"at"`,
   fully backward compatible.
2. **Store command content as base64 rather than embedding the JSON string directly inside the JSON file.** Reasons:
   - Avoids having to escape `"` for "JSON nested inside JSON," which is error-prone and hard to read (imagine
     `"cmd": "{\"Command\":\"ReadProduct\"}"` — fine for a few entries, painful for dozens).
   - base64 is binary-safe — if some future device uses a binary frame format other than JSON,
     the same field (`payloadBase64`) doesn't need to change.
   - Convenient for editor/script validation: the decoded result must be valid JSON that `json.loads` can parse successfully.
3. **Commands that need input skip the structured form.** After decoding, the payload is a template with `<key>` placeholders,
   and the whole thing is put directly into the manual-send box, for the user to replace the placeholders with real values themselves before clicking "Send"
   — this is a different interaction from the existing AT template's (`AT+INTERVAL=<sec>`) auto-substitution + pop-up parameter form
   ([DeviceCommand::resolve](../src/core/device_library.h:36) +
   `CommandParamsPanel`). For this batch of JSON protocol commands we deliberately chose the simpler,
   more direct "edit in the text box and send" approach — see Section 9 for the reasoning and detailed flow.

## 3. Top-level structure

No new file is added — it's still a single `devices.json`. AT-protocol and JSON-protocol
devices are mixed together in the `models` array, distinguished by each model's `protocol` field:

```json
{
  "version": "lib-2026.08.12",
  "models": [
    { "id": "WS1 Pro", "protocol": "at", "...": "existing 4 devices, fields unchanged" },
    { "id": "GS1-JSON-01", "protocol": "json", "...": "new protocol device" }
  ]
}
```

## 4. model (device) fields

| Field | Type | Required | Description |
|---|---|---|---|
| `id` | string | Yes | Unique identifier for the device model, carried over from the current behavior — used as the dropdown value, favorites/settings key, and internal reference. Not localized; generally just the model name, e.g. `"GS1-JSON-01"`. |
| `protocol` | `"at"` \| `"json"` | No, defaults to `"at"` | The command protocol type for this device. Determines whether each command in `commands[]` is parsed per Section 6 (`json`) or via the original `cmd`/`params` fields (`at`). |
| `name` | LocalizedText | No | **New**: the device's display name (e.g. "GS1 JSON Gateway"). Kept separate from `id` because `id` must stay stable and shouldn't be changed casually, while the display name may need localization or copy changes later. When omitted, the UI shows `id` directly. |
| `description` | LocalizedText | Yes | Carried over from the current behavior: a one-line description of the model, shown as gray text below the model dropdown. |
| `serial` | SerialDefaults | No | **New**: this device's serial port parameters (baud rate, data bits, etc.), see Section 5. When omitted, the App's current serial settings are left unchanged. |
| `commands` | Command[] | Yes | The command list. The fields of each entry in the array are determined by `protocol`, see Section 6. |

`LocalizedText` is the existing `{ "zh": "...", "en": "..." }` structure, unchanged.

## 5. serial (serial port defaults, new)

```json
"serial": {
  "baudRate": 115200,
  "dataBits": 8,
  "parity": "None",
  "stopBits": 1,
  "flowControl": "None"
}
```

| Field | Type | Allowed values | Default |
|---|---|---|---|
| `baudRate` | number | any positive integer (commonly 1200~921600) | 115200 |
| `dataBits` | number | `5` `6` `7` `8` | 8 |
| `parity` | string | `"None"` `"Even"` `"Odd"` `"Space"` `"Mark"` | `"None"` |
| `stopBits` | number | `1` `1.5` `2` | 1 |
| `flowControl` | string | `"None"` `"Hardware"` `"Software"` | `"None"` |

All five fields may be omitted; omitted ones use the defaults in the table above. **Behavior confirmed: as soon as this model is selected in the
device command page / connection wizard, the serial settings panel immediately force-overrides according to this `serial` block** (baud
rate, data bits, etc. are all changed to the values given here), regardless of anything the user changed manually before — selecting a model is taken to mean
the user wants to follow that device's spec, and old values are not preserved. Models that omit `serial` (all 4 existing AT
devices are like this) leave the serial settings unchanged when selected, preserving current behavior.

The string values align with `Qt::SerialPort`'s enum names (`QSerialPort::Parity` /
`QSerialPort::FlowControl`); implementation just needs a string-to-enum mapping layer, reusing
the option list already present in [SerialOptions](../src/app/serial_options.h).

## 6. command fields — when `protocol: "json"`

```json
{
  "id": "read-product",
  "group": { "zh": "设备信息", "en": "Device Info" },
  "name": { "zh": "读取设备信息", "en": "Read device info" },
  "type": "query",
  "needsInput": false,
  "payloadBase64": "eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ=="
}
```

| Field | Type | Required | Description |
|---|---|---|---|
| `id` | string | Yes | **New**: a stable identifier for the command (e.g. `"read-product"`). The existing AT command favorites feature uses `name.zh` as the key ([settings_store.cpp](../src/core/settings_store.cpp)), so changing the Chinese name breaks favorites — this batch of JSON protocol commands takes the opportunity to give every command a stable `id` that favorites/history both use. |
| `group` | LocalizedText | Yes | Group name, carried over from current behavior ("Device Info"/"Data"/"Config"...), used to section the list. |
| `name` | LocalizedText | Yes | Command display name, carried over from current behavior. |
| `type` | `"query"` \| `"set"` \| `"action"` | Yes | Command type, see table below. |
| `needsInput` | boolean | Yes | **Determines the behavior after clicking this command** (see Section 9): when `false`, sends immediately; when `true`, does not send but instead puts the decoded command text into the manual-send box, waiting for the user to edit it and click "Send". |
| `params` | CommandParam[] | No, hint-only | ⚠️ **No longer drives a parameter form** (in the original design, JSON protocol commands would pop up a structured form like AT commands; this has now been changed to direct text-box editing, see Section 9). This field is kept purely as documentation — recording what `<key>` placeholders exist in this command and roughly what should go in them, to make it easy to show a hint line above the input box later. The structure isn't enforced; recommend keeping `{ "key", "label", "hint", "default" }`. |
| `payloadBase64` | string | Yes | The command content: write out the target JSON text (**for commands that need input, use `<key>` placeholders in place of concrete values — kept as-is, no programmatic substitution**), UTF-8 encode it, then base64. When `needsInput: true`, this decoded text (with placeholders left as-is) is the initial content placed into the manual-send box. |

### `type` value meanings

| Value | Meaning | Typically `needsInput` | Example |
|---|---|---|---|
| `query` | Read-only query, no side effects, can be sent anytime | No | `{"Command":"ReadProduct"}` |
| `set` | Writes/modifies device configuration | Usually yes | `{"Command":"SetInterval","Value":"<sec>"}` |
| `action` | Triggers a one-off action with side effects (reboot, reset, etc.); the UI layer may want to consider a confirmation before sending | Usually no | `{"Command":"Reboot"}` |

## 7. base64 encoding rule

`payloadBase64` = `Base64( UTF8( target JSON text ) )`. You can run the lines below directly in a terminal
to verify they match the examples given in this document (on Windows PowerShell use
`[Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes('...'))`; the version below is
`bash`):

```bash
printf '%s' '{"Command":"ReadProduct"}' | base64
# eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ==

printf '%s' '{"Command":"SetInterval","Value":"<sec>"}' | base64
# eyJDb21tYW5kIjoiU2V0SW50ZXJ2YWwiLCJWYWx1ZSI6IjxzZWM+In0=

printf '%s' '{"Command":"Reboot"}' | base64
# eyJDb21tYW5kIjoiUmVib290In0=
```

Decoding the other way for verification (`base64 -d`):

```bash
echo 'eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ==' | base64 -d
# {"Command":"ReadProduct"}
```

## 8. Full example

A complete model definition for a JSON protocol device, covering all three command types `query` / `set` /
`action`:

```json
{
  "id": "GS1-JSON-01",
  "protocol": "json",
  "name": { "zh": "GS1 JSON 网关", "en": "GS1 JSON Gateway" },
  "description": {
    "zh": "轻松连 GS1 · JSON 协议版本，4G 工业网关",
    "en": "Easylink GS1 · JSON-protocol variant, 4G industrial gateway"
  },
  "serial": {
    "baudRate": 115200,
    "dataBits": 8,
    "parity": "None",
    "stopBits": 1,
    "flowControl": "None"
  },
  "commands": [
    {
      "id": "read-product",
      "group": { "zh": "设备信息", "en": "Device Info" },
      "name": { "zh": "读取设备信息", "en": "Read device info" },
      "type": "query",
      "needsInput": false,
      "payloadBase64": "eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ=="
    },
    {
      "id": "set-interval",
      "group": { "zh": "配置", "en": "Config" },
      "name": { "zh": "设置上传间隔", "en": "Set upload interval" },
      "type": "set",
      "needsInput": true,
      "params": [
        { "key": "sec", "label": { "zh": "上传间隔 (秒)", "en": "Upload interval (s)" }, "hint": "300", "default": "300" }
      ],
      "payloadBase64": "eyJDb21tYW5kIjoiU2V0SW50ZXJ2YWwiLCJWYWx1ZSI6IjxzZWM+In0="
    },
    {
      "id": "set-wifi",
      "group": { "zh": "配置", "en": "Config" },
      "name": { "zh": "设置 WiFi", "en": "Set WiFi" },
      "type": "set",
      "needsInput": true,
      "params": [
        { "key": "ssid", "label": { "zh": "SSID", "en": "SSID" }, "hint": "UbiBot-Office", "default": "UbiBot-Office" },
        { "key": "pwd", "label": { "zh": "密码", "en": "Password" }, "hint": "••••••", "default": "" }
      ],
      "payloadBase64": "eyJDb21tYW5kIjoiU2V0V2lmaSIsIlNzaWQiOiI8c3NpZD4iLCJQYXNzd29yZCI6Ijxwd2Q+In0="
    },
    {
      "id": "reboot",
      "group": { "zh": "维护", "en": "Maintenance" },
      "name": { "zh": "重启设备", "en": "Reboot device" },
      "type": "action",
      "needsInput": false,
      "payloadBase64": "eyJDb21tYW5kIjoiUmVib290In0="
    }
  ]
}
```

`set-interval` decodes to `{"Command":"SetInterval","Value":"<sec>"}` — because
`needsInput: true`, clicking this command puts this text as-is (placeholder not substituted) into the manual-send
box, and the user changes `<sec>` to `600` themselves and clicks "Send"; what actually gets sent is the text string as edited by the user
(whether it's valid JSON, whether `<sec>` was fully replaced — that's all on the user;
the App does not validate it).

## 9. Send flow (confirmed, but see revision below)

> **Subsequent revision**: The device command library is now positioned as "quickly find a command" — it is no longer
> responsible for sending, and no longer cares whether the serial port is open. Clicking any row in the list (regardless of
> JSON protocol or not, regardless of whether `needsInput` is `true` or `false`) only puts the command text into the manual-send box
> (`AppController.loadCommandIntoDraft` / for parameterized ones,
> `AppController.loadCommandWithParamsIntoDraft`), and never auto-sends; actually sending
> is always only possible via the manual-send box's own "Send" (`sendManualText()`, which checks whether the serial port is
> open). The description below of `needsInput: false` sending directly, and other mentions elsewhere in this document of
> "auto-substitute, send directly," are the old behavior from before this change and are kept only as a historical design record;
> the `needsInput` field itself still exists with unchanged meaning (`true` means the payload still has
> unreplaced `<key>` placeholders that the user needs to fill in themselves) — it just no longer drives "whether to send immediately."

For `protocol: "json"` commands, clicking a row in the list splits into two cases (**historical record — current
actual behavior is per the revision above**):

### `needsInput: false` (typically: `query`, `action`) — send directly

1. Decode `payloadBase64` with `QByteArray::fromBase64()`, then `QString::fromUtf8()`
   to get the JSON text (this class of command has no placeholders).
2. (Historical design, superseded by the "subsequent revision" above) At the time, this followed the same path as parameterless AT commands: send
   this text directly, **automatically appending `\r\n`** (reusing the existing
   [composeAsciiPayload](../src/app/app_controller.cpp:191) — commands from both protocols go through
   the same unified framing, with no separate customization).
3. Whether to echo into the data monitor area is decided by the `echoTx` setting, consistent with existing command behavior.

### `needsInput: true` (typically: `set`) — put into manual-send box, user completes it

1. Likewise decode `payloadBase64` to get the JSON text, **leaving the `<key>` placeholders as-is, with no
   substitution whatsoever**.
2. Write this text as a whole into the manual-send box (`AppController.draftText`), **without auto-sending**,
   waiting for the user to change the placeholders to real values themselves.
   - If the send box already has unsent content the user is editing, overwrite it directly — consistent with the current behavior of
     double-clicking a history entry in the "Command History" panel ([Main.qml](../qml/Main.qml:393): overwrites
     draftText directly), with no extra confirmation pop-up.
3. Once the user finishes editing and clicks "Send," it goes through the existing `sendManualText()` path — likewise
   automatically appending `\r\n`, and likewise recorded in send history. In other words, `needsInput: true`
   JSON commands **do not need** the existing [CommandParamsPanel](../qml/CommandParamsPanel.qml)
   structured parameter form; the `params` field is thus reduced to the "reference only" role described in Section 6.

AT protocol (`protocol: "at"` or omitted) command behavior is **completely unchanged**: commands with parameters still go through
the `CommandParamsPanel` form + auto-substitution + direct send. This change only affects JSON
protocol commands.

## 10. Differences from the existing format

| | Existing AT protocol (`protocol: "at"`, or omitted) | New JSON protocol (`protocol: "json"`) |
|---|---|---|
| Command content field | `cmd` (plain-text string template, e.g. `"AT+INTERVAL=<sec>"`) | `payloadBase64` (base64 of the target JSON text) |
| Placeholder location | Written directly in the `cmd` string | Written in the JSON text after base64 decoding |
| Command classification | None | `type`: `query` / `set` / `action` |
| Interaction when input is needed | Pops up the `CommandParamsPanel` structured form; after filling it in, auto-substitutes and sends directly | Puts the text with placeholders into the manual-send box; the user edits it themselves and clicks "Send" |
| Role of `params` | Drives the parameter form (label/hint/default all render as form fields) | Documentation only, does not drive any UI |
| Stable command key | None (favorites feature borrows `name.zh`; renaming loses favorites) | Explicit `id` field |
| Device display name | No separate field, UI shows `id` directly | New `name` field (localized) |
| Serial port parameters | None, uses the App's current settings | New `serial` field; force-overrides upon model selection |
| Frame terminator | Automatically appends `\r\n` | Automatically appends `\r\n` (consistent across both protocols) |

The existing 4 devices (WS1 / WS1 Pro / GS1-AL4G1RS / SP1) **need no changes** — omitting `protocol`
is treated as `"at"`, fully backward compatible.

## 11. Confirmed / Open Questions

**Confirmed** (this v2 has updated the body text accordingly):

- Selecting a model with `serial` → serial settings are **force-overridden**, regardless of anything the user changed before.
- JSON commands **automatically append `\r\n`** when sent, consistent with AT commands.
- `needsInput: true` JSON commands **do not pop up a parameter form** — instead the command text is put into the manual-send
  box, for the user to edit and click "Send" themselves; the `params` field is downgraded to purely documentational.

**A few minor questions remain**, which don't block starting implementation and can be settled along the way:

1. **Are the three `type` values (`query`/`set`/`action`) enough?** For example, should a separate
   `subscribe` (subscription/continuous reporting) type be added?
2. **Should the existing 4 AT protocol devices also get an `id` field added** (switching the favorites key from
   `name.zh` to a stable `id`, incidentally fixing the minor "renaming loses favorites" issue)? This is
   unrelated to the JSON protocol itself, but since the new protocol has already introduced `id`, should the old data
   be backfilled too, to unify the favorites mechanism?
3. Should an "expected response" related field be added to commands (e.g. which field should be present in the
   JSON the device returns), for future automatic validation of whether a reply succeeded? This is clearly out of scope for now — listed
   here for the record; if you don't need it for the time being, it doesn't need to be designed yet.

None of the minor questions above block starting work — I can go ahead and implement the `DeviceLibrary`/
`DeviceCommand` code changes and the actual `devices.json` data per the current design. Whether to add new `type` values,
or backfill `id` on old devices, can be decided and added anytime later; neither is a breaking change.
