<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
<context>
    <name>AppController</name>
    <message>
        <source>%1 OPEN</source>
        <translation>%1 ОТКРЫТ</translation>
    </message>
    <message>
        <source>%1 CLOSED</source>
        <translation>%1 ЗАКРЫТ</translation>
    </message>
    <message>
        <source>Port is not open — click &quot;Open port&quot; first.</source>
        <translation>Порт не открыт — сначала нажмите «Открыть порт».</translation>
    </message>
    <message>
        <source>Select a serial port first.</source>
        <translation>Сначала выберите последовательный порт.</translation>
    </message>
    <message>
        <source>Command library is up to date (%1).</source>
        <translation>Библиотека команд актуальна (%1).</translation>
    </message>
    <message>
        <source>Version %1 is available, but requires app version %2 or later -- please update the app first.</source>
        <translation>Доступна версия %1, но требуется приложение версии %2 или новее — сначала обновите приложение.</translation>
    </message>
    <message>
        <source>Version %1 is available.</source>
        <translation>Доступна версия %1.</translation>
    </message>
    <message>
        <source>Downloaded data could not be applied: %1</source>
        <translation>Не удалось применить загруженные данные: %1</translation>
    </message>
    <message>
        <source>Updated to %1.</source>
        <translation>Обновлено до %1.</translation>
    </message>
    <message>
        <source>You&apos;re on the latest version (%1).</source>
        <translation>У вас последняя версия (%1).</translation>
    </message>
    <message>
        <source>The send box isn&apos;t valid HEX, so it was left unchanged.</source>
        <translation>Содержимое поля отправки не является корректным HEX, оно оставлено без изменений.</translation>
    </message>
    <message>
        <source>The HEX data contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>Данные HEX содержат байты, которые нельзя показать как текст, поэтому они оставлены без изменений.</translation>
    </message>
    <message>
        <source>%1 opened · %2</source>
        <translation>%1 открыт · %2</translation>
    </message>
    <message>
        <source>Port closed</source>
        <translation>Порт закрыт</translation>
    </message>
    <message>
        <source>Not sent: invalid HEX. Use pairs of 0-9 / A-F digits, e.g. &quot;7B 22 0D 0A&quot;.</source>
        <translation>Не отправлено: неверный HEX. Используйте пары цифр 0-9 / A-F, например &quot;7B 22 0D 0A&quot;.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; started — %2 step(s).</source>
        <translation>Пакет «%1» запущен — %2 шаг(ов).</translation>
    </message>
    <message>
        <source>Batch step %1 skipped: invalid HEX &quot;%2&quot;.</source>
        <translation>Шаг пакета %1 пропущен: неверный HEX &quot;%2&quot;.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; finished.</source>
        <translation>Пакет «%1» завершён.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; stopped.</source>
        <translation>Пакет «%1» остановлен.</translation>
    </message>
    <message>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Log saved to %1</source>
        <translation>Журнал сохранён в %1</translation>
    </message>
    <message>
        <source>No update server configured (.env) -- using the bundled command library only.</source>
        <translation>Сервер обновлений не настроен (.env) — используется только встроенная библиотека команд.</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>Проверка обновлений…</translation>
    </message>
    <message>
        <source>Downloading…</source>
        <translation>Загрузка…</translation>
    </message>
    <message>
        <source>No update source configured.</source>
        <translation>Источник обновлений не настроен.</translation>
    </message>
    <message>
        <source>Automatic installation isn&apos;t available on this platform. Opening the download page…</source>
        <translation>На этой платформе автоматическая установка недоступна. Открывается страница загрузки…</translation>
    </message>
    <message>
        <source>No release page available.</source>
        <translation>Страница выпуска недоступна.</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>Загрузка обновления…</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>Загрузка отменена.</translation>
    </message>
    <message>
        <source>Downloaded file failed checksum verification -- update not applied.</source>
        <translation>Загруженный файл не прошёл проверку контрольной суммы — обновление не применено.</translation>
    </message>
    <message>
        <source>Preparing update…</source>
        <translation>Подготовка обновления…</translation>
    </message>
    <message>
        <source>Failed to launch the updater script.</source>
        <translation>Не удалось запустить сценарий обновления.</translation>
    </message>
    <message>
        <source>Restarting to apply update…</source>
        <translation>Перезапуск для применения обновления…</translation>
    </message>
    <message>
        <source>Self-update isn&apos;t supported on this platform yet -- please download the new version manually.</source>
        <translation>Автообновление на этой платформе пока не поддерживается — загрузите новую версию вручную.</translation>
    </message>
