#include "core/software_update_client.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QVersionNumber>

namespace {
constexpr int kCheckTimeoutMs = 8000;
// No per-chunk timeout on the download itself -- a ~100MB release over a
// slow connection can legitimately take minutes; QNetworkReply::downloadProgress
// is what the UI uses to show it's still alive, not a timeout.
}  // namespace

SoftwareUpdateClient::SoftwareUpdateClient(QString baseUrl, QString apiKey, QObject *parent)
    : QObject(parent), manager_(new QNetworkAccessManager(this)), baseUrl_(std::move(baseUrl)),
      apiKey_(std::move(apiKey)) {
    while (baseUrl_.endsWith(QLatin1Char('/'))) baseUrl_.chop(1);
}

void SoftwareUpdateClient::get(const QString &path, const QString &query,
                                const std::function<void(QNetworkReply *)> &onFinished) {
    QUrl url(baseUrl_ + path);
    if (!query.isEmpty()) url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(kCheckTimeoutMs);
    request.setRawHeader("Accept", "application/json");
    if (!apiKey_.isEmpty()) request.setRawHeader("X-Api-Key", apiKey_.toUtf8());

    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, onFinished] {
        reply->deleteLater();
        onFinished(reply);
    });
}

void SoftwareUpdateClient::checkForUpdate(const QString &currentVersion) {
    if (!isConfigured()) {
        emit checkFinished(false, false, false, QString(), QString(), 0, QString(), false, QString(), QString(),
                            tr("Update server not configured (.env)."));
        return;
    }

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("product"), QLatin1String(kProductSlug));
    query.addQueryItem(QStringLiteral("os"), QStringLiteral("windows"));
    // Required by the endpoint's validation but not actually used for any
    // filtering/gating server-side today (see docs/app-self-update.md#2) --
    // any non-empty string satisfies it.
    query.addQueryItem(QStringLiteral("serial"), QStringLiteral("-"));

    get(QStringLiteral("/software/list"), query.toString(QUrl::FullyEncoded),
        [this, currentVersion](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                emit checkFinished(false, false, false, QString(), QString(), 0, QString(), false, QString(),
                                    QString(), reply->errorString());
                return;
            }

            QJsonParseError parseError;
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
            if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
                emit checkFinished(false, false, false, QString(), QString(), 0, QString(), false, QString(),
                                    QString(), tr("Malformed response: %1").arg(parseError.errorString()));
                return;
            }

            // Not a bare array -- larke-admin's ResponseJson trait (which
            // SoftwareController uses) wraps every response as
            // {"success": bool, "code": int, "message": string, "data": ...}
            // regardless of endpoint; the product list itself is "data".
            const QJsonObject envelope = doc.object();
            if (!envelope.value(QStringLiteral("success")).toBool()) {
                emit checkFinished(false, false, false, QString(), QString(), 0, QString(), false, QString(),
                                    QString(),
                                    envelope.value(QStringLiteral("message")).toString(tr("Server reported an error.")));
                return;
            }

            // Server returns every registered product, not just ours (see
            // this class's header comment) -- find the one matching our
            // slug ourselves.
            QJsonObject match;
            bool found = false;
            for (const QJsonValue &v : envelope.value(QStringLiteral("data")).toArray()) {
                const QJsonObject obj = v.toObject();
                if (obj.value(QStringLiteral("slug")).toString() == QLatin1String(kProductSlug)) {
                    match = obj;
                    found = true;
                    break;
                }
            }
            if (!found) {
                emit checkFinished(false, false, false, QString(), QString(), 0, QString(), false, QString(),
                                    QString(),
                                    tr("This app isn't registered on the update server (slug \"%1\" not found).")
                                        .arg(QLatin1String(kProductSlug)));
                return;
            }

            const QJsonValue versionVal = match.value(QStringLiteral("version"));
            if (!versionVal.isObject()) {
                // Registered, but no Version row uploaded for "windows" yet
                // (or none with is_available=true) -- not an error, just
                // nothing to offer.
                emit checkFinished(true, false, false, QString(), QString(), 0, QString(), false, QString(),
                                    QString(), QString());
                return;
            }

            const QJsonObject verObj = versionVal.toObject();
            const QString remoteVersion = verObj.value(QStringLiteral("version")).toString();
            const QString downloadUrl = verObj.value(QStringLiteral("file")).toString();
            const qint64 fileSize = verObj.value(QStringLiteral("file_size")).toVariant().toLongLong();
            const QString changelog = verObj.value(QStringLiteral("description")).toString();
            const bool forceUpdate = verObj.value(QStringLiteral("is_force_update")).toBool(false);
            const QString minRequiredVersion = verObj.value(QStringLiteral("min_required_version")).toString();
            const QString sha256 = verObj.value(QStringLiteral("sha256")).toString();

            const bool updateAvailable = !remoteVersion.isEmpty() &&
                QVersionNumber::fromString(remoteVersion) > QVersionNumber::fromString(currentVersion);
            const bool appTooOld = updateAvailable && !minRequiredVersion.isEmpty() &&
                QVersionNumber::fromString(currentVersion) < QVersionNumber::fromString(minRequiredVersion);

            emit checkFinished(true, updateAvailable, appTooOld, remoteVersion, downloadUrl, fileSize, changelog,
                                forceUpdate, minRequiredVersion, sha256, QString());
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
    request.setRawHeader("Accept", "*/*");
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
