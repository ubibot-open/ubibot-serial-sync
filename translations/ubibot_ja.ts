<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja" sourcelanguage="en">
<context>
    <name>AppController</name>
    <message>
        <source>%1 OPEN</source>
        <translation>%1 オープン</translation>
    </message>
    <message>
        <source>%1 CLOSED</source>
        <translation>%1 クローズ</translation>
    </message>
    <message>
        <source>Port is not open — click &quot;Open port&quot; first.</source>
        <translation>ポートが開いていません。先に「ポートを開く」をクリックしてください。</translation>
    </message>
    <message>
        <source>Select a serial port first.</source>
        <translation>先にシリアルポートを選択してください。</translation>
    </message>
    <message>
        <source>Command library is up to date (%1).</source>
        <translation>コマンドライブラリは最新です（%1）。</translation>
    </message>
    <message>
        <source>Version %1 is available, but requires app version %2 or later -- please update the app first.</source>
        <translation>バージョン %1 が利用可能ですが、アプリのバージョン %2 以降が必要です。先にアプリを更新してください。</translation>
    </message>
    <message>
        <source>Version %1 is available.</source>
        <translation>バージョン %1 が利用可能です。</translation>
    </message>
    <message>
        <source>Downloaded data could not be applied: %1</source>
        <translation>ダウンロードしたデータを適用できませんでした：%1</translation>
    </message>
    <message>
        <source>Updated to %1.</source>
        <translation>%1 に更新しました。</translation>
    </message>
    <message>
        <source>You&apos;re on the latest version (%1).</source>
        <translation>最新バージョンです（%1）。</translation>
    </message>
    <message>
        <source>The send box isn&apos;t valid HEX, so it was left unchanged.</source>
        <translation>送信欄の内容が有効な HEX ではないため、変更していません。</translation>
    </message>
    <message>
        <source>The HEX data contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>HEX データにテキストとして表示できないバイトが含まれているため、変更していません。</translation>
    </message>
    <message>
        <source>%1 opened · %2</source>
        <translation>%1 を開きました · %2</translation>
    </message>
    <message>
        <source>Port closed</source>
        <translation>ポートを閉じました</translation>
    </message>
    <message>
        <source>Not sent: invalid HEX. Use pairs of 0-9 / A-F digits, e.g. &quot;7B 22 0D 0A&quot;.</source>
        <translation>送信しませんでした：HEX が無効です。0-9 / A-F の 2 桁ずつで入力してください（例：&quot;7B 22 0D 0A&quot;）。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; started — %2 step(s).</source>
        <translation>バッチ「%1」を開始しました — %2 ステップ。</translation>
    </message>
    <message>
        <source>Batch step %1 skipped: invalid HEX &quot;%2&quot;.</source>
        <translation>バッチのステップ %1 をスキップしました：無効な HEX &quot;%2&quot;。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; finished.</source>
        <translation>バッチ「%1」が完了しました。</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; stopped.</source>
        <translation>バッチ「%1」を停止しました。</translation>
    </message>
    <message>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Log saved to %1</source>
        <translation>ログを %1 に保存しました</translation>
    </message>
    <message>
        <source>No update server configured (.env) -- using the bundled command library only.</source>
        <translation>更新サーバーが設定されていません（.env）。同梱のコマンドライブラリのみを使用します。</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>更新を確認しています…</translation>
    </message>
    <message>
        <source>Downloading…</source>
        <translation>ダウンロードしています…</translation>
    </message>
    <message>
        <source>No update source configured.</source>
        <translation>更新元が設定されていません。</translation>
    </message>
    <message>
        <source>Automatic installation isn&apos;t available on this platform. Opening the download page…</source>
        <translation>このプラットフォームでは自動インストールを利用できません。ダウンロードページを開きます…</translation>
    </message>
    <message>
        <source>No release page available.</source>
        <translation>リリースページを取得できません。</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>更新をダウンロードしています…</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>ダウンロードをキャンセルしました。</translation>
    </message>
    <message>
        <source>Downloaded file failed checksum verification -- update not applied.</source>
        <translation>ダウンロードしたファイルのチェックサム検証に失敗しました。更新は適用されていません。</translation>
    </message>
    <message>
        <source>Preparing update…</source>
        <translation>更新を準備しています…</translation>
    </message>
    <message>
        <source>Failed to launch the updater script.</source>
        <translation>アップデーター スクリプトを起動できませんでした。</translation>
    </message>
    <message>
        <source>Restarting to apply update…</source>
        <translation>更新を適用するために再起動しています…</translation>
    </message>
    <message>
        <source>Self-update isn&apos;t supported on this platform yet -- please download the new version manually.</source>
        <translation>このプラットフォームではまだ自動更新に対応していません。新しいバージョンを手動でダウンロードしてください。</translation>
    </message>
