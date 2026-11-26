// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "core/software_update_client.h"

#include "core/env_config.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QVersionNumber>

namespace {

// Generous: api.github.com is not always quick to reach, and this runs in the
// background (the dialog shows an indeterminate progress bar meanwhile).
constexpr int kCheckTimeoutMs = 15000;
// Not a total download budget -- QNetworkRequest's transfer timeout is an
// idle timeout, so a slow-but-moving ~40MB download is never cut off.
constexpr int kDownloadStallTimeoutMs = 30000;
// Release bodies are free-form; keep a pathological one from turning the
// update dialog into a wall of text.
constexpr int kMaxChangelogChars = 2000;

// The asset-name suffix .github/workflows/release.yml gives the package for
// the platform we're running on. Empty on a platform it doesn't build for.
QString platformAssetSuffix() {
#if defined(Q_OS_WIN)
    return QStringLiteral("-windows-x64.zip");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("-macos.dmg");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("-linux-x86_64.AppImage");
#else
    return QString();
#endif
}

QByteArray userAgent() {
    return QStringLiteral("UbiBotSerialAssistant/")
        .append(QCoreApplication::applicationVersion())
        .toUtf8();
}

QString trimmedChangelog(const QString &body) {
    const QString text = body.trimmed();
    if (text.size() <= kMaxChangelogChars) return text;
    return text.left(kMaxChangelogChars).trimmed() + QStringLiteral("…");
}

}  // namespace

SoftwareUpdateClient::SoftwareUpdateClient(QObject *parent)
    : QObject(parent), manager_(new QNetworkAccessManager(this)) {
    // EnvConfig::value() only falls back when the key is absent, so an
    // empty-but-present override (the shape .env.example ships) must be
    // treated as "not overridden" too.
    repo_ = EnvConfig::instance().value(QStringLiteral("APP_UPDATE_GITHUB_REPO")).trimmed();
    if (repo_.isEmpty()) repo_ = QLatin1String(kDefaultRepo);
    while (repo_.endsWith(QLatin1Char('/'))) repo_.chop(1);
    if (repo_.startsWith(QLatin1Char('/'))) repo_.remove(0, 1);
}

void SoftwareUpdateClient::checkForUpdate(const QString &currentVersion) {
    if (!isConfigured()) {
        emit checkFinished(false, false, QString(), QString(), QString(), 0, QString(), QString(),
                           tr("No update source configured."));
        return;
    }

    // Unauthenticated API: GitHub requires a User-Agent, and answers with 404
    // when the repository has no published release at all.
    QNetworkRequest request(
        QUrl(QStringLiteral("https://api.github.com/repos/%1/releases/latest").arg(repo_)));
    request.setTransferTimeout(kCheckTimeoutMs);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader("User-Agent", userAgent());

    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentVersion] {
        reply->deleteLater();

        const QByteArray payload = reply->readAll();
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        const QJsonObject release = doc.object();

        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString error = reply->errorString();
            // 403/429 from the API is almost always the anonymous rate limit,
            // and its own "message" explains that better than Qt's generic
            // "Host requires authentication" / "Unknown error".
            const QString serverMessage = release.value(QStringLiteral("message")).toString();
            if ((status == 403 || status == 429) && !serverMessage.isEmpty())
                error = tr("GitHub refused the request: %1").arg(serverMessage);
            else if (status == 404 && reply->error() == QNetworkReply::ContentNotFoundError)
                error = tr("No published release was found.");
            emit checkFinished(false, false, QString(), QString(), QString(), 0, QString(), QString(), error);
            return;
        }
        if (parseError.error != QJsonParseError::NoError || !release.contains(QStringLiteral("tag_name"))) {
            emit checkFinished(false, false, QString(), QString(), QString(), 0, QString(), QString(),
                               tr("Malformed GitHub response: %1").arg(parseError.errorString()));
            return;
        }

        QString remoteVersion = release.value(QStringLiteral("tag_name")).toString().trimmed();
        if (remoteVersion.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) remoteVersion.remove(0, 1);
        const QVersionNumber remote = QVersionNumber::fromString(remoteVersion);
        if (remote.isNull()) {
            emit checkFinished(false, false, QString(), QString(), QString(), 0, QString(), QString(),
                               tr("Release tag \"%1\" is not a version number.").arg(remoteVersion));
            return;
        }

        const QString releasePageUrl = release.value(QStringLiteral("html_url")).toString();
        const QString changelog = trimmedChangelog(release.value(QStringLiteral("body")).toString());
        const bool updateAvailable = remote > QVersionNumber::fromString(currentVersion);

        if (!updateAvailable) {
            emit checkFinished(true, false, remoteVersion, QString(), QString(), 0, QString(), releasePageUrl,
                                QString());
            return;
        }

        // Prefer the versioned copy of the package; fall back to the fixed
        // name (both are the same bytes in a release built by release.yml).
        const QString suffix = platformAssetSuffix();
        QJsonObject asset;
        for (const QJsonValue &value : release.value(QStringLiteral("assets")).toArray()) {
            const QJsonObject candidate = value.toObject();
            const QString name = candidate.value(QStringLiteral("name")).toString();
            if (suffix.isEmpty() || !name.endsWith(suffix, Qt::CaseInsensitive)) continue;
            if (name.contains(remoteVersion)) {
                asset = candidate;
                break;
            }
            if (asset.isEmpty()) asset = candidate;
        }

        const QString downloadUrl = asset.value(QStringLiteral("browser_download_url")).toString();
        if (downloadUrl.isEmpty()) {
            emit checkFinished(false, false, QString(), QString(), QString(), 0, QString(), releasePageUrl,
                               suffix.isEmpty()
                                   ? tr("Release %1 has no package for this platform.").arg(remoteVersion)
                                   : tr("Release %1 has no %2 package.").arg(remoteVersion, suffix.mid(1)));
            return;
        }

        // GitHub computes this when an asset is uploaded; assets from older
        // releases may not carry one, in which case the download goes
        // unverified (same as the previous server-based flow).
        QString sha256 = asset.value(QStringLiteral("digest")).toString();
        if (sha256.startsWith(QLatin1String("sha256:"), Qt::CaseInsensitive)) sha256.remove(0, 7);

        emit checkFinished(true, true, remoteVersion, changelog, downloadUrl,
                           asset.value(QStringLiteral("size")).toVariant().toLongLong(), sha256, releasePageUrl,
                           QString());
    });
}

