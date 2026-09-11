#include "core/driver_check.h"

#include <QFileInfo>

#if defined(Q_OS_WIN)

bool DriverCheck::isCh340DriverInstalled() {
    // Mirrors UbiBot's existing Electron app's own check (see
    // docs/ch340-driver-hint.md) -- both driver-file names it looks for,
    // CH341SER.SYS and CH341S64.SYS (older/32-bit vs. 64-bit driver package
    // builds; different WCH driver versions have shipped under either
    // name), under wherever Windows itself is actually installed.
    //
    // SystemRoot (Windows always sets this for every process, to wherever
    // it's actually installed -- typically but not always "C:\Windows")
    // is used here instead of the Electron app's own "guess C:\ if the
    // current drive doesn't look right" fallback: that fallback exists
    // there because Node's process.cwd() reflects whatever drive the app
    // happens to be *running from* (e.g. a portable zip build launched
    // from a USB stick or a non-system drive), which has no reliable
    // relationship to which drive Windows itself is installed on --
    // SystemRoot doesn't have that problem since Windows sets it to the
    // real answer regardless of where the calling process lives, so no
    // equivalent fallback guess is needed here.
    QString systemRoot = qEnvironmentVariable("SystemRoot");
    if (systemRoot.isEmpty()) systemRoot = QStringLiteral("C:/Windows");
    const QString driversDir = systemRoot + QStringLiteral("/System32/drivers/");
    return QFileInfo::exists(driversDir + QStringLiteral("CH341SER.SYS")) ||
           QFileInfo::exists(driversDir + QStringLiteral("CH341S64.SYS"));
}

#elif defined(Q_OS_MACOS)

bool DriverCheck::isCh340DriverInstalled() {
    // Same path UbiBot's existing Electron app checks. Older-style kext,
    // not the newer signed system-extension (dext) mechanism some CH340
    // driver distributions have moved to on recent macOS -- ported as-is
    // from the app already shipping today rather than re-derived, so
    // behavior matches what users of that app already experience; revisit
    // if a dext-only driver package is ever what's actually distributed.
    return QFileInfo::exists(QStringLiteral("/Library/Extensions/usbserial.kext"));
}

#else

bool DriverCheck::isCh340DriverInstalled() { return true; }

#endif

QString DriverCheck::downloadUrl() {
#if defined(Q_OS_WIN)
    return QStringLiteral("https://www.ubibot.io/download/3168");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("https://www.ubibot.io/download/3166");
#else
    return QString();
#endif
}

QList<DriverCheck::DriverInfo> DriverCheck::knownDrivers() {
    return {
        {QStringLiteral("CH340 / CH341"), QStringLiteral("USB-to-serial driver used by most UbiBot devices"),
         downloadUrl(), isCh340DriverInstalled()},
        // Add future drivers (CP210x, FTDI, ...) as one more entry here --
        // each just needs its own installed-check function and per-platform
        // URL, same shape as the CH340 one above; nothing about
        // AppController::availableDrivers() or DriverListDialog.qml needs
        // to change to pick it up.
    };
}