</context>
<context>
    <name>DeviceLibraryUpdateClient</name>
    <message>
        <source>Remote update server not configured (.env).</source>
        <translation>Удалённый сервер обновлений не настроен (.env).</translation>
    </message>
    <message>
        <source>Malformed response: %1</source>
        <translation>Некорректный ответ: %1</translation>
    </message>
    <message>
        <source>Checksum mismatch -- downloaded data may be corrupted.</source>
        <translation>Несовпадение контрольной суммы — загруженные данные могут быть повреждены.</translation>
    </message>
    <message>
        <source>Response has no models[] array.</source>
        <translation>В ответе нет массива models[].</translation>
    </message>
</context>
<context>
    <name>QLight::CaptionButton</name>
    <message>
        <source>Minimize</source>
        <translation>Свернуть</translation>
    </message>
    <message>
        <source>Maximize</source>
        <translation>Развернуть</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Restore</source>
        <translation>Восстановить</translation>
    </message>
</context>
<context>
    <name>QLight::Dialog</name>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
</context>
<context>
    <name>QLight::InfoBar</name>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QLight::TeachingTip</name>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>could not start &apos;tar&apos; (bundled with Windows 10 1803+/Windows 11) -- %1</source>
        <translation>не удалось запустить &apos;tar&apos; (входит в Windows 10 1803+/Windows 11) — %1</translation>
    </message>
    <message>
        <source>&apos;tar&apos; timed out extracting the update</source>
        <translation>&apos;tar&apos; не успел распаковать обновление</translation>
    </message>
    <message>
        <source>&apos;tar&apos; failed (exit %1): %2</source>
        <translation>ошибка &apos;tar&apos; (код %1): %2</translation>
    </message>
    <message>
        <source>downloaded archive doesn&apos;t contain %1 -- not a valid release package</source>
        <translation>загруженный архив не содержит %1 — это не пакет выпуска</translation>
    </message>
    <message>
        <source>could not write updater script to %1: %2</source>
        <translation>не удалось записать сценарий обновления в %1: %2</translation>
    </message>
</context>
<context>
    <name>SerialManager</name>
    <message>
        <source>Serial port disconnected.</source>
        <translation>Последовательный порт отключён.</translation>
    </message>
</context>
<context>
    <name>SerialOptions</name>
    <message>
        <source>None</source>
        <translation>Нет</translation>
    </message>
    <message>
        <source>Even</source>
        <translation>Чётный</translation>
    </message>
    <message>
        <source>Odd</source>
        <translation>Нечётный</translation>
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
        <translation>Источник обновлений не настроен.</translation>
    </message>
    <message>
        <source>GitHub refused the request: %1</source>
        <translation>GitHub отклонил запрос: %1</translation>
    </message>
    <message>
        <source>No published release was found.</source>
        <translation>Опубликованных выпусков не найдено.</translation>
    </message>
    <message>
        <source>Malformed GitHub response: %1</source>
        <translation>Некорректный ответ GitHub: %1</translation>
    </message>
    <message>
        <source>Release tag &quot;%1&quot; is not a version number.</source>
        <translation>Тег выпуска «%1» не является номером версии.</translation>
    </message>
    <message>
        <source>Release %1 has no package for this platform.</source>
        <translation>В выпуске %1 нет пакета для этой платформы.</translation>
    </message>
    <message>
        <source>Release %1 has no %2 package.</source>
        <translation>В выпуске %1 нет пакета %2.</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>Не удаётся записать в %1: %2</translation>
    </message>
