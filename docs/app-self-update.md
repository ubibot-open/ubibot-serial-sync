# 软件自更新 · 设计说明

状态：**已实现**。本文档记录设计取舍和已知局限，供以后维护/排查参考——和
[device-library-update-protocol.md](device-library-update-protocol.md)（设备
指令库那套远程更新）是两个独立功能，这个更新的是 **App 自己**
（`UbiBotSerialAssistant.exe` 本身），不是设备指令数据。

## 1. 总览

菜单 `帮助 (&Help) → Check for software update` 触发
`AppController.checkForAppUpdate()`，弹出 `SoftwareUpdateDialog.qml`。有新版本
时用户点「Update now」，走完整下载 → 校验 → 解压 → 退出 App → 用一个内置生成的
批处理脚本把新文件覆盖到安装目录 → 重新拉起 App 的流程，全程不需要用户去浏览器
手动下载。

两个新类分工明确：
- [SoftwareUpdateClient](../src/core/software_update_client.h)——只管网络：查
  版本、下载 zip。
- [SelfUpdateInstaller](../src/core/self_update_installer.h)——只管本地文件系统：
  解压、校验、生成并启动那个负责"覆盖安装目录"的批处理脚本。纯静态函数，不
  依赖 Qt 的信号槽，因为每一步都是同步的本地操作。

`AppController` 把两者串起来，暴露 `appUpdateState`/`appUpdateMessage`/
`remoteAppVersion`/`appUpdateAvailable`/`appUpdateForced`/`appUpdateProgress`
这几个属性给 `SoftwareUpdateDialog.qml` 绑定，和设备指令库那套
`libraryUpdateState` 系列属性是完全同构的设计（特意用不同的属性名/菜单文案，
避免和"检查指令库更新"混淆）。

## 2. 复用现有的 Software/Version 系统，而不是新写一套协议

`ubibot-appcenter` 后台本来就有一套通用的"软件/版本"分发系统
（`softwares`/`versions` 两张表，`App\Http\Controllers\Admin\
SoftwareController::api_list()`，公开路由 `GET /api/software/list`），本来是
给其他 UbiBot 产品用的，`admin-react` 里也已经有现成的上传界面（版本号、
更新说明、平台、强制更新开关都能填，上传 zip/exe 直接拿到下载链接）。这次直接
复用它，而不是像设备指令库那样另起一套协议，理由：

- 后台管理界面是现成的——运维发新版本，直接去 `admin-react` 的"软件版本管理"
  页面传文件、填版本号，不需要我们再给这个功能单独做一套上传/管理界面。
- 唯一要做的一次性接入工作，是 `ubibot-appcenter` 仓库（另一个仓库，和这个不在
  同一个目录树下）`server/database/migrations/` 里新增的迁移
  `2026_08_25_010000_register_ubibot_serial_assistant_software.php`，把这个
  App 注册成 `softwares` 表里 `slug = "ubibot-serial-assistant"` 的一行——
  **这个迁移需要手动跑一次 `php artisan migrate`**（没有自动帮跑，因为那是
  一个跑在共享远程数据库上的迁移，不该由脚本代劳）。

**响应不是裸数组**：`SoftwareController` 用的是 `larke-admin` 框架自带的
`ResponseJson` trait（`$this->success('success', $software)`），所以实际响应
是这套框架统一的信封格式 `{"success": bool, "code": int, "message": string,
"data": [...]}`，产品列表本身在 `data` 字段里，不是顶层数组——这是实测
`curl` 出来确认的（最初照探索报告里"返回数组"的描述直接假设成了裸数组，编译
能过，第一次真的对着本地起的 Laravel 实例测才发现解析错了，已改正）。

复用还带来两个已知局限，都在客户端做了防御性处理：

1. **`/software/list` 的 `product`/`serial` 查询参数验证了但没实际拿来过滤**
   ——接口会把*所有*注册过的软件产品都返回回来（各自带上按 `os` 过滤出的最新
   版本），不是只返回我们自己的。`SoftwareUpdateClient::checkForUpdate()` 里
   自己按 `slug == "ubibot-serial-assistant"` 在返回数组里找到属于自己的那条,
   找不到就报"这个 App 还没在更新服务器上注册"，不会误把别的产品的版本信息
   当成自己的。
2. **`versions.sha256` 字段有，但整条链路（`SoftwareVersionController`、
   `admin-react` 的上传表单）都没有任何地方会去写它**——目前实际拿到的永远是
   空值。`SoftwareUpdateClient`/`SelfUpdateInstaller` 依然把校验逻辑写好了
   （`sha256` 非空时才校验，校验不过直接中止、不解压不覆盖），完全是"有则校验，
   无则跳过"，等哪天后台真的开始填这个字段，客户端不用改代码就自动生效。

## 3. 版本比较 / 强制更新 / 最低版本

和设备指令库更新那套的 `minAppVersion` 处理是同一个思路：

- `version.version`（服务端）与 `AppController.appVersion`（本地，来自
  `CMakeLists.txt` 的 `PROJECT_VERSION`）都用 `QVersionNumber` 比较，不是
  简单的字符串不等——避免 "1.10.0" 被字典序错误地判定成比 "1.9.0" 旧这种问题。
