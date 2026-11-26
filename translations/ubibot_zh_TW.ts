<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW" sourcelanguage="en">
<context>
    <name>AppController</name>
    <message>
        <source>%1 OPEN</source>
        <translation>%1 已開啟</translation>
    </message>
    <message>
        <source>%1 CLOSED</source>
        <translation>%1 已關閉</translation>
    </message>
    <message>
        <source>Port is not open — click &quot;Open port&quot; first.</source>
        <translation>序列埠尚未開啟，請先點擊「開啟序列埠」。</translation>
    </message>
    <message>
        <source>Select a serial port first.</source>
        <translation>請先選擇一個序列埠。</translation>
    </message>
    <message>
        <source>Command library is up to date (%1).</source>
        <translation>指令庫已是最新（%1）。</translation>
    </message>
    <message>
        <source>Version %1 is available, but requires app version %2 or later -- please update the app first.</source>
        <translation>版本 %1 可用，但需要應用程式版本 %2 或更高，請先更新應用程式。</translation>
    </message>
    <message>
        <source>Version %1 is available.</source>
        <translation>版本 %1 可用。</translation>
    </message>
    <message>
        <source>Downloaded data could not be applied: %1</source>
        <translation>下載的資料無法套用：%1</translation>
    </message>
    <message>
        <source>Updated to %1.</source>
        <translation>已更新至 %1。</translation>
    </message>
    <message>
        <source>You&apos;re on the latest version (%1).</source>
        <translation>已是最新版本（%1）。</translation>
    </message>
    <message>
        <source>The send box isn&apos;t valid HEX, so it was left unchanged.</source>
        <translation>傳送框內容不是有效的 HEX，已保持原樣。</translation>
    </message>
    <message>
        <source>The HEX data contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>HEX 資料中含有無法顯示為文字的位元組，已保持原樣。</translation>
    </message>
    <message>
        <source>%1 opened · %2</source>
        <translation>%1 已開啟 · %2</translation>
    </message>
    <message>
        <source>Port closed</source>
        <translation>序列埠已關閉</translation>
    </message>
    <message>
        <source>Not sent: invalid HEX. Use pairs of 0-9 / A-F digits, e.g. &quot;7B 22 0D 0A&quot;.</source>
        <translation>未傳送：HEX 格式無效。請使用成對的 0-9 / A-F 字元，例如 &quot;7B 22 0D 0A&quot;。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; started — %2 step(s).</source>
        <translation>批次指令「%1」已開始 — 共 %2 步驟。</translation>
    </message>
    <message>
        <source>Batch step %1 skipped: invalid HEX &quot;%2&quot;.</source>
        <translation>批次步驟 %1 已略過：HEX 無效 &quot;%2&quot;。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; finished.</source>
        <translation>批次指令「%1」已完成。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; stopped.</source>
        <translation>批次指令「%1」已停止。</translation>
    </message>
    <message>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Log saved to %1</source>
        <translation>記錄已儲存至 %1</translation>
    </message>
    <message>
        <source>No update server configured (.env) -- using the bundled command library only.</source>
        <translation>未設定更新伺服器（.env），僅使用內建指令庫。</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>正在檢查更新…</translation>
    </message>
    <message>
        <source>Downloading…</source>
        <translation>正在下載…</translation>
    </message>
    <message>
        <source>No update source configured.</source>
        <translation>未設定更新來源。</translation>
    </message>
    <message>
        <source>Automatic installation isn&apos;t available on this platform. Opening the download page…</source>
        <translation>此平台不支援自動安裝。正在開啟下載頁面…</translation>
    </message>
    <message>
        <source>No release page available.</source>
        <translation>無法取得發行頁面。</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>正在下載更新…</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>下載已取消。</translation>
    </message>
    <message>
        <source>Downloaded file failed checksum verification -- update not applied.</source>
        <translation>下載的檔案校驗失敗，未套用更新。</translation>
    </message>
    <message>
        <source>Preparing update…</source>
        <translation>正在準備更新…</translation>
    </message>
    <message>
        <source>Failed to launch the updater script.</source>
        <translation>無法啟動更新指令碼。</translation>
    </message>
    <message>
        <source>Restarting to apply update…</source>
        <translation>正在重新啟動以套用更新…</translation>
    </message>
    <message>
        <source>Self-update isn&apos;t supported on this platform yet -- please download the new version manually.</source>
        <translation>此平台暫不支援自動更新，請手動下載新版本。</translation>
    </message>