</context>
<context>
    <name>ui::AboutDialog</name>
    <message>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>Помощник последовательного порта UbiBot</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>Версия %1</translation>
    </message>
    <message>
        <source>A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6 and C++17.</source>
        <translation>Инструмент отладки последовательного порта для IoT-устройств UbiBot (WS1, WS1 Pro, GS1-AL4G1RS, SP1 и др.), созданный на Qt 6 и C++17.</translation>
    </message>
    <message>
        <source>Build time</source>
        <translation>Время сборки</translation>
    </message>
    <message>
        <source>Company</source>
        <translation>Компания</translation>
    </message>
    <message>
        <source>UbiBot · United States of America</source>
        <translation>UbiBot · США</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>Веб-сайт</translation>
    </message>
    <message>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
    <message>
        <source>Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.</source>
        <translation>Создано с использованием Qt %1 по лицензии GNU Lesser General Public License v3 (LGPLv3). Qt является товарным знаком The Qt Company Ltd. Qt подключается динамически, поэтому входящие в комплект библиотеки Qt можно заменить совместимой сборкой LGPLv3.</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>ui::BatchEditDialog</name>
    <message>
        <source>Edit batch command</source>
        <translation>Изменить пакетную команду</translation>
    </message>
    <message>
        <source>New batch command</source>
        <translation>Новая пакетная команда</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Название</translation>
    </message>
    <message>
        <source>e.g. Startup sequence</source>
        <translation>например: последовательность запуска</translation>
    </message>
    <message>
        <source>Interval</source>
        <translation>Интервал</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>мс</translation>
    </message>
    <message>
        <source>Steps</source>
        <translation>Шаги</translation>
    </message>
    <message>
        <source>Add step</source>
        <translation>Добавить шаг</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <source>Command text</source>
        <translation>Текст команды</translation>
    </message>
    <message>
        <source>CRC</source>
        <translation>CRC</translation>
    </message>
    <message>
        <source>Remove step</source>
        <translation>Удалить шаг</translation>
    </message>
    <message>
        <source>Not valid HEX, so it was left unchanged.</source>
        <translation>Некорректный HEX, оставлено без изменений.</translation>
    </message>
    <message>
        <source>Contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>Содержит байты, которые нельзя показать как текст, оставлено без изменений.</translation>
    </message>
</context>
<context>
    <name>ui::BatchListDialog</name>
    <message>
        <source>Batch commands</source>
        <translation>Пакетные команды</translation>
    </message>
    <message>
        <source>No batch commands yet</source>
        <translation>Пакетных команд пока нет</translation>
    </message>
    <message>
        <source>New batch</source>
        <translation>Новый пакет</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>%1 steps · %2 ms interval</source>
        <translation>%1 шаг(ов) · интервал %2 мс</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Стоп</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Запустить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
</context>
<context>
    <name>ui::CommandLibraryPanel</name>
    <message>
        <source>Device model</source>
        <translation>Модель устройства</translation>
    </message>
    <message>
        <source>Search commands</source>
        <translation>Поиск команд</translation>
    </message>
    <message>
        <source>Drag a row to reorder</source>
        <translation>Перетащите строку для изменения порядка</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>Новый шаблон</translation>
    </message>
    <message>
        <source>Batch commands</source>
        <translation>Пакетные команды</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
</context>
<context>
    <name>ui::CommandParamsPanel</name>
    <message>
        <source>Needs parameters</source>
        <translation>Требуются параметры</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Insert</source>
        <translation>Вставить</translation>
    </message>
</context>
<context>
    <name>ui::DataMonitorView</name>
    <message>
        <source>Data monitor</source>
        <translation>Монитор данных</translation>
    </message>
    <message>
        <source>Data sent and received appears here once a port is open</source>
        <translation>После открытия порта здесь появятся отправленные и полученные данные</translation>
    </message>
    <message>
        <source>%1 lines</source>
        <translation>%1 строк</translation>
    </message>
    <message>
        <source>Rx %1</source>
        <translation>Приём %1</translation>
    </message>
    <message>
        <source>Tx %1</source>
        <translation>Передача %1</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation>Копировать</translation>
    </message>
    <message>
        <source>Clear log</source>
        <translation>Очистить журнал</translation>
    </message>
    <message>
        <source>Save log…</source>
        <translation>Сохранить журнал…</translation>
    </message>
