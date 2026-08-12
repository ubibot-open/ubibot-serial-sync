# 设备指令库 · JSON 协议扩展设计

状态：**设计稿，多数关键问题已拍板**（见文末「待确认事项」，剩下的是次要项）。本文档只定义数据结构，不涉及本次的代码改动。

> **v2 更新**：根据反馈确认——① 选中型号后串口参数**强制覆盖**（不是"仅在未改动时预填"）；
> ② JSON 指令发送时**自动追加 `\r\n`**，和现有 AT 指令的行为完全一致；
> ③ 需要输入的 JSON 指令**不走参数表单**，而是把指令模板直接放进手动发送框，
> 由用户自己改完再点「发送」。第 5、6、9、10 节已按此更新。

## 1. 背景

现有 `resources/devices.json`（[devices.json](../resources/devices.json)，对应 C++
端 [DeviceLibrary](../src/core/device_library.h)）里的指令都是纯文本 AT 指令模板，例如：

```json
{ "name": { "zh": "读取设备信息", "en": "Read device info" }, "cmd": "AT+DEVINFO?" }
```

现在要接入一批用 **JSON 报文**通信的设备，指令本身是一段 JSON，例如：

```json
{"Command":"ReadProduct"}
```

需要在指令库里描述：设备名、设备描述、设备的串口参数（波特率、数据位等）、以及指令列表（指令名、类型、是否需要用户输入、指令内容的 base64 编码）。

## 2. 设计原则

1. **与现有 AT 协议共存，不推翻现有 4 款设备的数据**。在 model 级加一个
   `protocol` 字段区分 `"at"` / `"json"`，旧数据不填这个字段时按 `"at"`
   处理，完全向后兼容。
2. **指令内容用 base64 存，而不是把 JSON 字符串直接嵌进 JSON 文件**。原因：
   - 避免"JSON 里塞 JSON"要对 `"` 做转义，写起来容易错、也难读（试想
     `"cmd": "{\"Command\":\"ReadProduct\"}"` 这种写法，几条还好，几十条就很痛苦）。
   - base64 是二进制安全的，以后如果某个设备用的不是 JSON 而是别的二进制帧
     格式，同一套字段（`payloadBase64`）不用改。
   - 编辑器/脚本校验方便：解出来的必须是能 `json.loads` 成功的合法 JSON。
3. **需要输入的指令不走结构化表单**，payload 解码后是带 `<key>` 占位符的
   模板，直接整段放进手动发送框，由用户自己把占位符改成真实值再点「发送」
   —— 和现有 AT 模板（`AT+INTERVAL=<sec>`）那套自动替换 + 弹参数表单
   （[DeviceCommand::resolve](../src/core/device_library.h:36) +
   `CommandParamsPanel`）是两种不同的交互，JSON 协议这批指令刻意选了更
   简单直接的"文本框里改完直接发"方式，理由和具体流程见第 9 节。

## 3. 顶层结构

不新增文件，仍是一份 `devices.json`，`models` 数组里 AT 协议和 JSON 协议的
设备混在一起，靠每个 model 的 `protocol` 字段区分：

```json
{
  "version": "lib-2026.08.12",
  "models": [
    { "id": "WS1 Pro", "protocol": "at", "...": "现有 4 款设备，字段不变" },
    { "id": "GS1-JSON-01", "protocol": "json", "...": "新协议设备" }
  ]
}
```

## 4. model（设备）字段

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `id` | string | 是 | 设备型号的唯一标识，沿用现状——用作下拉框的值、收藏/设置的 key、内部引用。不做本地化，一般就是型号名，如 `"GS1-JSON-01"`。 |
| `protocol` | `"at"` \| `"json"` | 否，默认 `"at"` | 该设备的指令协议类型。决定 `commands[]` 里每条指令按第 6 节（`json`）还是原有 `cmd`/`params` 字段（`at`）解析。 |
| `name` | LocalizedText | 否 | **新增**：设备的展示名（如"GS1 JSON 网关"）。和 `id` 分开，是因为 `id` 要保持稳定不能随便改，但展示名可能需要本地化/以后改文案。不填时 UI 直接显示 `id`。 |
| `description` | LocalizedText | 是 | 沿用现状：型号的一句话描述，显示在型号下拉框下方的灰字里。 |
| `serial` | SerialDefaults | 否 | **新增**：该设备的串口参数（波特率/数据位等），见第 5 节。省略时沿用 App 当前的串口设置不变。 |
| `commands` | Command[] | 是 | 指令列表。数组内每一项的字段由 `protocol` 决定，见第 6 节。 |

`LocalizedText` 就是现有的 `{ "zh": "...", "en": "..." }` 结构，不变。

## 5. serial（串口默认参数，新增）

```json
"serial": {
  "baudRate": 115200,
  "dataBits": 8,
  "parity": "None",
  "stopBits": 1,
  "flowControl": "None"
}
```

