/**
 * This file is part of Podcatcher for Sailfish OS.
 * Author: Johan Paul (johan.paul@gmail.com)
 *
 * Podcatcher for Sailfish OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Podcatcher for Sailfish OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Podcatcher for Sailfish OS.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QVariant>

#include <QtDebug>
#include <QThread>

#include "podcastglobals.h"
#include "podcastepisode.h"
#include "podcastmanager.h"

#include <unistd.h>

PodcastEpisode::PodcastEpisode(QObject *parent) :
    QObject(parent)
{
    m_state = PodcastEpisode::GetState;
    m_bytesDownloaded = 0;
    m_lastPlayed = QDateTime();
    m_hasBeenCanceled = false;
    m_currentDownload = nullptr;
    m_playFilename = "";
    m_user = "";
    m_password = "";
    m_downloadFile = nullptr;
    m_new = false;
    m_finished = false;

    //m_saveOnSDCOnf = new MGConfItem("/apps/ControlPanel/Podcatcher/saveOnSDCard", this);

}

PodcastEpisode::~PodcastEpisode()
{
    qDebug() << "Destructing episode "<< this->m_title << " from " << (this->channel() ? this->channel()->title() : "unknown channel");
}

void PodcastEpisode::setTitle(const QString &title)
{
    m_title = title;
}

QString PodcastEpisode::title() const
{
    return m_title;
}

void PodcastEpisode::setDownloadLink(const QString &downloadLink)
{
    m_downloadLink = downloadLink;
}

QString PodcastEpisode::downloadLink() const
{
    if (m_downloadLink.isEmpty()) {
        qWarning() << "Download link for postcast is empty! Cannot download.";
    }
    return m_downloadLink;
}

void PodcastEpisode::setDescription(const QString &desc)
{
    m_description = desc;
}

QString PodcastEpisode::description() const
{
    return m_description;
}

void PodcastEpisode::setPubTime(const QDateTime &pubDate)
{
    m_pubDateTime = pubDate;
}

QDateTime PodcastEpisode::pubTime() const
{
    return m_pubDateTime;
}

void PodcastEpisode::setDuration(const QString &duration)
{
    m_duration = duration;
}

QString PodcastEpisode::duration() const
{
    return m_duration;
}

void PodcastEpisode::setDownloadSize(qint64 downloadSize)
{
    m_downloadSize = downloadSize;
}

qint64 PodcastEpisode::downloadSize() const
{
    return m_downloadSize;
}

qint64 PodcastEpisode::alreadyDownloaded()
{
    return m_bytesDownloaded;
}

void PodcastEpisode::setDbId(int id)
{
    m_dbid = id;
}


int PodcastEpisode::dbid() const
{
    return m_dbid;
}

void PodcastEpisode::setState(PodcastEpisode::EpisodeStates newState)
{
    if (m_state != newState) {
        qDebug() << "Setting episode state to " << newState;
        m_state = newState;
        emit episodeChanged();
    }
}

QString PodcastEpisode::episodeState() const
{

    // Optimize: since downloading is asked several times during downloading, put it here first.
    if (m_state == DownloadingState) {
        return QString("downloading");
    }

    /*
    if(m_finished){
        return "finished";
    }
    */

    if (m_lastPlayed.isValid() && !m_finished && !m_playFilename.isEmpty()) {
        return "played";
    }

    if (!m_playFilename.isEmpty()) {
        return "downloaded";
    }

    if (!m_hasBeenCanceled) {
        if (m_downloadLink.isEmpty()) {
            return "undownloadable";
        }
    }

    switch(m_state) {
    case DownloadedState:
        return QString("downloaded");
    case QueuedState:
        return QString("queued");
    case GetState:
    case CanceledState:
    default:
        return QString("get");
    }
}

bool PodcastEpisode::finished() const
{
    return m_finished;
}

void PodcastEpisode::setPlayFilename(const QString &playFilename)
{
    if (playFilename != m_playFilename) {
        m_playFilename = playFilename;
        emit episodeChanged();
    }
}

QString PodcastEpisode::playFilename() const
{
    return m_playFilename;
}