</context>
<context>
    <name>DeviceLibraryUpdateClient</name>
    <message>
        <source>Remote update server not configured (.env).</source>
        <translation>リモート更新サーバーが設定されていません（.env）。</translation>
    </message>
    <message>
        <source>Malformed response: %1</source>
        <translation>不正な応答：%1</translation>
    </message>
    <message>
        <source>Checksum mismatch -- downloaded data may be corrupted.</source>
        <translation>チェックサムが一致しません。ダウンロードしたデータが破損している可能性があります。</translation>
    </message>
    <message>
        <source>Response has no models[] array.</source>
        <translation>応答に models[] 配列がありません。</translation>
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
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Restore</source>
        <translation>元に戻す</translation>
    </message>
</context>
<context>
    <name>QLight::Dialog</name>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
</context>
<context>
    <name>QLight::InfoBar</name>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
</context>
<context>
    <name>QLight::TeachingTip</name>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>could not start &apos;tar&apos; (bundled with Windows 10 1803+/Windows 11) -- %1</source>
        <translation>&apos;tar&apos; を起動できませんでした（Windows 10 1803 以降/Windows 11 に付属）：%1</translation>
    </message>
    <message>
        <source>&apos;tar&apos; timed out extracting the update</source>
        <translation>&apos;tar&apos; による更新の展開がタイムアウトしました</translation>
    </message>
    <message>
        <source>&apos;tar&apos; failed (exit %1): %2</source>
        <translation>&apos;tar&apos; が失敗しました（終了コード %1）：%2</translation>
    </message>
    <message>
        <source>downloaded archive doesn&apos;t contain %1 -- not a valid release package</source>
        <translation>ダウンロードしたアーカイブに %1 が含まれていません。有効なリリース パッケージではありません</translation>
    </message>
    <message>
        <source>could not write updater script to %1: %2</source>
        <translation>アップデーター スクリプトを %1 に書き込めませんでした：%2</translation>
    </message>
</context>
<context>
    <name>SerialManager</name>
    <message>
        <source>Serial port disconnected.</source>
        <translation>シリアルポートが切断されました。</translation>
    </message>
</context>
<context>
    <name>SerialOptions</name>
    <message>
        <source>None</source>
        <translation>なし</translation>
    </message>
    <message>
        <source>Even</source>
        <translation>偶数</translation>
    </message>
    <message>
        <source>Odd</source>
        <translation>奇数</translation>
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
        <translation>更新元が設定されていません。</translation>
    </message>
    <message>
        <source>GitHub refused the request: %1</source>
        <translation>GitHub がリクエストを拒否しました: %1</translation>
    </message>
    <message>
        <source>No published release was found.</source>
        <translation>公開されているリリースが見つかりません。</translation>
    </message>
    <message>
        <source>Malformed GitHub response: %1</source>
        <translation>GitHub の応答形式が不正です: %1</translation>
    </message>
    <message>
        <source>Release tag &quot;%1&quot; is not a version number.</source>
        <translation>リリースタグ「%1」はバージョン番号ではありません。</translation>
    </message>
    <message>
        <source>Release %1 has no package for this platform.</source>
        <translation>リリース %1 にはこのプラットフォーム向けのパッケージがありません。</translation>
    </message>
    <message>
        <source>Release %1 has no %2 package.</source>
        <translation>リリース %1 には %2 パッケージがありません。</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>%1 に書き込めません：%2</translation>
    </message>