</context>
<context>
    <name>DeviceLibraryUpdateClient</name>
    <message>
        <source>Remote update server not configured (.env).</source>
        <translation>未設定遠端更新伺服器（.env）。</translation>
    </message>
    <message>
        <source>Malformed response: %1</source>
        <translation>回應格式錯誤：%1</translation>
    </message>
    <message>
        <source>Checksum mismatch -- downloaded data may be corrupted.</source>
        <translation>校驗和不符，下載的資料可能已損毀。</translation>
    </message>
    <message>
        <source>Response has no models[] array.</source>
        <translation>回應中缺少 models[] 陣列。</translation>
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
        <translation>關閉</translation>
    </message>
    <message>
        <source>Restore</source>
        <translation>還原</translation>
    </message>
</context>
<context>
    <name>QLight::Dialog</name>
    <message>
        <source>OK</source>
        <translation>確定</translation>
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
        <translation>關閉</translation>
    </message>
</context>
<context>
    <name>QLight::TeachingTip</name>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>could not start &apos;tar&apos; (bundled with Windows 10 1803+/Windows 11) -- %1</source>
        <translation>無法啟動 &apos;tar&apos;（Windows 10 1803+/Windows 11 內建）：%1</translation>
    </message>
    <message>
        <source>&apos;tar&apos; timed out extracting the update</source>
        <translation>&apos;tar&apos; 解壓更新逾時</translation>
    </message>
    <message>
        <source>&apos;tar&apos; failed (exit %1): %2</source>
        <translation>&apos;tar&apos; 執行失敗（結束碼 %1）：%2</translation>
    </message>
    <message>
        <source>downloaded archive doesn&apos;t contain %1 -- not a valid release package</source>
        <translation>下載的壓縮檔不包含 %1，不是有效的發行套件</translation>
    </message>
    <message>
        <source>could not write updater script to %1: %2</source>
        <translation>無法寫入更新指令碼至 %1：%2</translation>
    </message>
</context>
<context>
    <name>SerialManager</name>
    <message>
        <source>Serial port disconnected.</source>
        <translation>序列埠已中斷連線。</translation>
    </message>
</context>
<context>
    <name>SerialOptions</name>
    <message>
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <source>Even</source>
        <translation>偶校驗</translation>
    </message>
    <message>
        <source>Odd</source>
        <translation>奇校驗</translation>
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
        <translation>未設定更新來源。</translation>
    </message>
    <message>
        <source>GitHub refused the request: %1</source>
        <translation>GitHub 拒絕了請求：%1</translation>
    </message>
    <message>
        <source>No published release was found.</source>
        <translation>找不到已發行的版本。</translation>
    </message>
    <message>
        <source>Malformed GitHub response: %1</source>
        <translation>GitHub 回應格式錯誤：%1</translation>
    </message>
    <message>
        <source>Release tag &quot;%1&quot; is not a version number.</source>
        <translation>發行標籤「%1」不是版本號。</translation>
    </message>
    <message>
        <source>Release %1 has no package for this platform.</source>
        <translation>版本 %1 沒有適用於目前平台的安裝包。</translation>
    </message>
    <message>
        <source>Release %1 has no %2 package.</source>
        <translation>版本 %1 沒有 %2 安裝包。</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>無法寫入 %1：%2</translation>
    </message>