| 字段 | 类型 | 允许值 | 默认 |
|---|---|---|---|
| `baudRate` | number | 任意正整数（常见 1200~921600） | 115200 |
| `dataBits` | number | `5` `6` `7` `8` | 8 |
| `parity` | string | `"None"` `"Even"` `"Odd"` `"Space"` `"Mark"` | `"None"` |
| `stopBits` | number | `1` `1.5` `2` | 1 |
| `flowControl` | string | `"None"` `"Hardware"` `"Software"` | `"None"` |

五个字段都可省略，省略的按上表默认值处理。**行为已确认：只要在设备指令页/
连接向导里选中了这个型号，串口设置面板就立刻按这份 `serial` 强制覆盖**（波特
率、数据位等全部改成这里给的值），不管用户之前手动改过什么——选型号即认为
用户是要按这台设备的规格来，不保留旧值。省略 `serial` 的型号（现有 4 款 AT
设备都是这样）选中时不改动串口设置，维持现状。

字符串取值和 `Qt::SerialPort` 的枚举名对齐（`QSerialPort::Parity` /
`QSerialPort::FlowControl`），实现时做一层字符串到枚举值的映射即可，复用
现有 [SerialOptions](../src/app/serial_options.h) 里已经有的选项列表。

## 6. command（指令）字段 —— `protocol: "json"` 时

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

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `id` | string | 是 | **新增**：指令的稳定标识（如 `"read-product"`）。现有 AT 指令的收藏功能是拿 `name.zh` 当 key（[settings_store.cpp](../src/core/settings_store.cpp)），这样一改中文名收藏就丢了——JSON 协议这批借这次机会给每条指令一个不会变的 `id`，收藏/记录都用它。 |
| `group` | LocalizedText | 是 | 分组名，沿用现状（"设备信息"/"数据"/"配置"…），列表里按它分节。 |
| `name` | LocalizedText | 是 | 指令展示名，沿用现状。 |
| `type` | `"query"` \| `"set"` \| `"action"` | 是 | 指令类型，见下表。 |
| `needsInput` | boolean | 是 | **决定点击这条指令后的行为**（见第 9 节）：`false` 时立刻发送；`true` 时不发送，而是把解码出的指令文本放进手动发送框，等用户自己改完点「发送」。 |
| `params` | CommandParam[] | 否，仅作提示用 | ⚠️ **不再驱动参数表单**（原设计里 JSON 协议会像 AT 指令一样弹结构化表单，现已改为直接编辑文本框，见第 9 节）。这个字段保留仅作为文档性说明——记录这条指令里有哪些 `<key>` 占位符、大概该填什么，方便以后在输入框上方显示一行提示文字。结构不强制，建议沿用 `{ "key", "label", "hint", "default" }`。 |
| `payloadBase64` | string | 是 | 指令内容：写出目标 JSON 文本（**需要输入的指令，用 `<key>` 占位符代替具体值，原样保留、不做程序替换**），UTF-8 编码后做 base64。`needsInput: true` 时，这段解码出的文本（占位符原样保留）就是要放进手动发送框的初始内容。 |

### `type` 取值说明

| 值 | 含义 | 一般是否 `needsInput` | 示例 |
|---|---|---|---|
| `query` | 只读查询，无副作用，随时可发 | 否 | `{"Command":"ReadProduct"}` |
| `set` | 写入/修改设备配置 | 通常是 | `{"Command":"SetInterval","Value":"<sec>"}` |
| `action` | 触发一次性动作，有副作用（重启、复位等），发送前 UI 层面可以考虑二次确认 | 通常否 | `{"Command":"Reboot"}` |

## 7. base64 编码规则

`payloadBase64` = `Base64( UTF8( 目标JSON文本 ) )`。下面几行你可以直接在终端
里跑，验证和本文档给的示例是否对得上（Windows PowerShell 用
`[Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes('...'))`，下面给
的是 `bash` 版本）：

```bash
printf '%s' '{"Command":"ReadProduct"}' | base64
# eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ==

printf '%s' '{"Command":"SetInterval","Value":"<sec>"}' | base64
# eyJDb21tYW5kIjoiU2V0SW50ZXJ2YWwiLCJWYWx1ZSI6IjxzZWM+In0=

printf '%s' '{"Command":"Reboot"}' | base64
# eyJDb21tYW5kIjoiUmVib290In0=
```

反过来解码校验（`base64 -d`）：

```bash
echo 'eyJDb21tYW5kIjoiUmVhZFByb2R1Y3QifQ==' | base64 -d
# {"Command":"ReadProduct"}
```

## 8. 完整示例

一个 JSON 协议设备的完整 model 定义，覆盖 `query` / `set` / `action` 三种
指令类型：

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

`set-interval` 解码后是 `{"Command":"SetInterval","Value":"<sec>"}`——因为
`needsInput: true`，点击这条指令后，这段文本会原样（占位符不替换）放进手动
发送框，用户自己把 `<sec>` 改成 `600`，再点「发送」，实际发出去的是用户改
完的那一串文本（是否合法 JSON、`<sec>` 有没有替换干净，都由用户自己保证，
App 不做校验）。

## 9. 发送流程（已确认）

`protocol: "json"` 的指令，点击列表里的一行后分两种情况：