</context>
<context>
    <name>ui::AboutDialog</name>
    <message>
        <source>About</source>
        <translation>バージョン情報</translation>
    </message>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot シリアルアシスタント</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>バージョン %1</translation>
    </message>
    <message>
        <source>A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6 and C++17.</source>
        <translation>UbiBot IoT デバイス（WS1、WS1 Pro、GS1-AL4G1RS、SP1 など）向けのシリアルポート デバッグ ツールです。Qt 6 と C++17 で開発されています。</translation>
    </message>
    <message>
        <source>Build time</source>
        <translation>ビルド日時</translation>
    </message>
    <message>
        <source>Company</source>
        <translation>会社</translation>
    </message>
    <message>
        <source>UbiBot · United States of America</source>
        <translation>UbiBot · アメリカ合衆国</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>ウェブサイト</translation>
    </message>
    <message>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
    <message>
        <source>Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.</source>
        <translation>Qt %1（GNU Lesser General Public License v3、LGPLv3）を使用して構築されています。Qt は The Qt Company Ltd. の商標です。Qt は動的リンクされているため、同梱の Qt ライブラリは互換性のある LGPLv3 ビルドに置き換えることができます。</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
</context>
<context>
    <name>ui::BatchEditDialog</name>
    <message>
        <source>Edit batch command</source>
        <translation>バッチコマンドを編集</translation>
    </message>
    <message>
        <source>New batch command</source>
        <translation>新規バッチコマンド</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <source>e.g. Startup sequence</source>
        <translation>例：起動シーケンス</translation>
    </message>
    <message>
        <source>Interval</source>
        <translation>間隔</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>ミリ秒</translation>
    </message>
    <message>
        <source>Steps</source>
        <translation>ステップ</translation>
    </message>
    <message>
        <source>Add step</source>
        <translation>ステップを追加</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <source>Command text</source>
        <translation>コマンド内容</translation>
    </message>
    <message>
        <source>CRC</source>
        <translation>CRC</translation>
    </message>
    <message>
        <source>Remove step</source>
        <translation>ステップを削除</translation>
    </message>
    <message>
        <source>Not valid HEX, so it was left unchanged.</source>
        <translation>有効な HEX ではないため、変更していません。</translation>
    </message>
    <message>
        <source>Contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>テキストとして表示できないバイトを含むため、変更していません。</translation>
    </message>
</context>
<context>
    <name>ui::BatchListDialog</name>
    <message>
        <source>Batch commands</source>
        <translation>バッチコマンド</translation>
    </message>
    <message>
        <source>No batch commands yet</source>
        <translation>バッチコマンドはまだありません</translation>
    </message>
    <message>
        <source>New batch</source>
        <translation>新規バッチ</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>%1 steps · %2 ms interval</source>
        <translation>%1 ステップ · 間隔 %2 ms</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>実行</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>削除</translation>
    </message>
</context>
<context>
    <name>ui::CommandLibraryPanel</name>
    <message>
        <source>Device model</source>
        <translation>デバイスモデル</translation>
    </message>
    <message>
        <source>Search commands</source>
        <translation>コマンドを検索</translation>
    </message>
    <message>
        <source>Drag a row to reorder</source>
        <translation>行をドラッグして並べ替え</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新しいテンプレート</translation>
    </message>
    <message>
        <source>Batch commands</source>
        <translation>バッチコマンド</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>削除</translation>
    </message>
</context>
<context>
    <name>ui::CommandParamsPanel</name>
    <message>
        <source>Needs parameters</source>
        <translation>パラメーターが必要です</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>プレビュー</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Insert</source>
        <translation>挿入</translation>
    </message>
</context>
<context>
    <name>ui::DataMonitorView</name>
    <message>
        <source>Data monitor</source>
        <translation>データモニター</translation>
    </message>
    <message>
        <source>Data sent and received appears here once a port is open</source>
        <translation>ポートを開くと、送受信したデータがここに表示されます</translation>
    </message>
    <message>
        <source>%1 lines</source>
        <translation>%1 行</translation>
    </message>
    <message>
        <source>Rx %1</source>
        <translation>受信 %1</translation>
    </message>
    <message>
        <source>Tx %1</source>
        <translation>送信 %1</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation>コピー</translation>
    </message>
    <message>
        <source>Clear log</source>
        <translation>ログをクリア</translation>
    </message>
    <message>
        <source>Save log…</source>
        <translation>ログを保存…</translation>
    </message>
