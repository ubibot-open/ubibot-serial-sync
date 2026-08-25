# 设备指令库 · 远程更新协议设计

状态：**设计稿，可据此开工**。本文档只定义客户端 ↔ 服务端的接口协议和客户端侧的
取数逻辑，不涉及服务端的具体实现技术栈（由维护设备列表的后台自行决定）。

## 1. 背景与目标

[resources/devices.json](../resources/devices.json)（对应
[DeviceLibrary](../src/core/device_library.h)）目前是**编译期打包进 exe 的
Qt 资源**——新增一个型号或一条指令，必须改这个文件、重新编译发布，用户才能
用上。

现在的计划：把"设备列表 + 每个设备支持的指令列表"这份数据的**权威版本放到服务端
维护**，客户端在启动时用上次下载的缓存（没有缓存则用编译期打包的兜底版本），
并提供"检查更新"/"下载并应用最新版本"两个动作，二者都通过下面两个只读 JSON
接口完成。**数据本身的 schema 不变**——服务端返回的 `models[]` 和现有
[device-json-protocol-schema.md](device-json-protocol-schema.md) 描述的完全
一致，这次只是新增了"从哪里取这份数据"的一层，取到之后如何解析是一样的代码
路径。

## 2. 客户端配置：`.env`

后台服务的地址不写进代码库（不同环境/内部域名没必要公开、也没必要每次改都
重新编译），改为运行时从一个**不提交到 git** 的 `.env` 文件读取（参见
[.env.example](../.env.example) 和 [.gitignore](../.gitignore)）：

```
DEVICE_LIBRARY_API_BASE_URL=https://appcenter.ubibot.com/api/device-library
DEVICE_LIBRARY_API_KEY=
```

- `DEVICE_LIBRARY_API_BASE_URL`：两个接口的公共前缀（不带末尾 `/`，客户端会
  自动去掉多余的斜杠）。**留空或整个 `.env` 文件不存在** = 不启用远程更新，
  App 只用编译期打包的 `resources/devices.json`，"检查更新"按钮会直接提示
  "未配置更新服务器"，不会报错崩溃。协议本身（下面第 4、5 节）只定义相对路径
  `/version`、`/latest`——`/api/device-library` 这段前缀完全是配置项，不是客户端
  代码里硬编码的东西；真实后台（见第 3.1 节）恰好把两个接口注册在这个路径下，
  所以生产环境的 `DEVICE_LIBRARY_API_BASE_URL` 才长这样。
- `DEVICE_LIBRARY_API_KEY`：可选。非空时客户端在每次请求上带
  `X-Api-Key: <这个值>` 请求头，供后台按需做访问控制；后台如果不需要鉴权，
  留空即可。当前真实后台（第 3.1 节）没做鉴权，留空即可。

客户端查找 `.env` 的顺序（见 [EnvConfig](../src/core/env_config.h)）：
1. 可执行文件所在目录（`deploy/UbiBotSerialAssistant/.env`）—— 生产/打包环境。
2. 当前工作目录 —— 开发时直接在构建目录里跑，方便临时指向测试后台。
3. 这份代码检出的源码根目录（编译期写死的路径，见
   [CMakeLists.txt](../CMakeLists.txt) 的 `APP_SOURCE_DIR`）—— 纯开发便利，
   让放在仓库根目录的 `.env` 在用 Qt Creator 跑（默认工作目录是构建目录，不是
   源码目录）时也能被找到；换一台机器/正式打包后这个路径自然不存在，查找会
   静默跳过。

三处都找不到就是禁用状态。**打包发布时，需要运维/发布流程把真正的 `.env`
放进 `deploy/UbiBotSerialAssistant/` 目录**（和 `windeployqt` 产物放一起），
这一步不在本仓库的构建脚本里自动完成。

## 3. 通用约定

- 两个接口都是 `GET`，返回 `Content-Type: application/json`。
- 客户端在 query string 上附加 `app=ubibot-serial-assistant`、
  `appVersion=<APP_VERSION>`（如 `0.2.2.1`），供后台做统计/灰度，服务端可以
  忽略。