void PodcastEpisode::setChannel(PodcastChannel* channel)
{
    m_channel = channel;
}


int PodcastEpisode::channelid() const
{
    if(m_channel)
        return m_channel->channelDbId();
    else {
        qWarning() << "Channel not set!";
        return -1;
    }
}

PodcastChannel *PodcastEpisode::channel()
{
    return m_channel;
}

void PodcastEpisode::downloadEpisode()
{
    qDebug() << "Downloading podcast:" << m_downloadLink;
    qDebug() << "CurrentThread" << QThread::currentThread();
    qDebug() << "Episode Thread" << this->thread();
    if (m_dlNetworkManager == nullptr) {
        qWarning() << "No QNetworkAccessManager specified for this episode. Cannot proceed.";
        return;
    }

    qDebug() << "QNAM accessible: "<< m_dlNetworkManager->Accessible;

    QUrl downloadUrl(m_downloadLink);
    if (!downloadUrl.isValid()) {
        qWarning() << "Provided podcast download URL is not valid.";
        return;
    }

    if(downloadUrl.userName().isEmpty() && !m_user.isEmpty()){
        downloadUrl.setUserName(m_user);
        downloadUrl.setPassword(m_password);
        qDebug()<< "Using credentials for " << m_user;
    }

    QNetworkRequest request;
    request.setUrl(downloadUrl);
    request.setRawHeader("User-Agent", "Podcatcher Podcast client");
    //request.setRawHeader( "Accept" , "*/*" );

    m_currentDownload = m_dlNetworkManager->get(request);

    QString downloadPath;
    downloadPath = getDownloadDir();

    if (downloadPath.isNull()){
        m_errorMessage = tr("SD card not available! Make sure SD card is mounted and decrypted!");
        emit podcastEpisodeDownloadFailed(this);
        m_currentDownload->abort();
        m_currentDownload->deleteLater();
    }

    QDir dirpath(downloadPath);
    if (!dirpath.exists()) {
        dirpath.mkpath(downloadPath);
    }

    qDebug() << "Saving download at " << downloadPath;

    QString path =  m_currentDownload->url().path();
    QString filename = QFileInfo(path).fileName();

    if(m_downloadFile){
        qWarning() << "Download file exists! Closing it.";
        m_downloadFile->close();
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }

    m_downloadFile = new QFile(downloadPath + filename);
    m_downloadFile->open(QIODevice::WriteOnly);

    connect(m_currentDownload, SIGNAL(finished()),
            this, SLOT(onPodcastEpisodeDownloadCompleted()));
    connect(m_currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(onDownloadProgress(qint64, qint64)));

    connect(m_currentDownload,SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onDownloadError(QNetworkReply::NetworkError)));

    connect(m_currentDownload, SIGNAL(metaDataChanged()),
            this, SLOT(onMetaDataChanged()));

    connect(m_currentDownload, SIGNAL(readyRead()),
            this,SLOT(onDownloadReadyRead()));
}

void PodcastEpisode::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_bytesDownloaded = bytesReceived;
    m_downloadSize = bytesTotal;
    //qDebug() << "Download Progress in" <<title() << bytesReceived;
    emit episodeChanged();
}

void PodcastEpisode::onDownloadReadyRead()
{
    if (m_downloadFile){
        qint64 written = m_downloadFile->write(m_currentDownload->readAll());

        if (written == -1){
            m_errorMessage = m_downloadFile->errorString();
            m_currentDownload->abort();
            emit podcastEpisodeDownloadFailed(this);
        }

    }
    //qDebug() << "Write received bytes.";
}

