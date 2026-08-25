# 设备指令库 · 模拟服务端

`docs/device-library-update-protocol.md` 描述的两个接口的一个最小实现，仅用于本地联调/演示——**不做任何鉴权**，数据直接从 `data/devices.json` + `data/meta.json` 读取（每次请求都重新读盘，改完文件直接点客户端的「Check for updates」就能看到效果，不用重启这个服务）。

## 运行

```bash
cd server
go run . -addr :8980
```

（`-addr`/`-data` 都是可选 flag，默认 `:8980` 和 `./data`。）

## 接入客户端

在仓库根目录建一个 `.env`（gitignore 了，不会被提交，见 [.env.example](../.env.example)）：

```
DEVICE_LIBRARY_API_BASE_URL=http://127.0.0.1:8980/api/device-library
DEVICE_LIBRARY_API_KEY=
```

把它放到打包产物同目录（`deploy/UbiBotSerialAssistant/.env`）或者项目当前工作目录（开发时直接在构建目录里跑），下次「Check for updates」就会打到这个模拟服务端。

路径里的 `/api/device-library` 是刻意和真实后台（`ubibot-appcenter` 仓库 `server/routes/api.php` 里注册的 `App\Http\Controllers\DeviceLibraryController`）对齐的——两边只是 host:port 不同，路径结构完全一样，`.env` 切换起来不用改别的。

## 数据文件

- `data/devices.json` —— 和 [resources/devices.json](../resources/devices.json) 同一套 schema（见 [docs/device-json-protocol-schema.md](../docs/device-json-protocol-schema.md)），`/latest` 原样把 `models` 透传回去。仓库里预置的这份把 `version` 改成了 `lib-2026.08.25`（比客户端编译期打包的 `lib-2026.08.12` 新），并多加了一条 `reboot` 指令，方便一启动就能看到「有更新」的效果。
- `data/meta.json` —— `/version` 接口里那几个 `devices.json` 本身没有的字段：`publishedAt`、`minAppVersion`、`changelog`（本地化更新说明）。缺失时对应字段就是空值，不影响接口正常返回。

## 两个接口

- `GET /api/device-library/version` —— 元数据（`version`/`publishedAt`/`minAppVersion`/`modelCount`/`commandCount`/`changelog`）。
- `GET /api/device-library/latest` —— 完整数据（`{ok, version, publishedAt, models}`），响应头带 `X-Content-SHA256`（整个响应体的 SHA-256），客户端会校验。

两个接口的完整字段说明、错误信封格式见 [docs/device-library-update-protocol.md](../docs/device-library-update-protocol.md)。
