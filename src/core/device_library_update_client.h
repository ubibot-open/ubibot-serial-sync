#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;

// Talks to the backend described in docs/device-library-update-protocol.md --
// two read-only JSON endpoints: {baseUrl}/version (cheap metadata, "is there
// anything newer") and {baseUrl}/latest (the full devices.json-shaped
// payload to actually apply). AppController owns one instance and reacts to
// the signals below to update its own libraryUpdate* properties and, on a
// successful fetch, hand the raw bytes to DeviceLibrary::loadFromJsonText().
//
// baseUrl/apiKey come from EnvConfig (a ".env" file deliberately not part of
// this repo -- see docs/device-library-update-protocol.md#2). An empty
// baseUrl means "remote update disabled": isConfigured() is false and both
// checkForUpdate()/fetchLatest() synchronously emit an error signal instead
// of making a request, so callers don't need to special-case this
// themselves.
class DeviceLibraryUpdateClient : public QObject {
    Q_OBJECT
public:
    explicit DeviceLibraryUpdateClient(QString baseUrl, QString apiKey, QObject *parent = nullptr);

    bool isConfigured() const { return !baseUrl_.isEmpty(); }

    // GET {baseUrl}/version. currentVersion is compared against the
    // response's own "version" as plain strings (see protocol doc for the
    // recommended lib-YYYY.MM.DD[.N] format, which sorts correctly as text)
    // to decide the updateAvailable flag on checkFinished.
    void checkForUpdate(const QString &currentVersion);

    // GET {baseUrl}/latest. Verifies the X-Content-SHA256 response header
    // (if the server sent one) against the raw response body before
    // reporting success -- a mismatch is reported as an error and the
    // payload is discarded rather than handed to the caller.
    void fetchLatest();

signals:
    // updateAvailable/remoteVersion/minAppVersion/message are meaningless
    // when ok is false. message is whichever of the server's localized
    // changelog.zh/changelog.en matches the app's current language, empty if
    // the server didn't send a changelog. minAppVersion is the response's
    // own field verbatim (empty when the server didn't send one, meaning "no
    // minimum") -- comparing it against the running app's own version is
    // left to the caller (AppController), since this class has no notion of
    // "the app's version".
    void checkFinished(bool ok, bool updateAvailable, const QString &remoteVersion, const QString &minAppVersion,
                        const QString &message, const QString &error);
    // rawJson is exactly what DeviceLibrary::loadFromJsonText() expects --
    // the same {version, models[...]} shape as resources/devices.json, just
    // fetched over the network instead of read from a Qt resource.
    void fetchFinished(bool ok, const QString &version, const QByteArray &rawJson, const QString &error);

private:
    void get(const QString &path, const std::function<void(QNetworkReply *)> &onFinished);

    QNetworkAccessManager *manager_;
    QString baseUrl_;
    QString apiKey_;
};
