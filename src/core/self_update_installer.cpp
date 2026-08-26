#include "core/self_update_installer.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>

namespace {

// Suffixed with the current process's own PID + a millisecond timestamp so
// two concurrent runs (or a leftover directory from a previous crashed
// attempt) never collide.
QString freshTempDir(const QString &prefix) {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString dir = QStringLiteral("%1/%2-%3-%4")
                             .arg(base, prefix)
                             .arg(QCoreApplication::applicationPid())
                             .arg(QDateTime::currentMSecsSinceEpoch());
    QDir().mkpath(dir);
    return dir;
}

}  // namespace

namespace SelfUpdateInstaller {

QString sha256OfFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return QString();

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) return QString();
    return QString::fromLatin1(hash.result().toHex());
}

Result extractAndValidate(const QString &zipPath, const QString &exeName, QString &outStagingDir) {
    const QString staging = freshTempDir(QStringLiteral("UbiBotSerialAssistant-update-staging"));

    QProcess tar;
    tar.setProgram(QStringLiteral("tar"));
    tar.setArguments({QStringLiteral("-xf"), zipPath, QStringLiteral("-C"), staging});
    tar.start();
    if (!tar.waitForStarted(5000)) {
        return {false, QObject::tr("could not start 'tar' (bundled with Windows 10 1803+/Windows 11) -- %1")
                            .arg(tar.errorString())};
    }
    if (!tar.waitForFinished(120000)) {
        tar.kill();
        return {false, QObject::tr("'tar' timed out extracting the update")};
    }
    if (tar.exitStatus() != QProcess::NormalExit || tar.exitCode() != 0) {
        return {false, QObject::tr("'tar' failed (exit %1): %2")
                            .arg(tar.exitCode())
                            .arg(QString::fromLocal8Bit(tar.readAllStandardError()))};
    }

    if (QFile::exists(staging + QLatin1Char('/') + exeName)) {
        outStagingDir = staging;
        return {true, QString()};
    }
    // The zip might have everything one directory level down (e.g. it was
    // created by zipping the deploy folder itself, not its contents) --
    // check exactly one level down before giving up.
    const QStringList entries = QDir(staging).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        const QString candidate = staging + QLatin1Char('/') + entry;
        if (QFile::exists(candidate + QLatin1Char('/') + exeName)) {
            outStagingDir = candidate;
            return {true, QString()};
        }
    }

    return {false,
            QObject::tr("downloaded archive doesn't contain %1 -- not a valid release package").arg(exeName)};
}

QString writeUpdateScript(const QString &stagingDir, const QString &installDir, const QString &exeName,
                           const QString &zipPath, qint64 waitForPid, QString &outError) {
    const QString scriptPath =
        freshTempDir(QStringLiteral("UbiBotSerialAssistant-update-script")) + QStringLiteral("/apply-update.bat");

    // Every path is baked in as a literal quoted string rather than passed
    // as a %1/%2 argument -- simpler than getting cmd.exe's argument-quoting
    // rules right for paths that might contain spaces.
    const QString nativeStaging = QDir::toNativeSeparators(stagingDir);
    const QString nativeInstall = QDir::toNativeSeparators(installDir);
    const QString nativeZip = QDir::toNativeSeparators(zipPath);

    QString script;
    QTextStream out(&script);
    out << "@echo off\r\n";
    out << "setlocal\r\n";
    // Wait for the process we were launched from to actually exit --
    // tasklist's output for a PID that's gone won't contain that PID.
    out << ":waitloop\r\n";
    out << "tasklist /FI \"PID eq " << waitForPid << "\" | find \"" << waitForPid << "\" >nul\r\n";
    out << "if not errorlevel 1 (\r\n";
    out << "  timeout /t 1 /nobreak >nul\r\n";
    out << "  goto waitloop\r\n";
    out << ")\r\n";
    // /MIR mirrors (deletes anything under installDir that isn't in staging
    // too, so a release that removed/renamed a DLL doesn't leave the old one
    // behind) except ".env" -- see this header's own doc comment. /R:20
    // /W:1 retries a locked file for up to ~20s instead of giving up
    // immediately, in case something (antivirus, a lingering handle) hasn't
    // let go of a file the instant the process list says the app exited.
    out << "robocopy \"" << nativeStaging << "\" \"" << nativeInstall
        << "\" /MIR /XF \".env\" /R:20 /W:1 /NFL /NDL /NJH /NJS >nul\r\n";
    out << "start \"\" \"" << nativeInstall << "\\" << exeName << "\"\r\n";
    out << "rmdir /s /q \"" << nativeStaging << "\" >nul 2>nul\r\n";
    out << "del \"" << nativeZip << "\" >nul 2>nul\r\n";
    // Self-delete trick: `(goto) 2>nul` swallows the harmless "unexpected
    // goto" error that command interpreter raises here, so the `& del`
    // after it still runs even though a batch file can't normally delete
    // itself mid-execution -- Windows allows this specific pattern because
    // the delete only actually happens once cmd.exe releases its handle on
    // the file, which is right after this line finishes.
    out << "(goto) 2>nul & del \"%~f0\"\r\n";
    out.flush();

    QFile file(scriptPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        outError = QObject::tr("could not write updater script to %1: %2").arg(scriptPath, file.errorString());
        return QString();
    }
    file.write(script.toLocal8Bit());
    file.close();
    return scriptPath;
}

bool launchDetached(const QString &scriptPath) {
    // Runs via cmd.exe /C rather than QProcess::startDetached(scriptPath)
    // directly -- Windows only knows how to run a .bat through the command
    // interpreter, not as a directly-executable image.
    return QProcess::startDetached(QStringLiteral("cmd.exe"), {QStringLiteral("/C"), scriptPath});
}

}  // namespace SelfUpdateInstaller
