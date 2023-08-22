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
#include <QTimer>
#include <QtQuick>
#include <QtDebug>
#include <QCryptographicHash>
//#include <contentaction5/contentaction.h>


#include "podcatcherui.h"
#include "podcastepisodesmodel.h"
#include "podcastepisodesmodelfactory.h"
#include "podcastglobals.h"

PodcatcherUI::PodcatcherUI()
{
    view = SailfishApp::createView();
    m_channelsModel = m_pManager.podcastChannelsModel();
    view->rootContext()->setContextProperty("channelsModel", m_channelsModel);

    view->rootContext()->setContextProperty("mediaMetaDataExtractor", &m_mMDE);

    m_chapterModel = new PodcastChapterModel(this);
    m_chapterModel->setChapters(m_mMDE.chapters());

    connect(&m_mMDE, SIGNAL(chaptersChanged(QList<PodcastChapter>*)),
            m_chapterModel, SLOT(onChaptersChanged(QList<PodcastChapter>*)));

    view->rootContext()->setContextProperty("chapterModel", m_chapterModel);

    view->rootContext()->setContextProperty("ui", this);

    view->engine()->addImageProvider("coverArt", &m_mMDE.imageProvider);

    view->setSource(SailfishApp::pathTo("qml/Podcatcher.qml"));

    m_mediaPlayerConf = new MGConfItem("/apps/ControlPanel/Podcatcher/mediaplayer", this);
    m_mediaPlayerPath = m_mediaPlayerConf->value().toString();
    qDebug() << "  * Mediaplayer:" << m_mediaPlayerPath;

    connect(m_mediaPlayerConf, SIGNAL(valueChanged()),
            this, SLOT(onMediaPlayerChanged()));

    modelFactory = PodcastEpisodesModelFactory::episodesFactory();

    connect(&m_pManager, SIGNAL(showInfoBanner(QString)),
            this, SIGNAL(showInfoBanner(QString)));

    connect(&m_pManager, SIGNAL(downloadingPodcasts(bool)),
            this, SIGNAL(downloadingPodcasts(bool)));

    connect(&m_pManager, SIGNAL(downloadingPodcasts(bool)),
            this, SLOT(onDownloadingPodcast(bool)));

    QObject *rootDeclarativeItem = view->rootObject();

    connect(rootDeclarativeItem, SIGNAL(showChannel(QString)),
            this, SLOT(onShowChannel(QString)));

    connect(rootDeclarativeItem, SIGNAL(deleteChannel(QString)),
            this, SLOT(onDeleteChannel(QString)));

    connect(rootDeclarativeItem, SIGNAL(downloadPodcast(int, int)),
            this, SLOT(onDownloadPodcast(int, int)));

    connect(rootDeclarativeItem, SIGNAL(playPodcast(int, int)),
            this, SLOT(onPlayPodcast(int, int)));


    connect(rootDeclarativeItem, SIGNAL(addToPlayList(int, int)),
            this, SLOT(onAddToPlayList(int, int)));

    connect(rootDeclarativeItem, SIGNAL(cancelDownload(int, int)),
            this, SLOT(onCancelDownload(int, int)));

    connect(rootDeclarativeItem, SIGNAL(cancelQueue(int, int)),
            this, SLOT(onCancelQueueing(int, int)));

    connect(rootDeclarativeItem, SIGNAL(allListened(QString)),
            this, SLOT(onAllListened(QString)));

    connect(rootDeclarativeItem, SIGNAL(allFinished(QString)),
            this, SLOT(onAllFinished(QString)));

    connect(rootDeclarativeItem, SIGNAL(deleteDownloaded(int, int)),
            this, SLOT(onDeletePodcast(int, int)));

    connect(rootDeclarativeItem, SIGNAL(markAsUnplayed(int, int)),
            this, SLOT(onMarkAsUnplayed(int,int)));

    connect(rootDeclarativeItem, SIGNAL(markAsFinished(int, int)),
            this, SLOT(onMarkAsFinished(int,int)));

    connect(rootDeclarativeItem, SIGNAL(markAsUnFinished(int, int)),
            this, SLOT(onMarkAsUnFinished(int,int)));

    connect(rootDeclarativeItem, SIGNAL(startStreaming(int, int)),
            this, SLOT(onStartStreaming(int, int)));

    connect(rootDeclarativeItem, SIGNAL(refreshEpisodes(int)),
            this, SLOT(onRefreshEpisodes(int)));

    m_autoSyncConf = new MGConfItem("/apps/ControlPanel/Podcatcher/autosync", this);

    if (m_autoSyncConf->value().toBool()){
        m_pManager.refreshAllChannels();   // Refresh all feeds and download new episodes.
    }

    QTimer::singleShot(10000, &m_pManager, SLOT(cleanupEpisodes()));

    connect(rootDeclarativeItem, SIGNAL(autoDownloadChanged(int, bool)),
            this, SLOT(onAutoDownloadChanged(int, bool)));

    view->show();

    QString PODCATCHER_PATH =  QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    qDebug() << "Paths:\n" << PODCATCHER_PATH <<"\n" << PODCATCHER_PODCAST_DLDIR;

    /* NOT COMPATIBLE WITH SAILJAIL
    QFile jollaMediaPlayer("/usr/bin/jolla-mediaplayer");
    if (m_mediaPlayerPath.isEmpty() && !jollaMediaPlayer.exists()){
        emit showInfoBanner("Jolla Mediaplayer not installed. Playback of the podcasts might not work.");
    }

    */

    //m_playlist = new PodcastPlaylist(this);

}