void SoftwareUpdateClient::download(const QString &url, const QString &destPath) {
    cancelDownload();

    auto *file = new QFile(destPath, this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit downloadFinished(false, QString(), tr("Cannot write to %1: %2").arg(destPath, file->errorString()));
        delete file;
        return;
    }
    downloadFile_ = file;

    QNetworkRequest request{QUrl(url)};
    request.setTransferTimeout(kDownloadStallTimeoutMs);
    // The asset URL redirects to objects.githubusercontent.com, so following
    // redirects is mandatory rather than merely convenient.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Accept", "application/octet-stream");
    request.setRawHeader("User-Agent", userAgent());
    QNetworkReply *reply = manager_->get(request);
    activeDownload_ = reply;

    // `file` (not the downloadFile_ member) is what readyRead/finished below
    // actually write to/close -- binding to this call's own QFile instance
    // rather than re-reading the member at signal time means a stray queued
    // signal from an aborted, superseded download can never write into (or
    // close/delete) whatever *new* download's file downloadFile_ has since
    // been reassigned to.
    connect(reply, &QNetworkReply::readyRead, this, [file, reply] { file->write(reply->readAll()); });
    connect(reply, &QNetworkReply::downloadProgress, this, &SoftwareUpdateClient::downloadProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply, file, destPath] {
        reply->deleteLater();
        file->write(reply->readAll());
        file->close();
        file->deleteLater();
        if (downloadFile_ == file) downloadFile_ = nullptr;
        if (activeDownload_ != reply) return;  // superseded by cancelDownload()/a newer download()
        activeDownload_ = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            emit downloadFinished(false, QString(), reply->errorString());
            return;
        }
        emit downloadFinished(true, destPath, QString());
    });
}

void SoftwareUpdateClient::cancelDownload() {
    // Deliberately doesn't touch the QFile here -- abort() is documented to
    // still fire finished() afterwards, and that reply's own finished-lambda
    // (see download() above) is the sole owner of closing/deleteLater-ing
    // its file via a locally-captured pointer. Closing/deleting it here too
    // would race that lambda and could double-free it. This method only
    // clears the bookkeeping members, so a subsequent download() doesn't
    // mistake the outgoing request for still being the active one.
    if (activeDownload_) {
        activeDownload_->abort();
        activeDownload_ = nullptr;
    }
    downloadFile_ = nullptr;
}