</context>
<context>
    <name>ui::DriverHintDialog</name>
    <message>
        <source>Serial driver may be missing</source>
        <translation>シリアルドライバーが見つからない可能性があります</translation>
    </message>
    <message>
        <source>The CH340 USB-to-serial driver doesn&apos;t appear to be installed on this computer. Without it, a connected UbiBot device won&apos;t show up as an available serial port.</source>
        <translation>このコンピューターには CH340 USB-シリアル変換ドライバーがインストールされていないようです。このドライバーがないと、接続した UbiBot デバイスは利用可能なシリアルポートとして表示されません。</translation>
    </message>
    <message>
        <source>Don&apos;t show this again</source>
        <translation>今後表示しない</translation>
    </message>
    <message>
        <source>Download driver</source>
        <translation>ドライバーをダウンロード</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
</context>
<context>
    <name>ui::DriverListDialog</name>
    <message>
        <source>USB-to-serial driver used by most UbiBot devices</source>
        <translation>ほとんどの UbiBot デバイスで使用される USB-シリアル変換ドライバー</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>ドライバーのダウンロード</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>No drivers are relevant on this platform.</source>
        <translation>このプラットフォームに関連するドライバーはありません。</translation>
    </message>
    <message>
        <source>Installed</source>
        <translation>インストール済み</translation>
    </message>
    <message>
        <source>Not detected</source>
        <translation>未検出</translation>
    </message>
    <message>
        <source>Download</source>
        <translation>ダウンロード</translation>
    </message>
</context>
<context>
    <name>ui::LibraryUpdateDialog</name>
    <message>
        <source>Command library</source>
        <translation>コマンドライブラリ</translation>
    </message>
    <message>
        <source>Download and apply</source>
        <translation>ダウンロードして適用</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>ui::MainView</name>
    <message>
        <source>Failed to open port</source>
        <translation>ポートを開けませんでした</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>ファイル(&amp;F)</translation>
    </message>
    <message>
        <source>Save log</source>
        <translation>ログを保存</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>編集(&amp;E)</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>クリア</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>表示(&amp;V)</translation>
    </message>
    <message>
        <source>Pause scrolling</source>
        <translation>スクロールを一時停止</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation>ツール(&amp;T)</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>ヘルプ(&amp;H)</translation>
    </message>
    <message>
        <source>Check for software update</source>
        <translation>ソフトウェアの更新を確認</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>ドライバーのダウンロード</translation>
    </message>
    <message>
        <source>About</source>
        <translation>バージョン情報</translation>
    </message>
    <message>
        <source>Serial driver not installed, so devices may not be detected.</source>
        <translation>シリアル ドライバーがインストールされていないため、デバイスが認識されない可能性があります。</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>詳細を表示</translation>
    </message>
    <message>
        <source>Serial</source>
        <translation>シリアル</translation>
    </message>
    <message>
        <source>Device commands</source>
        <translation>デバイスコマンド</translation>
    </message>
    <message>
        <source>Type data to send…</source>
        <translation>送信するデータを入力…</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>送信</translation>
    </message>
    <message>
        <source>History</source>
        <translation>履歴</translation>
    </message>
    <message>
        <source>Connected · %1</source>
        <translation>接続済み · %1</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>未接続</translation>
    </message>
    <message>
        <source>Send history</source>
        <translation>送信履歴</translation>
    </message>
    <message>
        <source>Clear history</source>
        <translation>履歴をクリア</translation>
    </message>
    <message>
        <source>No history yet</source>
        <translation>履歴はまだありません</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>UbiBot シリアルアシスタント</translation>
    </message>