void PodcatcherUI::addPodcast(const QString& rssUrl, const QString& logoUrl)
{
    qDebug() << "Current Thread" << QThread::currentThread();
    qDebug() << "PodcatcherUI thread" << this->thread();

    if (!logoUrl.isEmpty()) {
        qDebug() << "Got logo from the subscription feed:" << logoUrl;
        logoCache.insert(rssUrl, logoUrl);
    }

    QString newPodcast = rssUrl/*.toLower()*/;
    if (newPodcast.indexOf(QString("http://")) != 0 && newPodcast.indexOf(QString("https://"))) {
        newPodcast.prepend("http://");
    }
    qDebug() << "User entered podcast to fetch: " << rssUrl << " - fetching " << newPodcast;

    m_pManager.requestPodcastChannel(QUrl(newPodcast), logoCache);

    emit showInfoBanner(tr("Fetching channel information..."));
}

bool PodcatcherUI::isDownloading()
{
    return m_pManager.isDownloading();
}


void PodcatcherUI::onShowChannel(const QString& channelId)
{
    qDebug() << "Opening channel" << channelId;

    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId.toInt());
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    /*
    if (channel->description().length() > 270) {
        QString oldDesc = channel->description();
        oldDesc.truncate(270);
        QString newDesc = QString("%1%2").arg(oldDesc).arg("...");

        channel->setDescription(newDesc);
    }
    */

    view->rootContext()->setContextProperty("channel", channel);

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    // Sort when the episodes page is shown. We do not sort when the downloading state etc. of an episode has changed
    episodesModel->sortEpisodes();
    view->rootContext()->setContextProperty("episodesModel", episodesModel);
}


void PodcatcherUI::onDownloadPodcast(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);
    m_pManager.downloadPodcast(episode);
}

