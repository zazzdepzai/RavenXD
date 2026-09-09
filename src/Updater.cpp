#include "Updater.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QSettings>

Updater::Updater(QObject *parent) : QObject(parent) {}

void Updater::setManifestUrl(const QString &url) { m_manifestUrl = url; }
void Updater::setModsDir(const QString &dir) {
    m_modsDir = dir;
    QDir().mkpath(m_modsDir);
}

QString Updater::localVersionFile() const {
    return m_modsDir + "/.installed_versions.ini";
}

QString Updater::localVersionFor(const QString &fileName) const {
    QSettings s(localVersionFile(), QSettings::IniFormat);
    return s.value("versions/" + fileName).toString();
}

void Updater::recordLocalVersion(const QString &fileName, const QString &version) {
    QSettings s(localVersionFile(), QSettings::IniFormat);
    s.setValue("versions/" + fileName, version);
}

QString Updater::sha256OfFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return QString();
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&f)) return QString();
    return hash.result().toHex();
}

void Updater::checkForUpdates() {
    if (m_manifestUrl.isEmpty() || m_modsDir.isEmpty()) {
        emit manifestFetchFailed("Chưa cấu hình manifestUrl hoặc modsDir");
        return;
    }
    emit checkStarted();
    QNetworkRequest req{QUrl(m_manifestUrl)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RavenXDLauncher/2.0");
    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, &Updater::onManifestReply);
}

void Updater::onManifestReply() {
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit manifestFetchFailed(reply->errorString());
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        emit manifestFetchFailed("manifest.json không hợp lệ: " + perr.errorString());
        return;
    }

    const QJsonArray mods = doc.object().value("mods").toArray();
    m_pending.clear();
    m_updatedCount = 0;
    m_failedCount = 0;

    for (const QJsonValue &v : mods) {
        const QJsonObject o = v.toObject();
        ModEntry m;
        m.name = o.value("name").toString();
        m.fileName = o.value("fileName").toString();
        m.url = o.value("url").toString();
        m.version = o.value("version").toString();
        m.sha256 = o.value("sha256").toString().toLower();
        if (m.fileName.isEmpty() || m.url.isEmpty()) continue;

        const QString localPath = m_modsDir + "/" + m.fileName;
        const bool fileMissing = !QFile::exists(localPath);
        const bool versionMismatch = localVersionFor(m.fileName) != m.version;

        if (fileMissing || versionMismatch) {
            m_pending.push_back(m);
        }
    }

    if (m_pending.isEmpty()) {
        emit upToDate();
        return;
    }

    emit updatesFound(m_pending.size());
    downloadNextMod();
}

void Updater::downloadNextMod() {
    if (m_pending.isEmpty()) {
        emit allUpdatesFinished(m_updatedCount, m_failedCount);
        return;
    }

    m_currentMod = m_pending.takeFirst();
    emit modDownloadStarted(m_currentMod.name);

    QNetworkRequest req{QUrl(m_currentMod.url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RavenXDLauncher/2.0");
    m_currentModReply = m_net.get(req);

    connect(m_currentModReply, &QNetworkReply::downloadProgress,
            this, &Updater::onModDownloadProgress);
    connect(m_currentModReply, &QNetworkReply::finished,
            this, &Updater::onModDownloadReply);
}

void Updater::onModDownloadProgress(qint64 received, qint64 total) {
    emit modDownloadProgress(m_currentMod.name, received, total);
}

void Updater::onModDownloadReply() {
    QNetworkReply *reply = m_currentModReply;
    m_currentModReply = nullptr;
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        m_failedCount++;
        emit modDownloadFinished(m_currentMod.name, false, reply->errorString());
        downloadNextMod();
        return;
    }

    const QByteArray bytes = reply->readAll();
    const QString finalPath = m_modsDir + "/" + m_currentMod.fileName;
    const QString tmpPath = finalPath + ".part";

    {
        QFile out(tmpPath);
        if (!out.open(QIODevice::WriteOnly)) {
            m_failedCount++;
            emit modDownloadFinished(m_currentMod.name, false, "Không ghi được file tạm");
            downloadNextMod();
            return;
        }
        out.write(bytes);
    }

    // Verify integrity if manifest provided a hash.
    if (!m_currentMod.sha256.isEmpty()) {
        const QString actual = sha256OfFile(tmpPath);
        if (actual.compare(m_currentMod.sha256, Qt::CaseInsensitive) != 0) {
            QFile::remove(tmpPath);
            m_failedCount++;
            emit modDownloadFinished(m_currentMod.name, false, "Sai checksum, đã huỷ file");
            downloadNextMod();
            return;
        }
    }

    QFile::remove(finalPath);
    QFile::rename(tmpPath, finalPath);
    recordLocalVersion(m_currentMod.fileName, m_currentMod.version);

    m_updatedCount++;
    emit modDownloadFinished(m_currentMod.name, true, QString());
    downloadNextMod();
}