void PodcastEpisode::onPodcastEpisodeDownloadCompleted()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    QString redirectedUrl = PodcastManager::redirectedRequest(reply);
    if (!redirectedUrl.isEmpty()) {
        qDebug() << "We have been redirected from "<<m_downloadLink <<" to " << redirectedUrl;
        m_downloadLink = redirectedUrl;
        reply->deleteLater();
        downloadEpisode();
        return;
    }

    // TODO: Proper way of handling downloads that are not audio or video formats.

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qWarning() << "Download of podcast was not succesfull: " << reply->errorString();
        m_errorMessage = reply->errorString();
        reply->deleteLater();
        emit podcastEpisodeDownloadFailed(this);
        return;
    }

    m_downloadFile->write(reply->readAll());
    m_downloadFile->close();

    QFileInfo fileInfo(*m_downloadFile);
    m_playFilename = fileInfo.absoluteFilePath();

    qDebug() << "Podcast downloaded: " << m_playFilename;

    emit podcastEpisodeDownloaded(this);
    reply->deleteLater();
    m_downloadFile->deleteLater();
    m_downloadFile = nullptr;
}

void PodcastEpisode::setDownloadManager(QNetworkAccessManager *qnam)
{
    m_dlNetworkManager = qnam;
}

void PodcastEpisode::setLastPlayed(const QDateTime &lastPlayed)
{
    if (lastPlayed != m_lastPlayed) {
        m_lastPlayed = lastPlayed;
        emit episodeChanged();
    }
}

QDateTime PodcastEpisode::lastPlayed() const
{
    return m_lastPlayed;
}

void PodcastEpisode::setHasBeenCanceled(bool canceled)
{
    if(canceled)
        m_state = PodcastEpisode::CanceledState;

    if (canceled != m_hasBeenCanceled) {
        m_hasBeenCanceled = canceled;
        emit episodeChanged();
    }

}

void PodcastEpisode::setCredentails(const QString &user, const QString &password)
{
    m_user = user;
    m_password = password;
}

bool PodcastEpisode::hasBeenCanceled() const
{
    return m_hasBeenCanceled;
}

void PodcastEpisode::cancelCurrentDownload()
{
    if (m_currentDownload != nullptr &&
            m_state == DownloadingState) {
        qDebug() << "Canceling current episode download request...";

        setHasBeenCanceled(true);

        // Abort current download.
        disconnect(m_currentDownload, SIGNAL(finished()),
                   this, SLOT(onPodcastEpisodeDownloadCompleted()));
        disconnect(m_currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
                   this, SLOT(onDownloadProgress(qint64, qint64)));
        m_currentDownload->abort();

    }

    if (m_downloadFile){
        m_downloadFile->close();
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }
}

void PodcastEpisode::deleteDownload()
{
    if (m_playFilename.isEmpty()) {
        setFinished(true);
        return;
    } else {
        qDebug() << "Deleting locally downloaded podcast:" << m_playFilename;
    }

    QFile download(m_playFilename);
    if (!download.remove()) {
        QFileInfo fi(download);
        qWarning() << "Unable to remove locally downloaded podcast:" << fi.canonicalFilePath();
    }

    cancelCurrentDownload();
    setPlayFilename("");
    //setLastPlayed(QDateTime());
    setFinished(true);
    setState(PodcastEpisode::GetState);
    setHasBeenCanceled(true);             // TODO: This will denote to the UI not to download it again automatically. Better method name would be good.
}

void PodcastEpisode::setAsPlayed()
{
    setLastPlayed(QDateTime::currentDateTime());
}

void PodcastEpisode::setAsUnplayed()
{
    setFinished(false);
    setLastPlayed(QDateTime());
    setState(EpisodeStates::DownloadedState);
}

void PodcastEpisode::setAsFinished()
{
    setFinished(true);
    //    setState(EpisodeStates::FinishedState);
}


void PodcastEpisode::setAsUnFinished()
{
    setFinished(false);
    //    setState(EpisodeStates::FinishedState);
}

void PodcastEpisode::setFinished(bool finished)
{
    if (!m_finished == finished){
        m_finished = finished;
        emit episodeChanged();
    }
}


void PodcastEpisode::setPlayPosition(int playPosition){
    if (m_playPosition != playPosition){
        m_playPosition = playPosition;
        emit playPositionChanged(playPosition);
    }
}

void PodcastEpisode::setNew(bool episodeNew)
{
    if (episodeNew != m_new){
        m_new = episodeNew;
        emit episodeChanged();
    }
}

int PodcastEpisode::playPosition() const{
    return m_playPosition;
}

bool PodcastEpisode::episodeNew() const
{
    return m_new;
}