</context>
<context>
    <name>ui::AboutDialog</name>
    <message>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot 序列埠助手</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <source>A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6 and C++17.</source>
        <translation>一款針對 UbiBot 物聯網裝置（WS1、WS1 Pro、GS1-AL4G1RS、SP1 等）的序列埠除錯工具，以 Qt 6 與 C++17 開發。</translation>
    </message>
    <message>
        <source>Build time</source>
        <translation>建置時間</translation>
    </message>
    <message>
        <source>Company</source>
        <translation>公司</translation>
    </message>
    <message>
        <source>UbiBot · United States of America</source>
        <translation>UbiBot・美國</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>網站</translation>
    </message>
    <message>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
    <message>
        <source>Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.</source>
        <translation>採用 Qt %1 建置，遵循 GNU Lesser General Public License v3（LGPLv3）授權條款。Qt 為 The Qt Company Ltd. 之註冊商標。本程式以動態連結方式使用 Qt，隨附的 Qt 函式庫可替換為相容的 LGPLv3 版本。</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
</context>
<context>
    <name>ui::BatchEditDialog</name>
    <message>
        <source>Edit batch command</source>
        <translation>編輯批次指令</translation>
    </message>
    <message>
        <source>New batch command</source>
        <translation>新增批次指令</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名稱</translation>
    </message>
    <message>
        <source>e.g. Startup sequence</source>
        <translation>例如：啟動流程</translation>
    </message>
    <message>
        <source>Interval</source>
        <translation>間隔</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>毫秒</translation>
    </message>
    <message>
        <source>Steps</source>
        <translation>步驟</translation>
    </message>
    <message>
        <source>Add step</source>
        <translation>新增步驟</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
    <message>
        <source>Command text</source>
        <translation>指令內容</translation>
    </message>
    <message>
        <source>CRC</source>
        <translation>CRC</translation>
    </message>
    <message>
        <source>Remove step</source>
        <translation>刪除步驟</translation>
    </message>
    <message>
        <source>Not valid HEX, so it was left unchanged.</source>
        <translation>不是有效的 HEX，已保持原樣。</translation>
    </message>
    <message>
        <source>Contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>含有無法顯示為文字的位元組，已保持原樣。</translation>
    </message>
</context>
<context>
    <name>ui::BatchListDialog</name>
    <message>
        <source>Batch commands</source>
        <translation>批次指令</translation>
    </message>
    <message>
        <source>No batch commands yet</source>
        <translation>尚無批次指令</translation>
    </message>
    <message>
        <source>New batch</source>
        <translation>新增批次</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <source>%1 steps · %2 ms interval</source>
        <translation>%1 步驟 · 間隔 %2 毫秒</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>執行</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>編輯</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>刪除</translation>
    </message>
</context>
<context>
    <name>ui::CommandLibraryPanel</name>
    <message>
        <source>Device model</source>
        <translation>裝置型號</translation>
    </message>
    <message>
        <source>Search commands</source>
        <translation>搜尋指令</translation>
    </message>
    <message>
        <source>Drag a row to reorder</source>
        <translation>可拖曳行調整順序</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新增範本</translation>
    </message>
    <message>
        <source>Batch commands</source>
        <translation>批次指令</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>編輯</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>刪除</translation>
    </message>
</context>
<context>
    <name>ui::CommandParamsPanel</name>
    <message>
        <source>Needs parameters</source>
        <translation>需要參數</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>預覽</translation>
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
        <translation>資料監控</translation>
    </message>
    <message>
        <source>Data sent and received appears here once a port is open</source>
        <translation>開啟序列埠後，收發的資料將顯示在這裡</translation>
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
        <translation>傳送 %1</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <source>Clear log</source>
        <translation>清空記錄</translation>
    </message>
    <message>
        <source>Save log…</source>
        <translation>儲存記錄…</translation>
    </message>
