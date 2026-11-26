<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en">
<context>
    <name>AppController</name>
    <message>
        <source>%1 OPEN</source>
        <translation>%1 已打开</translation>
    </message>
    <message>
        <source>%1 CLOSED</source>
        <translation>%1 已关闭</translation>
    </message>
    <message>
        <source>Port is not open — click &quot;Open port&quot; first.</source>
        <translation>串口未打开，请先点击「打开串口」。</translation>
    </message>
    <message>
        <source>Select a serial port first.</source>
        <translation>请先选择一个串口。</translation>
    </message>
    <message>
        <source>Command library is up to date (%1).</source>
        <translation>指令库已是最新（%1）。</translation>
    </message>
    <message>
        <source>Version %1 is available, but requires app version %2 or later -- please update the app first.</source>
        <translation>版本 %1 可用，但需要应用版本 %2 或更高，请先更新应用。</translation>
    </message>
    <message>
        <source>Version %1 is available.</source>
        <translation>版本 %1 可用。</translation>
    </message>
    <message>
        <source>Downloaded data could not be applied: %1</source>
        <translation>下载的数据无法应用：%1</translation>
    </message>
    <message>
        <source>Updated to %1.</source>
        <translation>已更新到 %1。</translation>
    </message>
    <message>
        <source>You&apos;re on the latest version (%1).</source>
        <translation>已是最新版本（%1）。</translation>
    </message>
    <message>
        <source>The send box isn&apos;t valid HEX, so it was left unchanged.</source>
        <translation>发送框内容不是有效的 HEX，已保持原样。</translation>
    </message>
    <message>
        <source>The HEX data contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>HEX 数据中含有无法显示为文本的字节，已保持原样。</translation>
    </message>
    <message>
        <source>%1 opened · %2</source>
        <translation>%1 已打开 · %2</translation>
    </message>
    <message>
        <source>Port closed</source>
        <translation>串口已关闭</translation>
    </message>
    <message>
        <source>Not sent: invalid HEX. Use pairs of 0-9 / A-F digits, e.g. &quot;7B 22 0D 0A&quot;.</source>
        <translation>未发送：HEX 格式无效。请使用成对的 0-9 / A-F 字符，例如 &quot;7B 22 0D 0A&quot;。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; started — %2 step(s).</source>
        <translation>批量指令「%1」已开始 — 共 %2 步。</translation>
    </message>
    <message>
        <source>Batch step %1 skipped: invalid HEX &quot;%2&quot;.</source>
        <translation>批量步骤 %1 已跳过：HEX 无效 &quot;%2&quot;。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; finished.</source>
        <translation>批量指令「%1」已完成。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; stopped.</source>
        <translation>批量指令「%1」已停止。</translation>
    </message>
    <message>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Log saved to %1</source>
        <translation>日志已保存到 %1</translation>
    </message>
    <message>
        <source>No update server configured (.env) -- using the bundled command library only.</source>
        <translation>未配置更新服务器（.env），仅使用内置指令库。</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>正在检查更新…</translation>
    </message>
    <message>
        <source>Downloading…</source>
        <translation>正在下载…</translation>
    </message>
    <message>
        <source>No update source configured.</source>
        <translation>未配置更新源。</translation>
    </message>
    <message>
        <source>Automatic installation isn&apos;t available on this platform. Opening the download page…</source>
        <translation>此平台不支持自动安装。正在打开下载页面…</translation>
    </message>
    <message>
        <source>No release page available.</source>
        <translation>无法获取发布页面。</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>正在下载更新…</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>下载已取消。</translation>
    </message>
    <message>
        <source>Downloaded file failed checksum verification -- update not applied.</source>
        <translation>下载的文件校验失败，未应用更新。</translation>
    </message>
    <message>
        <source>Preparing update…</source>
        <translation>正在准备更新…</translation>
    </message>
    <message>
        <source>Failed to launch the updater script.</source>
        <translation>无法启动更新脚本。</translation>
    </message>
    <message>
        <source>Restarting to apply update…</source>
        <translation>正在重启以应用更新…</translation>
    </message>
    <message>
        <source>Self-update isn&apos;t supported on this platform yet -- please download the new version manually.</source>
        <translation>此平台暂不支持自动更新，请手动下载新版本。</translation>
    </message>
</context>
<context>
    <name>DeviceLibraryUpdateClient</name>
    <message>
        <source>Remote update server not configured (.env).</source>
        <translation>未配置远程更新服务器（.env）。</translation>
    </message>
    <message>
        <source>Malformed response: %1</source>
        <translation>响应格式错误：%1</translation>
    </message>
    <message>
        <source>Checksum mismatch -- downloaded data may be corrupted.</source>
        <translation>校验和不匹配，下载的数据可能已损坏。</translation>
    </message>
    <message>
        <source>Response has no models[] array.</source>
        <translation>响应中缺少 models[] 数组。</translation>
    </message>