void PodcatcherUI::onPlayPodcast(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    if (episodesModel == nullptr) {
        qWarning() << "Could not get episodes model. Cannot play episode.";
        return;
    }

    PodcastEpisode *episode = episodesModel->episode(index);

    QUrl file = QUrl::fromLocalFile(episode->playFilename());

    // If the file doens't exist, update the state in the DB
    // and do nothing more.
    QFile checkFile(file.toLocalFile());

    if (!checkFile.exists()) {
        if (file.toLocalFile().startsWith("/media/sdcard") || file.toLocalFile().startsWith("/run/media")){
            emit showInfoBanner(tr("Podcast episode not found.  Make sure SD card is mounted and decrypted!"));
            return;
        }else{
            qDebug() << "Original file " << file.toLocalFile() << " doesn't exist anymore.";
            episode->setPlayFilename("");
            episode->setState(PodcastEpisode::GetState);
            episode->setLastPlayed(QDateTime());
            episodesModel->refreshEpisode(episode);

            emit showInfoBanner(tr("Podcast episode not found."));
            return;
        }
    }

    episode->setLastPlayed(QDateTime::currentDateTime());
    episodesModel->refreshEpisode(episode);
    m_channelsModel->refreshChannel(channelId);

    qDebug() << "Launching the music player for file" << file.fileName();

    if (!m_mediaPlayerPath.isEmpty() && m_mediaPlayerPath != "internal"){
        QFile player(m_mediaPlayerPath);
        if (!player.exists()){
            emit showInfoBanner(tr("Mediaplayer program not found!"));
            return;
        }

        QString cmd = m_mediaPlayerPath + " " + file.fileName() + " &";
        qDebug() << "Running "<< cmd;
        //QProcess::startDetached(m_mediaPlayerPath + " " + file.fileName());
        system(cmd.toLocal8Bit());


    }else if (m_mediaPlayerPath.isEmpty()){
        QFile jollaMediaPlayer("/usr/bin/jolla-mediaplayer");
        if (!jollaMediaPlayer.exists()){
            emit showInfoBanner("Jolla Mediaplayer not installed. Playback of the episode might not work.");
        }
        if (! QDesktopServices::openUrl(file)){
            emit showInfoBanner(tr("I am sorry! Could not launch audio player for this podcast."));
        }
    }else{
        emit playFileWithInternalPlayer(episode->playFilename());
    }
}

void PodcatcherUI::onAddToPlayList(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    if (episodesModel == nullptr) {
        qWarning() << "Could not get episodes model. Cannot add episode to playlist.";
        return;
    }

    PodcastEpisode *episode = episodesModel->episode(index);

    //m_playlist->addEpisode(episode);
}

void PodcatcherUI::onDownloadingPodcast(bool _isDownloading)
{
    qDebug() << "isDownloading changed" << _isDownloading;
    emit isDownloadingChanged(_isDownloading);
}

void PodcatcherUI::onRefreshEpisodes(int channelId)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    qDebug() << "Refreshing channel: " << channelId << channel->title();
    if (channel == nullptr) {
        qWarning() << "Got NULL episode!";
        return;
    }
    m_pManager.refreshPodcastChannelEpisodes(channel, true);
}

void PodcatcherUI::onCancelQueueing(int channelId, int index)
{

    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    qDebug() << "Cancel queueing at " << channelId << index;
    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);
    episode->setState(PodcastEpisode::GetState);

    m_pManager.cancelQueueingPodcast(episode);

    episodesModel->refreshEpisode(episode);
}

void PodcatcherUI::onCancelDownload(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);
    m_pManager.cancelDownloadPodcast(episode);
    episodesModel->refreshEpisode(episode);
}

void PodcatcherUI::onDeleteChannel(const QString& channelId)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId.toInt());
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    qDebug() << "Yep, lets delete the channel and some episodes from channel" << channelId;
    m_pManager.removePodcastChannel(*channel);

}

void PodcatcherUI::onAllListened(const QString& channelId)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId.toInt());
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    qDebug() << "Yep, mark all listened on channel: " << channelId;

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    QList<PodcastEpisode *> unplayed = episodesModel->unplayedEpisodes();
    foreach(PodcastEpisode *episode, unplayed) {
        episode->setAsPlayed();
        episodesModel->refreshEpisode(episode);
    }

    m_channelsModel->refreshChannel(channelId.toInt());
}

void PodcatcherUI::onAllFinished(const QString &channelId)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId.toInt());
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    qDebug() << "Yep, mark all listened on channel: " << channelId;

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    QList<PodcastEpisode *> unfinished = episodesModel->unfinishedEpisodes();
    foreach(PodcastEpisode *episode, unfinished) {
        episode->setAsFinished();
        episodesModel->refreshEpisode(episode);
    }

    m_channelsModel->refreshChannel(channelId.toInt());
}