</context>
<context>
    <name>ui::DriverHintDialog</name>
    <message>
        <source>Serial driver may be missing</source>
        <translation>可能缺少序列埠驅動程式</translation>
    </message>
    <message>
        <source>The CH340 USB-to-serial driver doesn&apos;t appear to be installed on this computer. Without it, a connected UbiBot device won&apos;t show up as an available serial port.</source>
        <translation>本機似乎未安裝 CH340 USB 轉序列埠驅動程式。沒有這個驅動程式，已連接的 UbiBot 裝置不會顯示為可用序列埠。</translation>
    </message>
    <message>
        <source>Don&apos;t show this again</source>
        <translation>不再顯示</translation>
    </message>
    <message>
        <source>Download driver</source>
        <translation>下載驅動程式</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
</context>
<context>
    <name>ui::DriverListDialog</name>
    <message>
        <source>USB-to-serial driver used by most UbiBot devices</source>
        <translation>大多數 UbiBot 裝置使用的 USB 轉序列埠驅動程式</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>驅動程式下載</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <source>No drivers are relevant on this platform.</source>
        <translation>此平台沒有相關驅動程式。</translation>
    </message>
    <message>
        <source>Installed</source>
        <translation>已安裝</translation>
    </message>
    <message>
        <source>Not detected</source>
        <translation>未偵測到</translation>
    </message>
    <message>
        <source>Download</source>
        <translation>下載</translation>
    </message>
</context>
<context>
    <name>ui::LibraryUpdateDialog</name>
    <message>
        <source>Command library</source>
        <translation>指令庫</translation>
    </message>
    <message>
        <source>Download and apply</source>
        <translation>下載並套用</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>確定</translation>
    </message>
</context>
<context>
    <name>ui::MainView</name>
    <message>
        <source>Failed to open port</source>
        <translation>開啟序列埠失敗</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>檔案(&amp;F)</translation>
    </message>
    <message>
        <source>Save log</source>
        <translation>儲存記錄</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>結束</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>編輯(&amp;E)</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>清空</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>檢視(&amp;V)</translation>
    </message>
    <message>
        <source>Pause scrolling</source>
        <translation>暫停捲動</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation>工具(&amp;T)</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>說明(&amp;H)</translation>
    </message>
    <message>
        <source>Check for software update</source>
        <translation>檢查軟體更新</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>驅動程式下載</translation>
    </message>
    <message>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <source>Serial driver not installed, so devices may not be detected.</source>
        <translation>序列埠驅動程式未安裝，裝置可能無法被識別。</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>查看詳情</translation>
    </message>
    <message>
        <source>Serial</source>
        <translation>序列埠</translation>
    </message>
    <message>
        <source>Device commands</source>
        <translation>裝置指令</translation>
    </message>
    <message>
        <source>Type data to send…</source>
        <translation>輸入要傳送的資料…</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>傳送</translation>
    </message>
    <message>
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <source>Connected · %1</source>
        <translation>已連線 · %1</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>尚未連線</translation>
    </message>
    <message>
        <source>Send history</source>
        <translation>傳送歷史</translation>
    </message>
    <message>
        <source>Clear history</source>
        <translation>清空歷史記錄</translation>
    </message>
    <message>
        <source>No history yet</source>
        <translation>尚無歷史記錄</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot 序列埠助手</translation>
    </message>
