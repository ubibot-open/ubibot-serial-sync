#include "core/device_library_update_client.h"
#include "core/device_library.h"  // LocalizedText

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace {
constexpr int kTimeoutMs = 8000;

LocalizedText readLocalizedField(const QJsonObject &obj, const char *key) {
    // NB: QStringLiteral requires a literal token at the call site, not a
    // runtime `const char *` -- QLatin1String is the right tool for wrapping
    // a parameter instead.
    const QJsonObject sub = obj.value(QLatin1String(key)).toObject();
    LocalizedText t;
    t.zh = sub.value(QStringLiteral("zh")).toString();
    t.en = sub.value(QStringLiteral("en")).toString();
    return t;
}

// Both endpoints share the same "ok" / "error.message" envelope (see
// docs/device-library-update-protocol.md#3) -- returns the message to
// surface when ok is false, or a null QString when the response is fine.
QString envelopeError(const QJsonObject &obj) {
    if (obj.value(QStringLiteral("ok")).toBool(true)) return QString();
    const QString msg = obj.value(QStringLiteral("error")).toObject().value(QStringLiteral("message")).toString();
    return msg.isEmpty() ? QStringLiteral("Server reported an error.") : msg;
}
}  // namespace

DeviceLibraryUpdateClient::DeviceLibraryUpdateClient(QString baseUrl, QString apiKey, QObject *parent)
    : QObject(parent), manager_(new QNetworkAccessManager(this)), baseUrl_(std::move(baseUrl)),
      apiKey_(std::move(apiKey)) {
    // Trim trailing slashes so `baseUrl_ + "/version"` below never produces a
    // doubled "//" regardless of how the .env value was written.
    while (baseUrl_.endsWith(QLatin1Char('/'))) baseUrl_.chop(1);
}

void DeviceLibraryUpdateClient::get(const QString &path, const std::function<void(QNetworkReply *)> &onFinished) {
    QUrl url(baseUrl_ + path);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("app"), QStringLiteral("ubibot-serial-assistant"));
    query.addQueryItem(QStringLiteral("appVersion"), QStringLiteral(APP_VERSION));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(kTimeoutMs);
    request.setRawHeader("Accept", "application/json");
    if (!apiKey_.isEmpty()) request.setRawHeader("X-Api-Key", apiKey_.toUtf8());

    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, onFinished] {
        reply->deleteLater();
        onFinished(reply);
    });
}

void DeviceLibraryUpdateClient::checkForUpdate(const QString &currentVersion) {
    if (!isConfigured()) {
        emit checkFinished(false, false, QString(), QString(), QString(),
                            tr("Remote update server not configured (.env)."));
        return;
    }

    get(QStringLiteral("/version"), [this, currentVersion](QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            emit checkFinished(false, false, QString(), QString(), QString(), reply->errorString());
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit checkFinished(false, false, QString(), QString(), QString(),
                                tr("Malformed response: %1").arg(parseError.errorString()));
            return;
        }

        const QJsonObject obj = doc.object();
        const QString envelopeErr = envelopeError(obj);
        if (!envelopeErr.isNull()) {
            emit checkFinished(false, false, QString(), QString(), QString(), envelopeErr);
            return;
        }

        const QString remoteVersion = obj.value(QStringLiteral("version")).toString();
        const QString minAppVersion = obj.value(QStringLiteral("minAppVersion")).toString();
        const QString message = readLocalizedField(obj, "changelog").text();
        const bool updateAvailable = !remoteVersion.isEmpty() && remoteVersion != currentVersion;
        emit checkFinished(true, updateAvailable, remoteVersion, minAppVersion, message, QString());
    });
}

void DeviceLibraryUpdateClient::fetchLatest() {
    if (!isConfigured()) {
        emit fetchFinished(false, QString(), QByteArray(), tr("Remote update server not configured (.env)."));
        return;
    }

    get(QStringLiteral("/latest"), [this](QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            emit fetchFinished(false, QString(), QByteArray(), reply->errorString());
            return;
        }

        const QByteArray body = reply->readAll();

        // See docs/device-library-update-protocol.md#5 -- optional
        // whole-body integrity check, skipped silently if the server didn't
        // send the header.
        const QByteArray expectedHex = reply->rawHeader("X-Content-SHA256");
        if (!expectedHex.isEmpty()) {
            const QByteArray actualHex = QCryptographicHash::hash(body, QCryptographicHash::Sha256).toHex();
            if (actualHex.compare(expectedHex, Qt::CaseInsensitive) != 0) {
                emit fetchFinished(false, QString(), QByteArray(),
                                    tr("Checksum mismatch -- downloaded data may be corrupted."));
                return;
            }
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit fetchFinished(false, QString(), QByteArray(),
                                tr("Malformed response: %1").arg(parseError.errorString()));
            return;
        }

        const QJsonObject obj = doc.object();
        const QString envelopeErr = envelopeError(obj);
        if (!envelopeErr.isNull()) {
            emit fetchFinished(false, QString(), QByteArray(), envelopeErr);
            return;
        }
        if (!obj.contains(QStringLiteral("models"))) {
            emit fetchFinished(false, QString(), QByteArray(), tr("Response has no models[] array."));
            return;
        }

        emit fetchFinished(true, obj.value(QStringLiteral("version")).toString(), body, QString());
    });
}