</context>
<context>
    <name>ui::SaveLogDialog</name>
    <message>
        <source>Choose a folder</source>
        <translation>フォルダーを選択</translation>
    </message>
    <message>
        <source>Save session log</source>
        <translation>セッションログを保存</translation>
    </message>
    <message>
        <source>File name</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>保存先</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>形式</translation>
    </message>
    <message>
        <source>Browse…</source>
        <translation>参照…</translation>
    </message>
    <message>
        <source>Plain text (.log)</source>
        <translation>プレーンテキスト (.log)</translation>
    </message>
    <message>
        <source>HEX dump</source>
        <translation>HEX ダンプ</translation>
    </message>
    <message>
        <source>Continue logging to disk, rotating the file every day</source>
        <translation>毎日ファイルを切り替えながらディスクへの記録を継続</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
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
        <translation>ポート設定</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>ポート</translation>
    </message>
    <message>
        <source>Baud rate</source>
        <translation>ボーレート</translation>
    </message>
    <message>
        <source>Data bits</source>
        <translation>データビット</translation>
    </message>
    <message>
        <source>Parity</source>
        <translation>パリティ</translation>
    </message>
    <message>
        <source>Stop bits</source>
        <translation>ストップビット</translation>
    </message>
    <message>
        <source>Flow control</source>
        <translation>フロー制御</translation>
    </message>
    <message>
        <source>No ports found</source>
        <translation>ポートが見つかりません</translation>
    </message>
    <message>
        <source>Refresh ports</source>
        <translation>ポートを更新</translation>
    </message>
    <message>
        <source>Receive</source>
        <translation>受信</translation>
    </message>
    <message>
        <source>Show timestamp</source>
        <translation>タイムスタンプを表示</translation>
    </message>
    <message>
        <source>Wrap lines</source>
        <translation>折り返し</translation>
    </message>
    <message>
        <source>Echo sent data</source>
        <translation>送信データをエコー表示</translation>
    </message>
    <message>
        <source>Transmit</source>
        <translation>送信</translation>
    </message>
    <message>
        <source>Append CRC (CRC16/MODBUS)</source>
        <translation>CRC を付加 (CRC16/MODBUS)</translation>
    </message>
    <message>
        <source>Repeat send</source>
        <translation>繰り返し送信</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>ミリ秒</translation>
    </message>
    <message>
        <source>Unknown device</source>
        <translation>不明なデバイス</translation>
    </message>
    <message>
        <source>Close port</source>
        <translation>ポートを閉じる</translation>
    </message>
    <message>
        <source>Open port</source>
        <translation>ポートを開く</translation>
    </message>
</context>
<context>
    <name>ui::SettingsDialog</name>
    <message>
        <source>Settings &amp; About</source>
        <translation>設定とバージョン情報</translation>
    </message>
    <message>
        <source>Interface language</source>
        <translation>表示言語</translation>
    </message>
    <message>
        <source>System font</source>
        <translation>システムフォント</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>サイズ</translation>
    </message>
    <message>
        <source>Data monitor font</source>
        <translation>データモニターフォント</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>テーマ</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>ライト</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>ダーク</translation>
    </message>
    <message>
        <source>Command library</source>
        <translation>コマンドライブラリ</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translation>更新を確認</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>バージョン</translation>
    </message>
    <message>
        <source>Platform</source>
        <translation>プラットフォーム</translation>
    </message>
    <message>
        <source>Support</source>
        <translation>サポート</translation>
    </message>
    <message>
        <source>Devices</source>
        <translation>対応デバイス</translation>
    </message>
    <message>
        <source>Restore defaults</source>
        <translation>既定値に戻す</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>%1 (Qt %2)</source>
        <translation>%1（Qt %2）</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translation>最新です</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>更新があります</translation>
    </message>
    <message>
        <source>%1 models · %2 commands</source>
        <translation>%1 モデル · %2 コマンド</translation>
    </message>
</context>
<context>
    <name>ui::SoftwareUpdateDialog</name>
    <message>
        <source>Software Update</source>
        <translation>ソフトウェア更新</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>後で</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>再試行</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>今すぐ更新</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Version %1 is available (you have %2)</source>
        <translation>バージョン %1 が利用可能です（現在は %2）</translation>
    </message>
</context>
<context>
    <name>ui::TemplateDialog</name>
    <message>
        <source>Edit template</source>
        <translation>テンプレートを編集</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>新しいテンプレート</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <source>e.g. Reset device</source>
        <translation>例：デバイスをリセット</translation>
    </message>
    <message>
        <source>Content</source>
        <translation>内容</translation>
    </message>
    <message>
        <source>The literal text to send</source>
        <translation>送信する実際のテキスト</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
</TS>
