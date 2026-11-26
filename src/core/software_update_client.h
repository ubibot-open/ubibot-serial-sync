// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

// Checks for and downloads new versions of this app from its GitHub Releases
// -- the same releases .github/workflows/release.yml builds and publishes when
// a "vX.Y.Z.W" tag is pushed. There is no separate download server: version,
// changelog and package all come from the release itself.
//
// Check: GET https://api.github.com/repos/{owner}/{repo}/releases/latest,
// which returns the newest published, non-prerelease, non-draft release. Its
// tag ("v0.8.1.1", compared after dropping the "v") is the version, its body
// is the changelog, and the asset whose name ends in the running platform's
// suffix is the package (see platformAssetSuffix()). The workflow uploads two
// copies of every package -- a versioned name and a fixed one
// ("...-windows-x64.zip") that backs GitHub's stable
// /releases/latest/download/<name> link -- and this class prefers the
// versioned one.
//
// Checksums: the Releases API also returns a per-asset "digest"
// ("sha256:..."), which is what checkFinished() reports as sha256 and what
// AppController verifies the download against before extracting it. Assets
// uploaded before GitHub computed digests have none, in which case the
// download is applied unverified (the same best-effort stance as before).
//
// The repository must be public. The API is queried unauthenticated -- no
// token is embedded, since anything shipped in the binary is extractable --
// which caps it at 60 requests/hour per IP; see
// AppController::maybeCheckForAppUpdate() for the once-a-day throttle that
// keeps the automatic startup check well inside that budget.
//
// Network only. Extracting the downloaded package, swapping the installation
// and relaunching is SelfUpdateInstaller's job.
class SoftwareUpdateClient : public QObject {
    Q_OBJECT
public:
    // owner/name of the repository whose Releases carry the packages. Can be
    // overridden with a ".env" APP_UPDATE_GITHUB_REPO=owner/name (useful for
    // forks, and for testing against a throwaway release).
    static constexpr const char *kDefaultRepo = "ubibot-open/ubibot-serial-sync";

    explicit SoftwareUpdateClient(QObject *parent = nullptr);

    bool isConfigured() const { return repo_.contains(QLatin1Char('/')); }

    // Compares currentVersion against the latest release's tag with
    // QVersionNumber -- this project uses four-component versions
    // ("0.8.1.1"), which QVersionNumber handles fine (0.8.1.1 > 0.8.1).
    void checkForUpdate(const QString &currentVersion);

    // Downloads `url` to `destPath` (any existing file there is overwritten),
    // reporting progress via downloadProgress(). Only one download at a time
    // -- starting a new one while another is in flight aborts the old one
    // first.
    void download(const QString &url, const QString &destPath);
    void cancelDownload();

signals:
    // releasePageUrl is the release's html_url. Platforms without in-place
    // replacement (see SelfUpdateInstaller) open that instead of downloading,
    // so it's reported even when updateAvailable is false. version/
    // downloadUrl/fileSize/changelog/sha256 are all empty/zero when
    // updateAvailable is false or ok is false.
    void checkFinished(bool ok, bool updateAvailable, const QString &version, const QString &changelog,
                       const QString &downloadUrl, qint64 fileSize, const QString &sha256,
                       const QString &releasePageUrl, const QString &error);

    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(bool ok, const QString &filePath, const QString &error);

private:
    QNetworkAccessManager *manager_;
    QString repo_;

    QNetworkReply *activeDownload_ = nullptr;
    QFile *downloadFile_ = nullptr;
};