void PodcatcherUI::onDeletePodcast(int channelId, int index)
{
    qDebug() << "Deleting the locally downloaded podcast:" << channelId << index;

    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);
    qDebug() << "Episode name:" << episode->title() << episode->playFilename();

    QString hash = QCryptographicHash::hash(episode->playFilename().toLocal8Bit(), QCryptographicHash::Md5).toHex();
    MGConfItem playerPosition("/apps/ControlPanel/Podcatcher/position/"+hash, this);
    qDebug() << "Saved play position was " << playerPosition.value().toInt();
    playerPosition.unset();

    episode->deleteDownload();
    episodesModel->refreshEpisode(episode);

    m_channelsModel->refreshChannel(channelId);
}

void PodcatcherUI::onMarkAsUnplayed(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);

    episode->setAsUnplayed();
    episodesModel->refreshEpisode(episode);
}

void PodcatcherUI::onMarkAsFinished(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);

    episode->setAsFinished();
    episodesModel->refreshEpisode(episode);
}

void PodcatcherUI::onMarkAsUnFinished(int channelId, int index)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);

    episode->setAsUnFinished();
    episodesModel->refreshEpisode(episode);
}



void PodcatcherUI::deletePodcasts(int channelId)
{
    PodcastChannel * channel = m_pManager.podcastChannel(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    m_pManager.deleteAllDownloadedPodcasts(*channel);
    m_channelsModel->refreshChannel(channelId);
}

void PodcatcherUI::onStartStreaming(int channelId, int index)
{
    qDebug() << "Requested streaming of episode:" << channelId << index;

    PodcastChannel * channel = m_pManager.podcastChannel(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    PodcastEpisodesModel *episodesModel = modelFactory->episodesModel(*channel);
    PodcastEpisode *episode = episodesModel->episode(index);

    connect(episode, SIGNAL(streamingUrlResolved(QString, QString)),
            this, SLOT(onStreamingUrlResolved(QString, QString)));

    qDebug() << "Episode url:" << episode->downloadLink() << ", need to find a MP3 file for this link.";
    episode->getAudioUrl();
}

void PodcatcherUI::onStreamingUrlResolved(const QString& streamUrl, const QString& streamTitle)
{
    auto *episode = qobject_cast<PodcastEpisode *>(sender());
    disconnect(episode, SIGNAL(streamingUrlResolved(QString, QString)),
               this, SLOT(onStreamingUrlResolved(QString, QString)));

    if (streamUrl.isEmpty()) {
        emit showInfoBanner(tr("Unable to stream podcast."));
    } else {
        emit streamingUrlResolved(streamUrl, streamTitle);
    }
}

void PodcatcherUI::onAutoDownloadChanged(int channelId, bool autoDownload)
{
    PodcastChannel * channel = m_channelsModel->podcastChannelById(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }
    channel->setAutoDownloadOn(autoDownload);
    m_channelsModel->updateChannel(channel);
}

void PodcatcherUI::onMediaPlayerChanged(){
    m_mediaPlayerPath = m_mediaPlayerConf->value().toString();
    qDebug() << "Setting changed: Mediaplayer: " << m_mediaPlayerPath;
}

void PodcatcherUI::importFromGPodder(const QString& username, const QString& password)
{
    m_pManager.fetchSubscriptionsFromGPodder(username, password);
}

void PodcatcherUI::changeFeedURLIfValid(int channelId, const QString &url)
{
    PodcastChannel * channel = m_pManager.podcastChannel(channelId);
    if (!channel){
        qWarning() << "Could not get channel"  << channelId;
        return;
    }

    channel->setTrialUrl(url);
    m_pManager.refreshPodcastChannelEpisodes(channel, true, true);
}

bool PodcatcherUI::isConnectedToWiFi()
{
   return PodcastManager::isConnectedToWiFi();
}



bool PodcatcherUI::isLiteVersion()
{
#ifdef LITE
    return true;
#else
    return false;
#endif
}

QString PodcatcherUI::versionString()
{
    QString tmpVersion(QString::number(PODCATCHER_VERSION));
    QString version = tmpVersion.at(0);
    version.append(".").append(tmpVersion.at(1));
    version.append(".");
    for (int i=2; i<tmpVersion.length(); i++) {
        version.append(tmpVersion.at(i));
    }

    return version;
}

void PodcatcherUI::refreshChannels()
{
    m_pManager.refreshAllChannels();
}