</context>
<context>
    <name>ui::DriverHintDialog</name>
    <message>
        <source>Serial driver may be missing</source>
        <translation>Возможно, отсутствует драйвер порта</translation>
    </message>
    <message>
        <source>The CH340 USB-to-serial driver doesn&apos;t appear to be installed on this computer. Without it, a connected UbiBot device won&apos;t show up as an available serial port.</source>
        <translation>Похоже, на этом компьютере не установлен драйвер CH340 USB-to-serial. Без него подключённое устройство UbiBot не будет отображаться как доступный последовательный порт.</translation>
    </message>
    <message>
        <source>Don&apos;t show this again</source>
        <translation>Больше не показывать</translation>
    </message>
    <message>
        <source>Download driver</source>
        <translation>Скачать драйвер</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>ui::DriverListDialog</name>
    <message>
        <source>USB-to-serial driver used by most UbiBot devices</source>
        <translation>Драйвер USB-to-serial, используемый большинством устройств UbiBot</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>Загрузка драйверов</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>No drivers are relevant on this platform.</source>
        <translation>На этой платформе нет подходящих драйверов.</translation>
    </message>
    <message>
        <source>Installed</source>
        <translation>Установлен</translation>
    </message>
    <message>
        <source>Not detected</source>
        <translation>Не обнаружен</translation>
    </message>
    <message>
        <source>Download</source>
        <translation>Скачать</translation>
    </message>
</context>
<context>
    <name>ui::LibraryUpdateDialog</name>
    <message>
        <source>Command library</source>
        <translation>Библиотека команд</translation>
    </message>
    <message>
        <source>Download and apply</source>
        <translation>Загрузить и применить</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
</context>
<context>
    <name>ui::MainView</name>
    <message>
        <source>Failed to open port</source>
        <translation>Не удалось открыть порт</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <source>Save log</source>
        <translation>Сохранить журнал</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <source>Pause scrolling</source>
        <translation>Приостановить прокрутку</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation>&amp;Инструменты</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>Check for software update</source>
        <translation>Проверить обновления</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>Загрузка драйверов</translation>
    </message>
    <message>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <source>Serial driver not installed, so devices may not be detected.</source>
        <translation>Драйвер последовательного порта не установлен, устройства могут не определяться.</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Подробнее</translation>
    </message>
    <message>
        <source>Serial</source>
        <translation>Последовательный порт</translation>
    </message>
    <message>
        <source>Device commands</source>
        <translation>Команды устройства</translation>
    </message>
    <message>
        <source>Type data to send…</source>
        <translation>Введите данные для отправки…</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Отправить</translation>
    </message>
    <message>
        <source>History</source>
        <translation>История</translation>
    </message>
    <message>
        <source>Connected · %1</source>
        <translation>Подключено · %1</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>Не подключено</translation>
    </message>
    <message>
        <source>Send history</source>
        <translation>История отправки</translation>
    </message>
    <message>
        <source>Clear history</source>
        <translation>Очистить историю</translation>
    </message>
    <message>
        <source>No history yet</source>
        <translation>Пока нет истории</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>Помощник последовательного порта UbiBot</translation>
    </message>
</context>
<context>
    <name>ui::SaveLogDialog</name>
    <message>
        <source>Choose a folder</source>
        <translation>Выберите папку</translation>
    </message>
    <message>
        <source>Save session log</source>
        <translation>Сохранить журнал сессии</translation>
    </message>
    <message>
        <source>File name</source>
        <translation>Имя файла</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Расположение</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <source>Browse…</source>
        <translation>Обзор…</translation>
    </message>
    <message>
        <source>Plain text (.log)</source>
        <translation>Обычный текст (.log)</translation>
    </message>
    <message>
        <source>HEX dump</source>
        <translation>HEX-дамп</translation>
    </message>
    <message>
        <source>Continue logging to disk, rotating the file every day</source>
        <translation>Продолжать запись на диск с ежедневной ротацией файла</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