/*bool PodcastEpisode::isOnlyWebsiteUrl() const
{
    return (!isValidAudiofile() && QUrl(m_downloadLink).isValid());
}
*/
void PodcastEpisode::getAudioUrl()
{
    m_streamResolverTries = 0;
    m_streamResolverManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    QUrl url = this->downloadLink();
    if(url.userName().isEmpty()){
        url.setUserName(m_user);
        url.setPassword(m_password);
    }
    request.setUrl(this->downloadLink());

    QNetworkReply *reply = m_streamResolverManager->get(request);
    connect(reply, SIGNAL(metaDataChanged()),
            this,  SLOT(onAudioUrlMetadataChanged()));

}

QString PodcastEpisode::errorMessage() const
{
    return m_errorMessage;
}

void PodcastEpisode::onAudioUrlMetadataChanged()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    if (m_streamResolverTries >= 5) {
        qDebug() << "Did not find a proper audio URL to stream! Giving up after " << m_streamResolverTries << " tries.";
        emit streamingUrlResolved("", "");
        m_streamResolverManager->deleteLater();
    }

    if (isValidAudiofile(reply)) {
        emit streamingUrlResolved(reply->url().toString(), m_title);
        m_streamResolverManager->deleteLater();
    } else {
        QString redirectedUrl = PodcastManager::redirectedRequest(reply);

        if (QUrl(redirectedUrl).isValid()) {
            qDebug() << "We have been redirected...";
            QNetworkRequest request;
            request.setUrl(redirectedUrl);

            QNetworkReply *newReply = m_streamResolverManager->get(request);
            connect(newReply, SIGNAL(metaDataChanged()),
                    this,  SLOT(onAudioUrlMetadataChanged()));

            m_streamResolverTries++;

        } else {
            qDebug() << "Error resolving streaming URL!";
            emit streamingUrlResolved("", "");
            m_streamResolverManager->deleteLater();
        }
    }

    reply->deleteLater();
}

void PodcastEpisode::onDownloadError(QNetworkReply::NetworkError err)
{
    qDebug() << "While downloading " << m_title << " occured " << err;
}

void PodcastEpisode::onMetaDataChanged()
{
    qDebug() << m_currentDownload->rawHeaderList();
}

bool PodcastEpisode::isValidAudiofile(QNetworkReply *reply) const
{
    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

    qDebug() << "Content-Type is "<<contentType;

    if (contentType == "audio/mpeg" ||
            contentType == "audio/mpeg3" ||
            contentType == "audio/ogg"   ||
            contentType == "audio/x-ogg"   ||
            contentType == "audio/aac"   ||
            contentType  == "audio/x-m4a" ||
            contentType == "audio/mp4") {
        qDebug() << "Found audio URL to stream: " << reply->url();
        return true;
    }

    qDebug() << "No audio file found:" << reply->url();

    return false;
}

QString PodcastEpisode::getDownloadDir()
{
//if(!m_saveOnSDCOnf->value().toBool())
    auto conf = MGConfItem("/apps/ControlPanel/Podcatcher/saveOnSDCard", this);

    if (!conf.value().toBool())
        return PODCATCHER_PODCAST_DLDIR;
    else {
        QString sd = getSDPath();
        if(sd.isEmpty()){ //no SD mounted
            //m_saveOnSDCOnf->set(false);
            qWarning() << "SD not mounted.";
            //return "!SD";
            return QString();
            //return PODCATCHER_PODCAST_DLDIR;
        }

       return sd+"/podcasts/";
    }
}

QString PodcastEpisode::getSDPath()
{
    QStringList paths;
    char username[128];
    getlogin_r(username,128);

    paths << QString("/run/media/%1/").arg(username);
    paths <<  "/media/sdcard/";

    QString sd;

    foreach (const QString& path, paths) {

        QDir dir(path);
        if (dir.exists()){
            QStringList lst = dir.entryList(QDir::Dirs);

            foreach (const QString& s, lst) {
                if (s.startsWith("."))
                    continue;

                sd = QString("%1%2").arg(path).arg(s);
                return sd;
            }
        }
    }

    return sd;
}