- 请求超时 8 秒（客户端侧固定值），超时按网络错误处理，不重试——重试与否
  由用户手动再次点击"检查更新"决定。
- 响应体统一带一个顶层 `"ok"` 布尔字段；服务端**默认按 `ok: true` 处理**
  （字段缺失时客户端不当错误），但建议服务端显式给出。`ok: false` 时，客户端
  只读取 `error.message` 展示给用户，其余字段不要求存在：

  ```json
  { "ok": false, "error": { "code": "SERVICE_UNAVAILABLE", "message": "..." } }
  ```

- 版本号统一用形如 `lib-2026.08.20` 的可字典序比较的字符串（和现有
  `resources/devices.json` 的 `version` 字段同一套约定），服务端保证新版本
  号字典序大于旧版本号；客户端只做字符串相等/不等比较（不等 = 有更新），不做
  语义化版本比较。

### 3.1 真实后台实现

真实后台是 `ubibot-appcenter` 仓库的 `server/`（Laravel 应用）：
`app/Http/Controllers/DeviceLibraryController.php` 实现下面两个接口的具体
逻辑，`routes/api.php` 里注册路由；因为 `routes/api.php` 整体已经被
`RouteServiceProvider` 套了一层 `api` 前缀，所以最终对外路径是
`/api/device-library/version`、`/api/device-library/latest`（**不需要
鉴权**，和该项目里 `/api/software/list` 这类公开只读接口一个待遇）。数据来自
该仓库的 `resources/device-library/{devices,meta}.json` 两个文件——目前还
没有配套的后台管理 CRUD 界面，更新设备库靠直接改这两个 JSON 文件、走一次
正常部署流程；以后有了设备管理页面，再把 Controller 里的读文件逻辑换成读
数据库，这两个接口本身的路径/响应格式不用变。

本仓库 `server/` 目录下的 Go 模拟服务端（[server/README.md](../server/README.md)）
路径布局跟真实后台完全对齐，只是 host:port 不同，方便本地联调时随时切换。

## 4. 接口一：版本检查 `GET {baseUrl}/version`

只返回元数据，不含完整指令列表——用于"有没有更新"这类低成本轮询，不必每次
都拉几十 KB 的完整数据。

**请求**：`GET {baseUrl}/version?app=ubibot-serial-assistant&appVersion=0.2.2.1`

**响应**：

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

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `version` | string | 是 | 服务端当前权威版本号。 |
| `publishedAt` | string (ISO 8601) | 否 | 这个版本的发布时间，仅展示用。 |
| `minAppVersion` | string | 否 | 这份数据要求的最低 App 版本。客户端如果发现自己的 `AppController.appVersion` 低于这个值，即使 `version` 不同也不提示"可下载"，而是提示"请先升级 App 本体"（见第 6 节）。省略表示不限制。 |
| `modelCount` / `commandCount` | number | 否 | 纯展示，"有 N 个型号 / M 条指令"这类文案用。 |
| `changelog` | LocalizedText（`{zh,en}`） | 否 | 更新说明，按当前界面语言取一种展示在"检查更新"结果里。省略则不展示更新说明。 |

客户端逻辑：`version` 与本地当前生效版本（`AppController.libraryVersion`，即
上次成功应用的版本，或没下载过时编译期打包的版本）逐字符比较，不同即视为
"有更新"。

## 5. 接口二：拉取最新完整数据 `GET {baseUrl}/latest`

返回完整的设备/指令列表，**顶层结构和 `resources/devices.json` 完全一样**
（只是多包了几个元数据字段，`models[]` 内部 schema 见
[device-json-protocol-schema.md](device-json-protocol-schema.md)，不变）：

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

因为 `DeviceLibrary::loadFromJsonText()` 本来就只按字段名读 `version` 和
`models`，`ok`/`publishedAt` 这类多出来的顶层字段会被直接忽略——服务端完全
可以就是把自己维护的那份 `devices.json` 原样返回，外面套一层 `ok`/
`publishedAt`，不需要为这个接口单独维护一份"客户端专用格式"。

