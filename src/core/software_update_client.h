#pragma once

#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

// Talks to the *existing, general-purpose* "software/version" distribution
// system in ubibot-appcenter's Laravel backend (app/Http/Controllers/Admin/
// SoftwareController.php's api_list(), routes/api.php's public
// GET /api/software/list) rather than a bespoke protocol -- unlike the
// device-command library (see device_library_update_client.h), the backend
// side of this already existed (used by other UbiBot products) and just
// needed this app registered as one more row (see ubibot-appcenter's
// migration 2026_08_25_010000_register_ubibot_serial_assistant_software.php)
// plus someone uploading Version rows for it through the existing
// admin-react "Software Version Management" page -- see docs/app-self-update.md for the full
// picture, including why baseUrl/apiKey come from EnvConfig exactly like
// DeviceLibraryUpdateClient's do.
//
// GET {baseUrl}/software/list responds with larke-admin's standard
// {"success": bool, "code": int, "message": string, "data": [...]} envelope
// (SoftwareController uses that framework's ResponseJson trait like most of
// the rest of that backend) -- "data" is EVERY registered product (there's
// no server-side filtering by product despite the query param existing --
// see docs/app-self-update.md#2), each optionally carrying its newest
// available Version for the requested platform under a "version" key. This
// class's job is finding *our* entry in that array (matched by
// kProductSlug) and, once the user asks to install it, downloading the
// zipped release. Everything after that -- extracting, swapping files,
// relaunching -- is SelfUpdateInstaller's job, not this class's; this class
// only ever talks to the network.
class SoftwareUpdateClient : public QObject {
    Q_OBJECT
public:
    // Must match the `slug` column of the softwares row registered for this
    // app in ubibot-appcenter (see the migration referenced above).
    static constexpr const char *kProductSlug = "ubibot-serial-assistant";

    explicit SoftwareUpdateClient(QString baseUrl, QString apiKey, QObject *parent = nullptr);

    bool isConfigured() const { return !baseUrl_.isEmpty(); }

    // GET {baseUrl}/software/list?product=<kProductSlug>&os=windows&serial=-
    // ("product"/"serial" are validated server-side but not actually used
    // for filtering today -- see docs/app-self-update.md#2 -- sent anyway in
    // case that changes later; "os" is the one query param that's real,
    // filtering each product's attached version to that platform).
    // currentVersion is compared against the match's version.version via
    // QVersionNumber, same idiom as DeviceLibraryUpdateClient's minAppVersion
    // check.
    void checkForUpdate(const QString &currentVersion);

    // Downloads `url` to `destPath` (any existing file there is overwritten),
    // reporting progress via downloadProgress(). Only one download at a time
    // -- starting a new one while another is in flight aborts the old one
    // first.
    void download(const QString &url, const QString &destPath);
    void cancelDownload();

signals:
    // appTooOld is only meaningful when updateAvailable is true: the remote
    // side has a newer version, but its minRequiredVersion is higher than
    // currentVersion, so the caller shouldn't offer to install it directly
    // (see docs/app-self-update.md#3). version/downloadUrl/fileSize/
    // changelog/forceUpdate/minRequiredVersion/sha256 are all empty/zero
    // when updateAvailable is false or ok is false. sha256 is presently
    // always empty in practice -- the backend's `versions.sha256` column
    // exists but nothing in the admin upload flow ever populates it (see
    // docs/app-self-update.md#2) -- so checksum verification is best-effort,
    // matching the device-library updater's own "skip if absent" stance.
    void checkFinished(bool ok, bool updateAvailable, bool appTooOld, const QString &version,
                        const QString &downloadUrl, qint64 fileSize, const QString &changelog, bool forceUpdate,
                        const QString &minRequiredVersion, const QString &sha256, const QString &error);

    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(bool ok, const QString &filePath, const QString &error);

private:
    void get(const QString &path, const QString &query, const std::function<void(QNetworkReply *)> &onFinished);

    QNetworkAccessManager *manager_;
    QString baseUrl_;
    QString apiKey_;

    QNetworkReply *activeDownload_ = nullptr;
    QFile *downloadFile_ = nullptr;
};