</context>
<context>
    <name>QLight::CaptionButton</name>
    <message>
        <source>Minimize</source>
        <translation>最小化</translation>
    </message>
    <message>
        <source>Maximize</source>
        <translation>最大化</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Restore</source>
        <translation>还原</translation>
    </message>
</context>
<context>
    <name>QLight::Dialog</name>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>QLight::InfoBar</name>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>QLight::TeachingTip</name>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>could not start &apos;tar&apos; (bundled with Windows 10 1803+/Windows 11) -- %1</source>
        <translation>无法启动 &apos;tar&apos;（Windows 10 1803+/Windows 11 自带）：%1</translation>
    </message>
    <message>
        <source>&apos;tar&apos; timed out extracting the update</source>
        <translation>&apos;tar&apos; 解压更新超时</translation>
    </message>
    <message>
        <source>&apos;tar&apos; failed (exit %1): %2</source>
        <translation>&apos;tar&apos; 执行失败（退出码 %1）：%2</translation>
    </message>
    <message>
        <source>downloaded archive doesn&apos;t contain %1 -- not a valid release package</source>
        <translation>下载的压缩包不包含 %1，不是有效的发布包</translation>
    </message>
    <message>
        <source>could not write updater script to %1: %2</source>
        <translation>无法写入更新脚本到 %1：%2</translation>
    </message>
</context>
<context>
    <name>SerialManager</name>
    <message>
        <source>Serial port disconnected.</source>
        <translation>串口已断开连接。</translation>
    </message>
</context>
<context>
    <name>SerialOptions</name>
    <message>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <source>Even</source>
        <translation>偶校验</translation>
    </message>
    <message>
        <source>Odd</source>
        <translation>奇校验</translation>
    </message>
    <message>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
</context>
<context>
    <name>SoftwareUpdateClient</name>
    <message>
        <source>No update source configured.</source>
        <translation>未配置更新源。</translation>
    </message>
    <message>
        <source>GitHub refused the request: %1</source>
        <translation>GitHub 拒绝了请求：%1</translation>
    </message>
    <message>
        <source>No published release was found.</source>
        <translation>没有找到已发布的版本。</translation>
    </message>
    <message>
        <source>Malformed GitHub response: %1</source>
        <translation>GitHub 响应格式错误：%1</translation>
    </message>
    <message>
        <source>Release tag &quot;%1&quot; is not a version number.</source>
        <translation>发布标签「%1」不是版本号。</translation>
    </message>
    <message>
        <source>Release %1 has no package for this platform.</source>
        <translation>版本 %1 没有适用于当前平台的安装包。</translation>
    </message>
    <message>
        <source>Release %1 has no %2 package.</source>
        <translation>版本 %1 没有 %2 安装包。</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>无法写入 %1：%2</translation>
    </message>
</context>
<context>
    <name>ui::AboutDialog</name>
    <message>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot 串口助手</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <source>A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6 and C++17.</source>
        <translation>一款面向 UbiBot 物联网设备（WS1、WS1 Pro、GS1-AL4G1RS、SP1 等）的串口调试工具，基于 Qt 6 和 C++17 开发。</translation>
    </message>
    <message>
        <source>Build time</source>
        <translation>构建时间</translation>
    </message>
    <message>
        <source>Company</source>
        <translation>公司</translation>
    </message>
    <message>
        <source>UbiBot · United States of America</source>
        <translation>UbiBot · 美国</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>网站</translation>
    </message>
    <message>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
    <message>
        <source>Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.</source>
        <translation>基于 Qt %1 构建，遵循 GNU Lesser General Public License v3（LGPLv3）。Qt 是 The Qt Company Ltd. 的注册商标。本程序以动态链接方式使用 Qt，随附的 Qt 库可替换为兼容的 LGPLv3 版本。</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>ui::BatchEditDialog</name>
    <message>
        <source>Edit batch command</source>
        <translation>编辑批量指令</translation>
    </message>
    <message>
        <source>New batch command</source>
        <translation>新建批量指令</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <source>e.g. Startup sequence</source>
        <translation>例如：启动流程</translation>
    </message>
    <message>
        <source>Interval</source>
        <translation>间隔</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>毫秒</translation>
    </message>
    <message>
        <source>Steps</source>
        <translation>步骤</translation>
    </message>
    <message>
        <source>Add step</source>
        <translation>添加步骤</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <source>Command text</source>
        <translation>指令内容</translation>
    </message>
    <message>
        <source>CRC</source>
        <translation>CRC</translation>
    </message>
    <message>
        <source>Remove step</source>
        <translation>删除步骤</translation>
    </message>
    <message>
        <source>Not valid HEX, so it was left unchanged.</source>
        <translation>不是有效的 HEX，已保持原样。</translation>
    </message>
    <message>
        <source>Contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>含有无法显示为文本的字节，已保持原样。</translation>
    </message>