</context>
<context>
    <name>ui::SerialSettingsPanel</name>
    <message>
        <source>Port settings</source>
        <translation>Настройки порта</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <source>Baud rate</source>
        <translation>Скорость (бод)</translation>
    </message>
    <message>
        <source>Data bits</source>
        <translation>Биты данных</translation>
    </message>
    <message>
        <source>Parity</source>
        <translation>Чётность</translation>
    </message>
    <message>
        <source>Stop bits</source>
        <translation>Стоп-биты</translation>
    </message>
    <message>
        <source>Flow control</source>
        <translation>Управление потоком</translation>
    </message>
    <message>
        <source>No ports found</source>
        <translation>Порты не найдены</translation>
    </message>
    <message>
        <source>Refresh ports</source>
        <translation>Обновить порты</translation>
    </message>
    <message>
        <source>Receive</source>
        <translation>Приём</translation>
    </message>
    <message>
        <source>Show timestamp</source>
        <translation>Показывать время</translation>
    </message>
    <message>
        <source>Wrap lines</source>
        <translation>Перенос строк</translation>
    </message>
    <message>
        <source>Echo sent data</source>
        <translation>Отображать отправленные данные</translation>
    </message>
    <message>
        <source>Transmit</source>
        <translation>Передача</translation>
    </message>
    <message>
        <source>Append CRC (CRC16/MODBUS)</source>
        <translation>Добавлять CRC (CRC16/MODBUS)</translation>
    </message>
    <message>
        <source>Repeat send</source>
        <translation>Повторная отправка</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>мс</translation>
    </message>
    <message>
        <source>Unknown device</source>
        <translation>Неизвестное устройство</translation>
    </message>
    <message>
        <source>Close port</source>
        <translation>Закрыть порт</translation>
    </message>
    <message>
        <source>Open port</source>
        <translation>Открыть порт</translation>
    </message>
</context>
<context>
    <name>ui::SettingsDialog</name>
    <message>
        <source>Settings &amp; About</source>
        <translation>Настройки и о программе</translation>
    </message>
    <message>
        <source>Interface language</source>
        <translation>Язык интерфейса</translation>
    </message>
    <message>
        <source>System font</source>
        <translation>Системный шрифт</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Data monitor font</source>
        <translation>Шрифт монитора данных</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>Тема</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>Светлая</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>Тёмная</translation>
    </message>
    <message>
        <source>Command library</source>
        <translation>Библиотека команд</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translation>Проверить обновления</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <source>Platform</source>
        <translation>Платформа</translation>
    </message>
    <message>
        <source>Support</source>
        <translation>Поддержка</translation>
    </message>
    <message>
        <source>Devices</source>
        <translation>Устройства</translation>
    </message>
    <message>
        <source>Restore defaults</source>
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>%1 (Qt %2)</source>
        <translation>%1 (Qt %2)</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translation>Актуально</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>Доступно обновление</translation>
    </message>
    <message>
        <source>%1 models · %2 commands</source>
        <translation>Моделей: %1 · команд: %2</translation>
    </message>
</context>
<context>
    <name>ui::SoftwareUpdateDialog</name>
    <message>
        <source>Software Update</source>
        <translation>Обновление программы</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>Позже</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Повторить</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>Обновить сейчас</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <source>Version %1 is available (you have %2)</source>
        <translation>Доступна версия %1 (у вас %2)</translation>
    </message>
</context>
<context>
    <name>ui::TemplateDialog</name>
    <message>
        <source>Edit template</source>
        <translation>Изменить шаблон</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>Новый шаблон</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Название</translation>
    </message>
    <message>
        <source>e.g. Reset device</source>
        <translation>например: сброс устройства</translation>
    </message>
    <message>
        <source>Content</source>
        <translation>Содержимое</translation>
    </message>
    <message>
        <source>The literal text to send</source>
        <translation>Точный текст для отправки</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
</context>
</TS>