</context>
<context>
    <name>ui::SaveLogDialog</name>
    <message>
        <source>Choose a folder</source>
        <translation>選擇資料夾</translation>
    </message>
    <message>
        <source>Save session log</source>
        <translation>儲存記錄</translation>
    </message>
    <message>
        <source>File name</source>
        <translation>檔案名稱</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>儲存位置</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>格式</translation>
    </message>
    <message>
        <source>Browse…</source>
        <translation>瀏覽…</translation>
    </message>
    <message>
        <source>Plain text (.log)</source>
        <translation>純文字 (.log)</translation>
    </message>
    <message>
        <source>HEX dump</source>
        <translation>HEX 傾印</translation>
    </message>
    <message>
        <source>Continue logging to disk, rotating the file every day</source>
        <translation>自動依日期分割並持續儲存至磁碟</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ui::SerialSettingsPanel</name>
    <message>
        <source>Port settings</source>
        <translation>序列埠設定</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>連接埠</translation>
    </message>
    <message>
        <source>Baud rate</source>
        <translation>鮑率</translation>
    </message>
    <message>
        <source>Data bits</source>
        <translation>資料位元</translation>
    </message>
    <message>
        <source>Parity</source>
        <translation>校驗位元</translation>
    </message>
    <message>
        <source>Stop bits</source>
        <translation>停止位元</translation>
    </message>
    <message>
        <source>Flow control</source>
        <translation>流量控制</translation>
    </message>
    <message>
        <source>No ports found</source>
        <translation>找不到連接埠</translation>
    </message>
    <message>
        <source>Refresh ports</source>
        <translation>重新整理序列埠</translation>
    </message>
    <message>
        <source>Receive</source>
        <translation>接收</translation>
    </message>
    <message>
        <source>Show timestamp</source>
        <translation>顯示時間戳記</translation>
    </message>
    <message>
        <source>Wrap lines</source>
        <translation>自動換行</translation>
    </message>
    <message>
        <source>Echo sent data</source>
        <translation>顯示傳送內容</translation>
    </message>
    <message>
        <source>Transmit</source>
        <translation>傳送</translation>
    </message>
    <message>
        <source>Append CRC (CRC16/MODBUS)</source>
        <translation>附加 CRC 校驗（CRC16/MODBUS）</translation>
    </message>
    <message>
        <source>Repeat send</source>
        <translation>重複傳送</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>毫秒</translation>
    </message>
    <message>
        <source>Unknown device</source>
        <translation>未知裝置</translation>
    </message>
    <message>
        <source>Close port</source>
        <translation>關閉序列埠</translation>
    </message>
    <message>
        <source>Open port</source>
        <translation>開啟序列埠</translation>
    </message>
</context>
<context>
    <name>ui::SettingsDialog</name>
    <message>
        <source>Settings &amp; About</source>
        <translation>設定與關於</translation>
    </message>
    <message>
        <source>Interface language</source>
        <translation>介面語言</translation>
    </message>
    <message>
        <source>System font</source>
        <translation>系統字型</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>字型大小</translation>
    </message>
    <message>
        <source>Data monitor font</source>
        <translation>資料監控字型</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>主題</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>淺色</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>深色</translation>
    </message>
    <message>
        <source>Command library</source>
        <translation>指令庫</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translation>檢查更新</translation>
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
        <translation>支援</translation>
    </message>
    <message>
        <source>Devices</source>
        <translation>支援裝置</translation>
    </message>
    <message>
        <source>Restore defaults</source>
        <translation>還原預設值</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <source>%1 (Qt %2)</source>
        <translation>%1（Qt %2）</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translation>已是最新版本</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>有可用更新</translation>
    </message>
    <message>
        <source>%1 models · %2 commands</source>
        <translation>%1 個型號・%2 筆指令</translation>
    </message>
</context>
<context>
    <name>ui::SoftwareUpdateDialog</name>
    <message>
        <source>Software Update</source>
        <translation>軟體更新</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>稍後</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>重試</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>立即更新</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <source>Version %1 is available (you have %2)</source>
        <translation>版本 %1 可用（目前為 %2）</translation>
    </message>
</context>
<context>
    <name>ui::TemplateDialog</name>
    <message>
        <source>Edit template</source>
        <translation>編輯範本</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新增範本</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名稱</translation>
    </message>
    <message>
        <source>e.g. Reset device</source>
        <translation>例如：重新啟動裝置</translation>
    </message>
    <message>
        <source>Content</source>
        <translation>內容</translation>
    </message>
    <message>
        <source>The literal text to send</source>
        <translation>要傳送的原始文字</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
</TS>