### `needsInput: false`（典型：`query`、`action`）—— 直接发送

1. 从 `payloadBase64` 用 `QByteArray::fromBase64()` 解码，`QString::fromUtf8()`
   得到 JSON 文本（这类指令本身不含占位符）。
2. 和现有无参 AT 指令（[AppController::activateCommandRow](../src/app/app_controller.cpp:308)
   → `sendLiteral`）一样的路径：直接把这段文本发出去，**自动追加 `\r\n`**
   （复用现有 [composeAsciiPayload](../src/app/app_controller.cpp:191)，两种
   协议的指令统一走同一套 framing，不单独定制）。
3. 按 `echoTx` 设置决定是否回显到数据监视区，行为和现有指令一致。

### `needsInput: true`（典型：`set`）—— 放进手动发送框，用户自己完成

1. 同样解码 `payloadBase64` 得到 JSON 文本，**占位符 `<key>` 原样保留，不做
   任何替换**。
2. 把这段文本整个写入手动发送框（`AppController.draftText`），**不自动发送**，
   等用户自己把占位符改成真实值。
   - 如果发送框里已经有用户正在编辑的未发送内容，直接覆盖——和现在「指令
     历史」面板双击一条历史记录时的行为（[Main.qml](../qml/Main.qml:393)：
     直接覆盖 draftText）保持一致，不额外弹确认。
3. 用户改完点「发送」，走的是现有的 `sendManualText()` 路径——同样
   自动追加 `\r\n`，同样会被记入发送历史。也就是说 `needsInput: true` 的
   JSON 指令**不需要**现有 [CommandParamsPanel](../qml/CommandParamsPanel.qml)
   那套结构化参数表单，`params` 字段也就退化成第 6 节说的"仅供参考"。

AT 协议（`protocol: "at"` 或省略）的指令行为**完全不变**：有参数照样走
`CommandParamsPanel` 表单 + 自动替换 + 直接发送，这次的改动只影响 JSON
协议的指令。

## 10. 与现有格式的差异一览

| | 现有 AT 协议 (`protocol: "at"`，或省略) | 新 JSON 协议 (`protocol: "json"`) |
|---|---|---|
| 指令内容字段 | `cmd`（明文字符串模板，如 `"AT+INTERVAL=<sec>"`） | `payloadBase64`（目标 JSON 文本的 base64） |
| 占位符位置 | 直接写在 `cmd` 字符串里 | 写在 base64 解码后的 JSON 文本里 |
| 指令分类 | 无 | `type`：`query` / `set` / `action` |
| 需要输入时的交互 | 弹 `CommandParamsPanel` 结构化表单，填完自动替换、直接发送 | 把带占位符的文本放进手动发送框，用户自己改完点「发送」 |
| `params` 的作用 | 驱动参数表单（label/hint/default 都会渲染成表单项） | 仅文档性说明，不驱动任何 UI |
| 指令稳定 key | 无（收藏功能借用 `name.zh`，改名会丢收藏） | 显式 `id` 字段 |
| 设备展示名 | 无独立字段，UI 直接显示 `id` | 新增 `name`（本地化） |
| 串口参数 | 无，沿用 App 当前设置 | 新增 `serial`，选中型号即强制覆盖 |
| 帧结束符 | 自动追加 `\r\n` | 自动追加 `\r\n`（两种协议一致） |

现有 4 款设备（WS1 / WS1 Pro / GS1-AL4G1RS / SP1）**不需要改动**，`protocol`
省略即按 `"at"` 处理，完全向后兼容。

## 11. 已确认 / 待确认

**已确认**（本 v2 已按这些更新正文）：

- 选中带 `serial` 的型号 → 串口设置**强制覆盖**，不管用户之前改过什么。
- JSON 指令发送**自动追加 `\r\n`**，和 AT 指令一致。
- `needsInput: true` 的 JSON 指令**不弹参数表单**，改为把指令文本放进手动
  发送框，用户自己编辑、自己点「发送」；`params` 字段降级为纯文档性说明。

**还剩几个次要问题**，不影响先动手实现，可以边做边定：

1. **`type` 的三个值（`query`/`set`/`action`）够不够用**？比如要不要单独
   加一个 `subscribe`（订阅/持续上报）类型？
2. **现有 4 款 AT 协议设备要不要也补上 `id` 字段**（把收藏 key 从
   `name.zh` 换成稳定 `id`，顺带修掉"改名丢收藏"这个小问题）？这个和这次
   的 JSON 协议本身无关，但既然新协议已经引入了 `id`，是否要一并把旧数据
   也补齐，统一收藏机制。
3. 要不要给指令加一个"期望响应"相关的字段（比如设备返回的 JSON 里应该有
   哪个字段），用于以后自动校验回复是否成功？这个明显超出当前范围，先列
   在这里，你们如果暂时不需要就先不设计。

以上次要问题不影响开工，我可以先按当前设计落地 `DeviceLibrary`/
`DeviceCommand` 的代码改动和 `devices.json` 的实际数据；`type` 加不加新值、
旧设备要不要补 `id`，之后随时可以再加，不是破坏性变更。