### 完整性校验：`X-Content-SHA256` 响应头

服务端在响应头里加一行：

```
X-Content-SHA256: 9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08
```

值是**整个响应体原始字节**（不是某个字段、不是格式化后的 JSON）的 SHA-256
十六进制串。客户端收到响应后，先对拿到的原始字节做同样的 SHA-256，和这个响应
头比对，一致才解析、缓存、应用；不一致视为"下载数据可能损坏"，报错并保留
当前已生效的版本不动。这个头**可选**——服务端如果暂时没做，客户端只是跳过
校验，不强制要求。

（不校验单独字段而校验整个原始字节，是为了避免"服务端和客户端各自的 JSON
序列化实现是否字节对齐"这类跨语言canonical JSON 的麻烦——两边操作的是完全
同一段字节，SHA-256 自然一致，不需要担心 key 顺序、转义、浮点数格式化这些
差异。）

## 6. 客户端应用流程

1. **启动时**：`AppController` 构造时优先加载
   `SettingsStore::cachedLibraryJson()`（上次成功下载并应用的完整 JSON 文本）；
   为空或解析失败则回退到编译期打包的 `resources/devices.json`
   （`DeviceLibrary::loadFromResource()`）。不会在启动时自动发起网络请求。
2. **"检查更新"**（`AppController::checkForLibraryUpdate()`）：调用接口一，
   把 `version`/`changelog`/是否有更新 反映到 `libraryUpdateAvailable`、
   `remoteLibraryVersion`、`libraryUpdateMessage` 这几个属性上，供
   `SettingsAboutDialog.qml` 绑定展示。若 `minAppVersion` 高于本地
   `appVersion`，即使版本号不同也不置 `libraryUpdateAvailable = true`，而是
   把 `libraryUpdateMessage` 设为"需要先升级 App 到 X.X.X.X"这类提示。
3. **"下载并应用"**（`AppController::downloadLibraryUpdate()`，仅在
   `libraryUpdateAvailable` 为真时才在 UI 上可点）：调用接口二，校验通过后：
   - `DeviceLibrary::loadFromJsonText(rawJson)` 就地重新解析，替换掉内存里
     的整份型号/指令数据；
   - `SettingsStore::setCachedLibraryJson(rawJson)` 持久化，供下次启动直接用；
   - `CommandListModel::reload()` 重建当前显示的指令行；
   - `AppController` 发出 `libraryChanged()`，`modelIds`/`libraryVersion`/
     `modelCount`/`commandCount` 这几个属性据此在 QML 里自动刷新（不需要
     重启 App）。
4. 全程不自动重试、不后台静默轮询——两个动作都由用户在 Settings & About
   面板里手动触发，失败了就是给一句能读懂的错误文案，用户自己决定要不要
   再点一次。

## 7. 错误场景一览

| 场景 | 客户端表现 |
|---|---|
| `.env` 未配置/为空 | 两个方法都直接返回"未配置更新服务器"，不发请求 |
| 网络超时/连不上 | 展示 `QNetworkReply::errorString()`，保留当前已生效的数据不动 |
| 响应不是合法 JSON / 缺 `models` | 展示"响应格式错误"，保留当前已生效的数据不动 |
| `X-Content-SHA256` 校验不通过 | 展示"数据校验失败，可能已损坏"，**不应用**这次下载 |
| `minAppVersion` 高于本地 App 版本 | 有新版本号但不允许"下载并应用"，提示先升级 App 本体 |

## 8. 待确认事项（不影响先开工）

1. 接口二要不要支持 `?since=<version>` 做增量返回（当前设计里服务端始终
   返回全量 `models[]`，型号/指令量级预计在几百条以内，全量传输的体积和
   解析成本都可以忽略，暂不做增量)。
2. 要不要给"下载并应用"加一个本地"回滚到上一次缓存版本"的按钮——当前设计
   里 `SettingsStore` 只保留"最近一次成功应用"的一份缓存，没有历史版本，
   出问题时用户目前唯一的退路是等服务端修好后再次"下载并应用"覆盖。