- `version.min_required_version`：如果服务端这个新版本要求"必须从某个版本以后
  才能直接升级"，而本地版本比这个还旧，就不提供"Update now"，改成提示"请先
  升级到某版本"（和设备指令库的 `minAppVersion` 门槛判断逻辑完全对称，但这里
  语义反过来——`minAppVersion` 是"这份数据要求的最低 App 版本"，
  `min_required_version` 是"能直接跳到这个新版本的最低起始版本"，两者字段名
  容易搞混，注释里已经写清楚）。
- `version.is_force_update`：为真时 `SoftwareUpdateDialog.qml` 隐藏「Later」
  按钮、关掉点击遮罩/Esc 关闭（`closePolicy: Popup.NoAutoClose`），逼用户走完
  更新流程。**当前版本没有做"App 启动时自动检查"**——只有用户手动点了 Help 菜单
  才会触发检查，所以"强制更新"目前只在用户主动检查时才会生效，不会在后台默默
  拦住正在使用的用户。要不要加开机自动检查，是一个明显的后续可选项，先没做。

## 4. 打包格式：zip 整个目录 + `tar` 解压，没有引入新工具链

现状：客户端发布是纯手工流程（见 [README.md](../README.md)）——
`windeployqt` 生成一个 `deploy\UbiBotSerialAssistant\` 文件夹（exe + Qt 动态库
+ qml/翻译资源，~110MB），运维自己压缩成 zip 分发。仓库里没有任何安装包生成
工具链（Inno Setup / NSIS 都没有），也没有 CI。这次特意**没有**为了这个功能
去新增安装包工具链，而是直接在现有"zip 整个目录"的基础上做自更新：

- 下载下来的就是这个 zip；`SelfUpdateInstaller::extractAndValidate()` 用
  Windows 10 1803+/11 自带的 `tar.exe`（微软从那个版本开始内置了支持 zip 的
  bsdtar）解压到一个临时目录，不需要额外依赖任何第三方解压库，也不用为了
  "解压一个 zip" 这么小的需求把 Qt 的 zip 支持模块引进来。
- 解压后先做健全性检查：临时目录顶层（或恰好深一层，兼容"压缩了整个 deploy
  文件夹"和"压缩了文件夹里的内容"两种打包习惯）必须能找到
  `UbiBotSerialAssistant.exe`，找不到就直接报错、不往下走——防止服务器返回的
  不是真正的发布包（比如一个 HTML 错误页被存成了 .zip）。
- 校验通过后才会：把 App 自己退出、拉起一个自动生成的 `.bat` 脚本、脚本负责
  等 App 真正退出、`robocopy /MIR` 把临时目录镜像覆盖到安装目录、重新拉起 App、
  清理临时文件、自我删除。

**为什么是批处理脚本而不是真正的安装包**：运行中的 App 自己没法覆盖/删除自己
正在使用的 exe/dll（Windows 文件锁），标准做法要么是"下载安装包、退出、静默
运行安装包"（更稳妥，但要新增一整套安装包工具链和相关的运维流程改动），要么是
"下载压缩包、退出、用一个独立的小进程做文件替换"（不需要新工具链，但机制本身
更 hacky，出问题时的兜底手段更弱）。这次选的是后者——见下一节的已知局限。

## 5. `.env` 保护：`robocopy /XF ".env"`

安装目录里如果放了 `.env`（设备指令库更新用的那个，见
[device-library-update-protocol.md](device-library-update-protocol.md)），
`robocopy /MIR` 默认会把它当成"目标里多出来的文件"直接删掉——因为下载下来的
发布包里天然不会带这个文件。`SelfUpdateInstaller::writeUpdateScript()` 生成的
脚本显式加了 `/XF ".env"`，专门排除掉这一个文件，其余镜像行为不变（含删除
上一版本里已经不存在的旧 DLL/文件）。

## 6. 已知局限（有意不在这次实现，供以后参考）

1. **假设安装目录本身可写，不处理需要管理员权限的安装位置**——现在的部署方式
   是"随便复制这个文件夹到哪都行"，一般不会装在 `Program Files` 这类需要提权
   的目录下；真要支持装在那种位置，`robocopy` 那一步会因为权限不足静默失败
   （批处理脚本没有任何机制能把失败反馈回已经退出的 App），需要额外做 UAC
   提权。
2. **没有失败回滚**——`extractAndValidate()` 失败会在 App 还在运行、安装目录
   完全没被动的前提下直接报错退出流程，这部分是安全的；但一旦批处理脚本真的
   开始跑 `robocopy /MIR`，如果这一步中途被打断（比如系统突然断电），安装目录
   可能停在一半新一半旧的状态，没有自动检测/修复机制，只能让用户重新走一次
   "Check for software update" 覆盖回完整版本。
3. **批处理窗口会短暂闪一下**——`cmd.exe /C script.bat` 会开一个控制台窗口，
   直到脚本自我删除退出前都可见，纯粹是观感上的瑕疵，不影响功能。
4. **没有 App 启动时自动检查**——见第 3 节末尾，纯粹是当前范围没做，不是做不到。
5. **`sha256` 校验目前形同虚设**——见第 2 节，字段本身没人填，等后台补上这块
   再验证客户端这段代码是否真的按预期工作。
