#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVector>

// One entry inside the remote manifest.json describing a mod/component
// that the launcher is responsible for keeping in sync.
struct ModEntry {
    QString name;       // Display name, e.g. "JEI"
    QString fileName;   // File name on disk, e.g. "jei-1.20.1.jar"
    QString url;        // Direct download URL for the file
    QString version;    // Version string reported by the manifest
    QString sha256;     // Expected sha256 hash (lowercase hex)
};

// Handles checking a remote manifest, diffing it against what's installed
// locally, and downloading only what changed. Fully asynchronous (signals),
// so it never blocks the UI thread.
class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject *parent = nullptr);

    // URL to a JSON file shaped like:
    // { "mods": [ {"name":"JEI","fileName":"jei.jar","url":"https://.../jei.jar",
    //              "version":"1.2.0","sha256":"..."} , ... ] }
    void setManifestUrl(const QString &url);

    // Local folder mods get installed into, e.g. ".../instances/1.20.1/mods"
    void setModsDir(const QString &dir);

    // Kicks off: download manifest -> diff -> download changed files.
    void checkForUpdates();

signals:
    void checkStarted();
    void manifestFetchFailed(const QString &reason);
    void upToDate();                                   // nothing to do
    void updatesFound(int count);                      // N mods need updating
    void modDownloadStarted(const QString &modName);
    void modDownloadProgress(const QString &modName, qint64 received, qint64 total);
    void modDownloadFinished(const QString &modName, bool ok, const QString &error);
    void allUpdatesFinished(int updatedCount, int failedCount);

private slots:
    void onManifestReply();
    void onModDownloadReply();
    void onModDownloadProgress(qint64 received, qint64 total);

private:
    void downloadNextMod();
    QString localVersionFile() const;
    QString localVersionFor(const QString &fileName) const;
    void recordLocalVersion(const QString &fileName, const QString &version);
    static QString sha256OfFile(const QString &path);

    QNetworkAccessManager m_net;
    QString m_manifestUrl;
    QString m_modsDir;

    QVector<ModEntry> m_pending;   // mods still needing (re)download
    int m_updatedCount = 0;
    int m_failedCount = 0;
    QNetworkReply *m_currentModReply = nullptr;
    ModEntry m_currentMod;
};
