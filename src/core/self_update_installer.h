#pragma once

#include <QString>

// Local-filesystem half of the self-update flow: turning a downloaded
// release zip into a running, swapped-in installation. Pure static helpers
// (no QObject/signals) since every step here is a short, synchronous local
// operation -- SoftwareUpdateClient (network) and AppController
// (orchestration + the QML-facing state machine) are what have the async/
// signal-based shape; this is just plumbing between them. See
// docs/app-self-update.md for the full design and its known limitations.
// Windows-only (tar.exe/robocopy/cmd.exe) -- callers must guard with
// Q_OS_WIN; see app_controller.cpp.
namespace SelfUpdateInstaller {

struct Result {
    bool ok = false;
    QString error;
};

// SHA-256 of a local file's contents, lowercase hex -- used to verify a
// download against the server's optional `sha256` field (see
// SoftwareUpdateClient) before it's ever extracted/applied. Returns an empty
// string on a read failure (treated by the caller as "can't verify", not as
// a mismatch -- see docs/app-self-update.md#2 for why the field is almost
// always empty in practice today).
QString sha256OfFile(const QString &path);

// Extracts `zipPath` into a freshly created temp directory (returned via
// `outStagingDir` on success) using the `tar` utility bundled with Windows
// 10 1803+/Windows 11 (no zip support in Qt Core, and pulling in a third-
// party zip library felt like overkill for "unzip one file" -- see
// docs/app-self-update.md#4 for the alternatives considered), then checks
// that `exeName` exists either at the staging directory's top level or one
// directory down (covers both "zip the contents of the deploy folder" and
// "zip the deploy folder itself") as a sanity check that the archive
// actually contains a real release and not, say, an HTML error page saved
// with a .zip extension.
Result extractAndValidate(const QString &zipPath, const QString &exeName, QString &outStagingDir);

// Writes a batch script (returned path; empty + outError set on failure)
// that, when run after this process has exited:
//   1. waits for `waitForPid` to no longer be a running process,
//   2. robocopy /MIR-mirrors `stagingDir` over `installDir` -- EXCEPT any
//      file literally named ".env" (see docs/app-self-update.md#5 for why:
//      that's the user's own local device-library-update-server config, not
//      part of a release, and /MIR would otherwise delete it since a
//      downloaded release naturally doesn't include one),
//   3. relaunches `installDir/exeName`,
//   4. best-effort deletes stagingDir, the original zip at `zipPath`, and
//      finally itself.
QString writeUpdateScript(const QString &stagingDir, const QString &installDir, const QString &exeName,
                           const QString &zipPath, qint64 waitForPid, QString &outError);

// Launches `scriptPath` detached (so it keeps running after this process
// exits) via cmd.exe. Returns false if it couldn't even be started --
// there's nothing further to observe once it's detached successfully, a
// batch script has no way to report its own outcome back to us.
bool launchDetached(const QString &scriptPath);

}  // namespace SelfUpdateInstaller