</context>
<context>
    <name>ui::BatchListDialog</name>
    <message>
        <source>Batch commands</source>
        <translation>批量指令</translation>
    </message>
    <message>
        <source>No batch commands yet</source>
        <translation>暂无批量指令</translation>
    </message>
    <message>
        <source>New batch</source>
        <translation>新建批量</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>%1 steps · %2 ms interval</source>
        <translation>%1 步 · 间隔 %2 毫秒</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>运行</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
</context>
<context>
    <name>ui::CommandLibraryPanel</name>
    <message>
        <source>Device model</source>
        <translation>设备型号</translation>
    </message>
    <message>
        <source>Search commands</source>
        <translation>搜索指令</translation>
    </message>
    <message>
        <source>Drag a row to reorder</source>
        <translation>可拖拽行调整顺序</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新建模板</translation>
    </message>
    <message>
        <source>Batch commands</source>
        <translation>批量指令</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
</context>
<context>
    <name>ui::CommandParamsPanel</name>
    <message>
        <source>Needs parameters</source>
        <translation>需要参数</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Insert</source>
        <translation>填入</translation>
    </message>
</context>
<context>
    <name>ui::DataMonitorView</name>
    <message>
        <source>Data monitor</source>
        <translation>数据监视</translation>
    </message>
    <message>
        <source>Data sent and received appears here once a port is open</source>
        <translation>打开串口后，收发的数据将显示在这里</translation>
    </message>
    <message>
        <source>%1 lines</source>
        <translation>%1 行</translation>
    </message>
    <message>
        <source>Rx %1</source>
        <translation>接收 %1</translation>
    </message>
    <message>
        <source>Tx %1</source>
        <translation>发送 %1</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <source>Clear log</source>
        <translation>清空记录</translation>
    </message>
    <message>
        <source>Save log…</source>
        <translation>保存记录…</translation>
    </message>
</context>
<context>
    <name>ui::DriverHintDialog</name>
    <message>
        <source>Serial driver may be missing</source>
        <translation>可能缺少串口驱动</translation>
    </message>
    <message>
        <source>The CH340 USB-to-serial driver doesn&apos;t appear to be installed on this computer. Without it, a connected UbiBot device won&apos;t show up as an available serial port.</source>
        <translation>本机似乎没有安装 CH340 USB 转串口驱动。没有这个驱动，已连接的 UbiBot 设备不会显示为可用串口。</translation>
    </message>
    <message>
        <source>Don&apos;t show this again</source>
        <translation>不再提示</translation>
    </message>
    <message>
        <source>Download driver</source>
        <translation>下载驱动</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>ui::DriverListDialog</name>
    <message>
        <source>USB-to-serial driver used by most UbiBot devices</source>
        <translation>大多数 UbiBot 设备使用的 USB 转串口驱动</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>驱动下载</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>No drivers are relevant on this platform.</source>
        <translation>此平台没有相关驱动。</translation>
    </message>
    <message>
        <source>Installed</source>
        <translation>已安装</translation>
    </message>
    <message>
        <source>Not detected</source>
        <translation>未检测到</translation>
    </message>
    <message>
        <source>Download</source>
        <translation>下载</translation>
    </message>
</context>
<context>
    <name>ui::LibraryUpdateDialog</name>
    <message>
        <source>Command library</source>
        <translation>指令库</translation>
    </message>
    <message>
        <source>Download and apply</source>
        <translation>下载并应用</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
</context>
<context>
    <name>ui::MainView</name>
    <message>
        <source>Failed to open port</source>
        <translation>打开串口失败</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <source>Save log</source>
        <translation>保存记录</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>退出</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>编辑(&amp;E)</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>清空</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>视图(&amp;V)</translation>
    </message>
    <message>
        <source>Pause scrolling</source>
        <translation>暂停滚动</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation>工具(&amp;T)</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <source>Check for software update</source>
        <translation>检查软件更新</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>驱动下载</translation>
    </message>
    <message>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <source>Serial driver not installed, so devices may not be detected.</source>
        <translation>串口驱动未安装，设备可能无法被识别。</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>查看详情</translation>
    </message>
    <message>
        <source>Serial</source>
        <translation>串口</translation>
    </message>
    <message>
        <source>Device commands</source>
        <translation>设备指令</translation>
    </message>
    <message>
        <source>Type data to send…</source>
        <translation>输入要发送的数据…</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <source>History</source>
        <translation>历史记录</translation>
    </message>
    <message>
        <source>Connected · %1</source>
        <translation>已连接 · %1</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <source>Send history</source>
        <translation>发送历史</translation>
    </message>
    <message>
        <source>Clear history</source>
        <translation>清空历史</translation>
    </message>
    <message>
        <source>No history yet</source>
        <translation>暂无历史记录</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot 串口助手</translation>
    </message>
</context>
<context>
    <name>ui::SaveLogDialog</name>
    <message>
        <source>Choose a folder</source>
        <translation>选择文件夹</translation>
    </message>
    <message>
        <source>Save session log</source>
        <translation>保存日志</translation>
    </message>
    <message>
        <source>File name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>保存位置</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>格式</translation>
    </message>
    <message>
        <source>Browse…</source>
        <translation>浏览…</translation>
    </message>
    <message>
        <source>Plain text (.log)</source>
        <translation>纯文本 (.log)</translation>
    </message>
    <message>
        <source>HEX dump</source>
        <translation>HEX 转储</translation>
    </message>
    <message>
        <source>Continue logging to disk, rotating the file every day</source>
        <translation>自动按天分割并持续保存到磁盘</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ui::SerialSettingsPanel</name>
    <message>
        <source>Port settings</source>
        <translation>串口设置</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <source>Baud rate</source>
        <translation>波特率</translation>
    </message>
    <message>
        <source>Data bits</source>
        <translation>数据位</translation>
    </message>
    <message>
        <source>Parity</source>
        <translation>校验位</translation>
    </message>
    <message>
        <source>Stop bits</source>
        <translation>停止位</translation>
    </message>
    <message>
        <source>Flow control</source>
        <translation>流控</translation>
    </message>
    <message>
        <source>No ports found</source>
        <translation>未找到端口</translation>
    </message>
    <message>
        <source>Refresh ports</source>
        <translation>刷新串口</translation>
    </message>
    <message>
        <source>Receive</source>
        <translation>接收</translation>
    </message>
    <message>
        <source>Show timestamp</source>
        <translation>显示时间戳</translation>
    </message>
    <message>
        <source>Wrap lines</source>
        <translation>自动换行</translation>
    </message>
    <message>
        <source>Echo sent data</source>
        <translation>显示发送内容</translation>
    </message>
    <message>
        <source>Transmit</source>
        <translation>发送</translation>
    </message>
    <message>
        <source>Append CRC (CRC16/MODBUS)</source>
        <translation>附加 CRC 校验（CRC16/MODBUS）</translation>
    </message>
    <message>
        <source>Repeat send</source>
        <translation>重复发送</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>毫秒</translation>
    </message>
    <message>
        <source>Unknown device</source>
        <translation>未知设备</translation>
    </message>
    <message>
        <source>Close port</source>
        <translation>关闭串口</translation>
    </message>
    <message>
        <source>Open port</source>
        <translation>打开串口</translation>
    </message>
</context>
<context>
    <name>ui::SettingsDialog</name>
    <message>
        <source>Settings &amp; About</source>
        <translation>设置与关于</translation>
    </message>
    <message>
        <source>Interface language</source>
        <translation>界面语言</translation>
    </message>
    <message>
        <source>System font</source>
        <translation>系统字体</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>字号</translation>
    </message>
    <message>
        <source>Data monitor font</source>
        <translation>数据监视字体</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>主题</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>浅色</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>深色</translation>
    </message>
    <message>
        <source>Command library</source>
        <translation>指令库</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>版本</translation>
    </message>
    <message>
        <source>Platform</source>
        <translation>平台</translation>
    </message>
    <message>
        <source>Support</source>
        <translation>支持</translation>
    </message>
    <message>
        <source>Devices</source>
        <translation>设备库</translation>
    </message>
    <message>
        <source>Restore defaults</source>
        <translation>恢复默认值</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>%1 (Qt %2)</source>
        <translation>%1（Qt %2）</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translation>已是最新</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>有可用更新</translation>
    </message>
    <message>
        <source>%1 models · %2 commands</source>
        <translation>%1 个型号 · %2 条指令</translation>
    </message>
</context>
<context>
    <name>ui::SoftwareUpdateDialog</name>
    <message>
        <source>Software Update</source>
        <translation>软件更新</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>稍后</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>重试</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>立即更新</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <source>Version %1 is available (you have %2)</source>
        <translation>版本 %1 可用（当前为 %2）</translation>
    </message>
</context>
<context>
    <name>ui::TemplateDialog</name>
    <message>
        <source>Edit template</source>
        <translation>编辑模板</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新建模板</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <source>e.g. Reset device</source>
        <translation>例如：重启设备</translation>
    </message>
    <message>
        <source>Content</source>
        <translation>内容</translation>
    </message>
    <message>
        <source>The literal text to send</source>
        <translation>要发送的原始文本</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
</TS>
