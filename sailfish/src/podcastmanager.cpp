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
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QAuthenticator>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QImage>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QSettings>

#include <QtDebug>
#include <QCryptographicHash>
#include <QtConcurrentRun>
#include <QFuture>

#include <QElapsedTimer>

#include "podcastmanager.h"
#include "podcastsqlmanager.h"
#include "podcastrssparser.h"
#include "podcastrssparser2.h"
#include "podcastglobals.h"

PodcastManager::PodcastManager(QObject *parent) :
    QObject(parent),
    m_channelsModel(new PodcastChannelsModel(this)),
    m_networkManager(new QNetworkAccessManager(this)),
    m_dlNetworkManager(new QNetworkAccessManager(this)),
    m_episodeModelFactory(PodcastEpisodesModelFactory::episodesFactory()),
    //m_isDownloading(false),
    m_autodownloadNumSettings(1),
    m_keepNumEpisodesSettings(0),
    m_autodownloadOnSettings(false),
    m_autoDelUnplayedSettings(false)
{

    connect(this, SIGNAL(podcastChannelReady(PodcastChannel*)),
            this, SLOT(savePodcastChannel(PodcastChannel*)));

    connect(this,SIGNAL(podcastEpisodeDownloadReady()),
            this,SLOT(onPodcastEpisodeDownloadReady()));

    connect(m_networkManager, SIGNAL(networkSessionConnected()),
            this, SLOT(onSessionConnected()));

    connect(m_networkManager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)),
            this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onRequestFinished(QNetworkReply*)));

    // Get the current settings values.
    qDebug() << "Current settings: ";
    m_autoSyncConf = new MGConfItem("/apps/ControlPanel/Podcatcher/autosync", this);
    m_autoSyncSettings = m_autoSyncConf->value().toBool();
    qDebug() << "  * AutoSync on:" << m_autodownloadOnSettings;

    m_autoDlConf = new MGConfItem("/apps/ControlPanel/Podcatcher/autodownload", this);
    m_autodownloadOnSettings = m_autoDlConf->value().toBool();
    qDebug() << "  * Autodownload on:" << m_autodownloadOnSettings;

    m_autoDlNumConf = new MGConfItem("/apps/ControlPanel/Podcatcher/autodownload_num", this);
    m_autodownloadNumSettings = m_autoDlNumConf->value().toInt();
    qDebug() << "  * Automatically get episodes: " << m_autodownloadNumSettings;

    m_keepNumEpisodesConf = new MGConfItem("/apps/ControlPanel/Podcatcher/keep_episodes", this);
    m_keepNumEpisodesSettings = m_keepNumEpisodesConf->value().toInt();
    qDebug() << "  * Autodelete episodes after num days:" << m_keepNumEpisodesSettings;

    m_autoDelUnplayedConf = new MGConfItem("/apps/ControlPanel/Podcatcher/keep_unplayed", this);
    m_autoDelUnplayedSettings = m_autoDelUnplayedConf->value().toBool();
    qDebug() << "  * Keep unplayed episodes:" << m_autoDelUnplayedSettings;

    // Connect to the changed signals for each of the settings above.
    connect(m_autoDlConf, SIGNAL(valueChanged()),
            this, SLOT(onAutodownloadOnChanged()));
    connect(m_autoDlNumConf, SIGNAL(valueChanged()),
            this, SLOT(onAutodownloadNumChanged()));
    connect(m_keepNumEpisodesConf, SIGNAL(valueChanged()),
            this, SLOT(onAutodelDaysChanged()));
    connect(m_autoDelUnplayedConf, SIGNAL(valueChanged()),
            this, SLOT(onAutodelUnplayedChanged()));


    updateAutoDLSettingsFromCache();



    //qDebug() << "We are in Thread" << QThread::currentThreadId();
}

PodcastChannelsModel * PodcastManager::podcastChannelsModel() const
{
    return m_channelsModel;
}

void PodcastManager::requestPodcastChannel(const QUrl &rssUrl, const QMap<QString, QString> &logoCache)
{
    qDebug() << "Requesting Podcast channel" << rssUrl;
    qDebug() << "Current Thread" << QThread::currentThread();
    qDebug() << "PodcastManager thread" << this->thread();

    m_logoCache = logoCache;

    if (!rssUrl.isValid()) {
        qWarning() << "Provided podcast channel URL is not valid.";
        QString faultyUrlSnippet = rssUrl.toString().left(10);
        emit showInfoBanner(tr("Unable to add subscription from %1 …").arg(faultyUrlSnippet));
        return;
    }

    auto *channel = new PodcastChannel(this);
    channel->setUrl(rssUrl.toString());

    if (m_channelsModel->channelAlreadyExists(channel)) {
        qDebug() << "Channel is already in DB. Not doing anything.";
        emit showInfoBanner(tr("Already subscribed to '%1'.").arg(channel->title()));
        delete channel;
        return;
    }

    channelRequestMap.insert(rssUrl.toString(), channel);

    QNetworkRequest request;
    request.setRawHeader("User-Agent", "Podcatcher Sailfish Podcast client");
    request.setRawHeader("Connection", "close");
    request.setUrl(rssUrl);

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, SIGNAL(finished()),
            this, SLOT(onPodcastChannelCompleted()));
}

void PodcastManager::refreshAllChannels()
{
    qDebug() << "\n ********* Refresh episodes for all channels ******** \n";

    //emit showInfoBanner(tr("Refreshing episodes..."));

    foreach(PodcastChannel *channel, m_channelsModel->channels()) {
        int channelid = channel->channelDbId();
        /*PodcastChannel */ channel = podcastChannel(channelid);

        qDebug() << "Refreshing channel: " << channelid << channel->title();

        if (channel == nullptr) {
            qWarning() << "Got NULL episode!";
            break;
        }
        refreshPodcastChannelEpisodes(channel, true);
    }
}


void PodcastManager::refreshPodcastChannelEpisodes(PodcastChannel *channel, bool forceNetwork, bool trial)
{

    if (!forceNetwork) {
        // No need to fetch from the net anything.
        savePodcastEpisodes(channel);
        return;
    }

    channel->setIsRefreshing(true);

    qDebug() << "Forced to get new episode data from the network.";

    QUrl rssUrl(trial? channel->trialURL(): channel->url());

        qDebug() << "Requesting Podcast channel episodes" << rssUrl.toString();

    if (!rssUrl.isValid()) {

        if (trial){
            channel->trialFailed();
        }

        qWarning() << "Provided podcast channel URL is not valid.";
        return;
    }

    QNetworkRequest request;
    request.setRawHeader("User-Agent", "Podcatcher Podcast client");
    request.setUrl(rssUrl);


    QNetworkReply *reply = m_networkManager->get(request);

    insertChannelForNetworkReply(reply, channel);

    connect(reply, SIGNAL(finished()),
            this, SLOT(onPodcastEpisodesRequestCompleted()));

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onPodcastEpisodesRequestError(QNetworkReply::NetworkError)));
}


PodcastChannel * PodcastManager::podcastChannel(int id)
{
    PodcastChannel *channel = nullptr;
    if (!m_channelsCache.contains(id)) {
        channel = m_channelsModel->podcastChannelById(id);
        if (channel == nullptr) {
            return channel;
        }
        m_channelsCache.insert(id, channel);
    } else {
        channel = m_channelsCache.value(id);
    }

    return channel;
}

void PodcastManager::downloadPodcast(PodcastEpisode *episode)
{
    qDebug() << "Episode" << episode->dbid() << "queued for downloading.";

    m_episodeDownloadQueue.append(episode);
    episode->setState(PodcastEpisode::QueuedState);
    emit podcastEpisodeDownloadReady();
    //executeNextDownload();
}

void PodcastManager::cancelQueueingPodcast(PodcastEpisode *episode)
{
    qDebug() << "Canceling queueing of episode:" << episode->title();

    if (m_episodeDownloadQueue.contains(episode)) {
        m_episodeDownloadQueue.removeOne(episode);
    } else {
        qWarning() << "Canceled episode was not in the queue.";
    }

    if (m_currentEpisodeDownloads.contains(episode)) {
        m_currentEpisodeDownloads.removeOne(episode);
    } else {
        qWarning() << "Canceled episode was not in the queue.";
    }
}

void PodcastManager::cancelDownloadPodcast(PodcastEpisode *episode)
{
    qDebug() << "Canceling download of episode:" << episode->title();

    episode->cancelCurrentDownload();

    if (m_episodeDownloadQueue.contains(episode)) {
        m_episodeDownloadQueue.removeOne(episode);
    } else {
        qWarning() << "Canceled episode was not in the queue.";
    }

    PodcastChannel *channel = m_channelsModel->podcastChannelById(episode->channelid());
    //channel->setIsDownloading(false);


    if(m_currentEpisodeDownloads.contains(episode)){
        m_currentEpisodeDownloads.removeOne(episode);
    }

    bool otherDownloadfromChannel = false;

    for (PodcastEpisode* other : std::as_const(m_currentEpisodeDownloads)) {
        if (other->channelid() == episode->channelid()){
            otherDownloadfromChannel = true;
            break;
        }
    }

    channel->setIsDownloading(otherDownloadfromChannel);

    emit downloadingPodcasts(isDownloading());
    //m_isDownloading = false;
    //executeNextDownload();
}


/********************************************************************************
 * Private implementations.
 ********************************************************************************/

void PodcastManager::onPodcastChannelCompleted()
{
    qDebug() << "Podcast network request completed.";
    qDebug() << "Current Thread" << QThread::currentThread();
    qDebug() << "PodcastManager thread" << this->thread();

    auto *reply = qobject_cast<QNetworkReply *>(sender());
    QString redirectedUrl = PodcastManager::redirectedRequest(reply);
    if (!redirectedUrl.isEmpty()) {
        requestPodcastChannel(QUrl(redirectedUrl));
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError){
        emit showInfoBanner(reply->errorString());
        qDebug() << "An error occured: " << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->close();

    if (data.size() < 1) {
        qDebug() << "No data in the network reply. Aborting";
        //emit showInfoBanner(tr("Unable to add subscription from that location"));
        emit showInfoBanner(tr("No data received."));
        reply->deleteLater();
        return;
    }

    PodcastChannel *channel = channelRequestMap.value(reply->url().toString());

    QString readyLogoUrl = m_logoCache.value(reply->url().toString());
    if (!readyLogoUrl.isEmpty()) {
        qDebug() << "Got logo from subscription information. Setting it." << readyLogoUrl;
        channel->setLogoUrl(readyLogoUrl);
        m_logoCache.remove(reply->url().toString());
    }

    channel->setXml(data);
    channelRequestMap.remove(reply->url().toString());

    reply->deleteLater();


    /*    if (PodcastRSSParser::isValidPodcastFeed(data) == false) {
        qDebug() << "Podcast feed is not valid! Not adding data to DB...";
        emit showInfoBanner("Podcast feed is not valid. Cannot add subscription...");
        return;
    }
*/

    bool rssOk;
    rssOk = PodcastRSSParser2::populateChannelFromChannelXML(channel,
                                                             channel->xml());
    if (!rssOk) {
        emit showInfoBanner(tr("Podcast feed is not valid. Cannot add subscription..."));
        return;
    }

    // Cache the channel logo locally on file system.
    if (!channel->logoUrl().isEmpty()) {
        QNetworkReply *logoReply = downloadChannelLogo(channel->logoUrl());
        // Use a local map to map data matching this network request to the reply
        // we get.
        insertChannelForNetworkReply(logoReply, channel);
    } else {
        emit podcastChannelReady(channel);
    }
}

void PodcastManager::onPodcastChannelLogoCompleted() {

    qDebug() << "Podcast channel logo network request completed";

    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == nullptr) {
        qWarning() << "Network reply is 0. Aborting.";
        //reply->deleteLater();
        return;
    }

    QString redirectedUrl = redirectedRequest(reply);
    if (!redirectedUrl.isEmpty()) {
        PodcastChannel *channel = channelForNetworkReply(reply);

        QNetworkReply *logoReply = downloadChannelLogo(redirectedUrl);
        reply->deleteLater();

        insertChannelForNetworkReply(logoReply, channel);

        return;
    }

    if (reply->bytesAvailable() < 1) {
        qWarning() << "Got no data from the network request when downloading the logo";
        emit showInfoBanner(reply->errorString());
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }

    PodcastChannel *channel = channelForNetworkReply(reply);
    reply->deleteLater();
    QString channelTitle = channel->title();

    // Use a MD5 hash of the channel name as the logo name that is stored locally.
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(channelTitle.toLatin1());
    QString localFilename = hash.result().toHex();
    qDebug() << "Hash for title " << channelTitle << "=>" << localFilename <<". Using it for cached logo image.";

    // Construct the local QImage from the network data.
    QByteArray imageData = reply->readAll();
    QImage channelLogo = QImage::fromData(imageData);

    reply->close();

    QString  PODCATCHER_PATH = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    //QString filename = PODCATCHER_PATH + localFilename + ".jpg";
    QString filename = PODCATCHER_PATH + "/" + localFilename + ".png";
    qDebug() << "Saving channel logo locally to: " << filename;

    if (channelLogo.isNull()) {
        qWarning() << "Image is NULL";
    }

    if (!channelLogo.save(filename)) {
        qWarning() << "Could not save image: " << filename;
    }

    channel->setLogo(QUrl::fromLocalFile(filename).toString());

    emit podcastChannelReady(channel);
}

void PodcastManager::onPodcastEpisodesRequestCompleted()
{
    qDebug() << "Podcast channel refresh finished";

    auto *reply = qobject_cast<QNetworkReply *>(sender());

    QObject::connect(reply, SIGNAL(destroyed(QObject*)), this,
                     SLOT(onDestroyingNetworkReply(QObject*)));

    PodcastChannel *channel = channelForNetworkReply(reply);
    if (channel == nullptr) {
        qWarning() << "Podcast channel from reply is NULL! Doing nothing.";
        reply->deleteLater();
        return;
    }

    qDebug() << "Status Code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    bool trial = reply->url() == channel->trialURL();


    QString redirectedUrl = redirectedRequest(reply);
    if (!redirectedUrl.isEmpty()) {
        // TODO: Update feed URL if permanent redirect
        reply->deleteLater();

        QNetworkRequest request;
        request.setRawHeader("User-Agent", "Podcatcher Podcast client");
        request.setUrl(redirectedUrl);

        if (trial){
            channel->setTrialUrl(redirectedUrl);
        }


        QNetworkReply *reply = m_networkManager->get(request);

        insertChannelForNetworkReply(reply, channel);

        connect(reply, SIGNAL(finished()),
                this, SLOT(onPodcastEpisodesRequestCompleted()));

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(onPodcastEpisodesRequestError(QNetworkReply::NetworkError)));


        return;
    }


    if (reply->error() != QNetworkReply::NoError){
        emit showInfoBanner(reply->errorString());
        if(trial){
            channel->trialFailed();
        }
        reply->deleteLater();
        return;
    }



    if (reply != nullptr) {
        QByteArray episodeXmlData = reply->readAll();
        channel->setXml(episodeXmlData);
    }

    reply->deleteLater();

    if(trial){
        if(PodcastRSSParser2::isValidPodcastFeed(channel->xml())){
        channel->trialSucceeded();}
        else{
            channel->trialFailed();
        }
    }

     savePodcastEpisodes(channel);
}

void PodcastManager::onPodcastEpisodesRequestError(QNetworkReply::NetworkError error)
{
    if (error != QNetworkReply::NoError) {
        emit showInfoBanner(tr("Cannot refresh. Network error."));
    }


    auto *reply = qobject_cast<QNetworkReply *>(sender());

    PodcastChannel *channel = channelForNetworkReply(reply);
    if (channel == nullptr) {
        qWarning() << "Podcast channel from reply is NULL! Doing nothing.";
        reply->deleteLater();
        return;
    }

    if (reply->url() == channel->trialURL()){
        channel->trialFailed();
    }

    channel->setIsRefreshing(false);
}

void PodcastManager::onPodcastEpisodesParsed()
{
    QList<PodcastEpisode*>* parsedEpisodes;


    auto* watcher =  dynamic_cast<QFutureWatcher<QList<PodcastEpisode*>*>*>(sender());

    parsedEpisodes = watcher->result();
    PodcastChannel* channel = channelForFutureWatcher(watcher);

    watcher->deleteLater();

    if (!parsedEpisodes) {
        emit showInfoBanner(tr("Podcast feed invalid. Cannot download episodes for '%1'.").arg(channel->title()));
    }else{


        //        for (PodcastEpisode* e: *parsedEpisodes){
        //            e->moveToThread(this->thread());
        //        }

        PodcastEpisodesModel *episodeModel = m_episodeModelFactory->episodesModel(*channel);  // FIXME: Pass only channel to episodes model - not the DB id.
        episodeModel->addEpisodes(*parsedEpisodes);

        qDebug() << "Downloading automatically new episodes:" << m_autodownloadOnSettings << " WiFi:" << PodcastManager::isConnectedToWiFi();

        // Automatically download new episodes in the channel if
        //  - If podcast channel has the auto-download enabled (which is controlled by the Podcatcher Settings too).
        //  - We are connected to the WiFi
        if (PodcastManager::isConnectedToWiFi() &&
                channel->isAutoDownloadOn()) {
            downloadNewEpisodes(episodeModel->channel());
        }
    }
    channel->setIsRefreshing(false);
}

void PodcastManager::onPodcastEpisodeDownloadReady()
{
    while (!m_episodeDownloadQueue.isEmpty()) {
        qApp->processEvents();
        emit downloadingPodcasts(true);  // Notify the UI

        //m_isDownloading = true;
        PodcastEpisode *episode = m_episodeDownloadQueue.first();
        m_episodeDownloadQueue.removeOne(episode);

        qDebug() << "Starting a new download..." << episode->title();

        connect(episode, SIGNAL(podcastEpisodeDownloaded(PodcastEpisode*)),
                this, SLOT(onPodcastEpisodeDownloaded(PodcastEpisode*)));

        connect(episode, SIGNAL(podcastEpisodeDownloadFailed(PodcastEpisode*)),
                this, SLOT(onPodcastEpisodeDownloadFailed(PodcastEpisode*)));

        PodcastChannel *channel = m_channelsModel->podcastChannelById(episode->channelid());
        channel->setIsDownloading(true);

        channel->addCredentials(episode);

        episode->setState(PodcastEpisode::DownloadingState);
        episode->setHasBeenCanceled(false);
        episode->setDownloadManager(m_dlNetworkManager);
        //using same QNAM does not fix problem with hanging downloads
        //episode->setDownloadManager(m_networkManager);
        episode->downloadEpisode();

        m_currentEpisodeDownloads.append(episode);

    }
}



bool PodcastManager::savePodcastEpisodes(PodcastChannel *channel)
{
    QByteArray episodeXmlData = channel->xml();

    //    QList<PodcastEpisode *> *parsedEpisodes;
    //    parsedEpisodes = PodcastRSSParser::populateEpisodesFromChannelXML(episodeXmlData);


    auto *watcher = new QFutureWatcher<QList<PodcastEpisode *>*>(this);

    watcher->setFuture(QtConcurrent::run(PodcastRSSParser2::populateEpisodesFromChannelXML,episodeXmlData, channel));

    insertChannelForFutureWatcher(watcher,channel);

    QObject::connect(watcher,SIGNAL(finished()), this, SLOT(onPodcastEpisodesParsed()));

    /*
    qDebug() << "Checking parser result for channel " << channel->title();

    QElapsedTimer timer;
    timer.start();

    QList<PodcastEpisode *>* one = PodcastRSSParser::populateEpisodesFromChannelXML(episodeXmlData, channel);
    int time1 = timer.elapsed();


    timer.start();
    QList<PodcastEpisode *>* two =  PodcastRSSParser2::populateEpisodesFromChannelXML(episodeXmlData, channel);
    int time2 = timer.elapsed();

    qDebug() << "The old parser needs "<<time1 << "µs, the new parser "<< time2 <<"µs.";

    compareEpisodes(one, two);
    */

    return true;

}

void PodcastManager::downloadNewEpisodes(PodcastChannel& channel) {
    PodcastEpisodesModel *episodesModel = m_episodeModelFactory->episodesModel(channel);

    qDebug() << "Downloading new episodes for channel: " << channel.channelDbId();

    // If the settings value for "get number of episodes" == 0, then we fetch "all episodes". So set 999.
    // Otherwise use the number as specified in the settings.
    int downloadEpisodes = (m_autodownloadNumSettings == 0) ? 999 : m_autodownloadNumSettings;
    QList<PodcastEpisode *> episodes = episodesModel->undownloadedEpisodes(downloadEpisodes);
    foreach(PodcastEpisode *episode, episodes) {
        qDebug() << "Downloading podcast:" << episode->downloadLink();
        downloadPodcast(episode);
    }
}


void PodcastManager::savePodcastChannel(PodcastChannel *channel)
{
    //channel->setAutoDownloadOn(m_autodownloadOnSettings);
    channel->setAutoDownloadOn(false);

    qDebug() << "Adding channel to DB:" << channel->title();
    m_channelsModel->addChannel(channel);

    qDebug() << "Podcast channel saved to DB. Refreshing episodes...";
    refreshPodcastChannelEpisodes(channel);

    emit podcastChannelSaved();
}

void PodcastManager::onPodcastEpisodeDownloaded(PodcastEpisode *episode)
{
    qDebug() << "Download completed...";

    disconnect(episode, SIGNAL(podcastEpisodeDownloaded(PodcastEpisode*)),
               this, SLOT(onPodcastEpisodeDownloaded(PodcastEpisode*)));

    disconnect(episode, SIGNAL(podcastEpisodeDownloadFailed(PodcastEpisode*)),
               this, SLOT(onPodcastEpisodeDownloadFailed(PodcastEpisode*)));

    episode->setState(PodcastEpisode::DownloadedState);

    PodcastEpisodesModel *episodeModel = m_episodeModelFactory->episodesModel(*episode->channel());
    episodeModel->refreshEpisode(episode);
    m_channelsModel->refreshChannel(episode->channelid());

    if (m_currentEpisodeDownloads.contains(episode)){
        m_currentEpisodeDownloads.removeOne(episode);
    }

    PodcastChannel *channel = m_channelsModel->podcastChannelById(episode->channelid());

    bool otherDownloadfromChannel = false;

    for (PodcastEpisode* other : std::as_const(m_currentEpisodeDownloads)) {
        if (other->channelid() == episode->channelid()){
            otherDownloadfromChannel = true;
            break;
        }
    }

    channel->setIsDownloading(otherDownloadfromChannel);

    emit podcastEpisodeDownloaded(episode);

    //m_isDownloading = false;

    //    if (m_episodeDownloadQueue.contains(episode)) {
    //        m_episodeDownloadQueue.removeOne(episode);
    //    }


    emit downloadingPodcasts(isDownloading());

    //executeNextDownload();
}

void PodcastManager::onPodcastEpisodeDownloadFailed(PodcastEpisode *episode)
{
    qDebug() << "Download failed...";

    disconnect(episode, SIGNAL(podcastEpisodeDownloaded(PodcastEpisode*)),
               this, SLOT(onPodcastEpisodeDownloaded(PodcastEpisode*)));

    disconnect(episode, SIGNAL(podcastEpisodeDownloadFailed(PodcastEpisode*)),
               this, SLOT(onPodcastEpisodeDownloadFailed(PodcastEpisode*)));

    if (isDownloading()) {
        emit showInfoBanner(tr("Podcast episode download failed: ")+episode->errorMessage());
    }

    //m_isDownloading = false;
    PodcastChannel *channel = m_channelsModel->podcastChannelById(episode->channelid());
    channel->setIsDownloading(false);
    episode->setState(PodcastEpisode::GetState);

    //    if (m_episodeDownloadQueue.contains(episode)) {
    //        m_episodeDownloadQueue.removeOne(episode);
    //    }

    if (m_currentEpisodeDownloads.contains(episode)){
        m_currentEpisodeDownloads.removeOne(episode);
    }

    emit downloadingPodcasts(isDownloading());
    //executeNextDownload();
}

//void PodcastManager::executeNextDownload()
//{
//    if (!m_isDownloading && !m_episodeDownloadQueue.isEmpty()) {
//        emit downloadingPodcasts(true);  // Notify the UI

//        m_isDownloading = true;
//        PodcastEpisode *episode = m_episodeDownloadQueue.first();

//        qDebug() << "Starting a new download..." << episode->title();

//        connect(episode, SIGNAL(podcastEpisodeDownloaded(PodcastEpisode*)),
//                this, SLOT(onPodcastEpisodeDownloaded(PodcastEpisode*)));

//        connect(episode, SIGNAL(podcastEpisodeDownloadFailed(PodcastEpisode*)),
//                this, SLOT(onPodcastEpisodeDownloadFailed(PodcastEpisode*)));

//        PodcastChannel *channel = m_channelsModel->podcastChannelById(episode->channelid());
//        channel->setIsDownloading(true);

//        channel->addCredentials(episode);

//        episode->setState(PodcastEpisode::DownloadingState);
//        episode->setHasBeenCanceled(false);
//        episode->setDownloadManager(m_dlNetworkManager);
//        //using same NAM does not fix problem with hanging downloads
//        //episode->setDownloadManager(m_networkManager);
//        episode->downloadEpisode();
//    } else {
//        if (m_episodeDownloadQueue.isEmpty()) {
//            emit downloadingPodcasts(false); // Notify the UI
//        }
//    }
//}

QString PodcastManager::redirectedRequest(QNetworkReply *reply)
{
     QUrl possibleRedirectUrl;
     QUrl base = reply->url();

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 308){
        QByteArray header = reply->rawHeader("location");
          possibleRedirectUrl= base.resolved(QUrl(QString::fromUtf8(header)));
    }else{
        QString target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        if (target.length() >0)
            possibleRedirectUrl = base.resolved(QUrl(target));
    }

    qDebug() << "Possible redirect URL: "<< possibleRedirectUrl;

    if (possibleRedirectUrl.isValid()) {
    //QUrl redirectedUrl = QUrl::fromUserInput(possibleRedirectUrl.toString());
        qDebug() << "We have been redirected. New URL is " << possibleRedirectUrl;
        return possibleRedirectUrl.toString();
    }

    return QString();
}

QNetworkReply * PodcastManager::downloadChannelLogo(const QString& logoUrl)
{
    QNetworkRequest r;
    r.setUrl(QUrl(logoUrl));

    QNetworkReply *logoReply = m_networkManager->get(r);

    connect(logoReply, SIGNAL(finished()),
            this, SLOT(onPodcastChannelLogoCompleted()));

    return logoReply;
}

void PodcastManager::insertChannelForNetworkReply(QNetworkReply *reply, PodcastChannel *channel)
{
    if (reply == nullptr) {
        qWarning() << "Network reply is NULL. Inserting nothing.";
    }

    if (channel == nullptr) {
        qWarning() << "Channel is NULL. Inserting nothing.";
    }

    m_channelNetworkRequestCache.insert(reply, channel);
}

void PodcastManager::insertChannelForFutureWatcher(QFutureWatcher<QList<PodcastEpisode*>*> *watcher, PodcastChannel *channel)
{
    if (watcher == nullptr) {
        qWarning() << "FutureWatcher is NULL. Inserting nothing.";
    }

    if (channel == nullptr) {
        qWarning() << "Channel is NULL. Inserting nothing.";
    }

    m_channelFutureWatcherCache.insert(watcher, channel);
}

PodcastChannel * PodcastManager::channelForNetworkReply(QNetworkReply *reply)
{
    PodcastChannel *channel = nullptr;
    if (reply == nullptr) {
        return channel;
    }

    if (m_channelNetworkRequestCache.isEmpty()) {
        qWarning() << "Podcast channel network cache is empty. Returning NULL channel!";
        return channel;
    }

    if (!m_channelNetworkRequestCache.contains(reply)) {
        qWarning() << "Podcast channel network cache does not contain network reply. Returning NULL channel!";
        return channel;
    }

    channel = m_channelNetworkRequestCache.value(reply);
    m_channelNetworkRequestCache.remove(reply);

    return channel;
}

PodcastChannel *PodcastManager::channelForFutureWatcher(QFutureWatcher<QList<PodcastEpisode*>*> *watcher)
{
    PodcastChannel *channel = nullptr;
    if (watcher == nullptr) {
        return channel;
    }

    if (m_channelFutureWatcherCache.isEmpty()) {
        qWarning() << "Podcast channel future cache is empty. Returning NULL channel!";
        return channel;
    }

    if (!m_channelFutureWatcherCache.contains(watcher)) {
        qWarning() << "Podcast channel future cache does not contain network reply. Returning NULL channel!";
        return channel;
    }

    channel = m_channelFutureWatcherCache.value(watcher);
    m_channelFutureWatcherCache.remove(watcher);

    return channel;
}

bool PodcastManager::isConnectedToWiFi()
{
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> confs = mgr.allConfigurations(QNetworkConfiguration::Active);
    foreach(const QNetworkConfiguration &netconf, confs) {
        if (netconf.bearerType() == QNetworkConfiguration::BearerWLAN) {
            qDebug() << "We are connected to a WiFi network.";
            return true;
        }
    }

    qDebug() << "We are NOT connected to a WiFI network.";
    return false;
}

void PodcastManager::removePodcastChannel(PodcastChannel& channel)
{
    /**
     * Delete episode data.
     */
    // Fetch model from model factory - then delete it from the factory's cache.
    PodcastEpisodesModel *episodesModel = m_episodeModelFactory->episodesModel(channel);
    QList<PodcastEpisode *> episodes = episodesModel->episodes();

    // Se if any episodes are being downloaded from this channel. Remove them from queue.
    foreach(PodcastEpisode* episode, episodes) {
        if (m_episodeDownloadQueue.contains(episode)) {
            onPodcastEpisodeDownloadFailed(episode);
            m_episodeDownloadQueue.removeOne(episode);  // removeOne: There ought to be always just one. Faster.
        }
    }

    // This will also call episodes->deleteDownload(); for all episodes in the model.
    m_episodeModelFactory->removeFromCache(channel);
    episodesModel->removeAll();
    delete episodesModel;

    /**
     * Delete channel data.
     * Do not touch the episode anymore!
     */
    // Deleting locally cached channel logo.

    QUrl channelLogoUrl(channel.logo());
    QFile channelLogo(channelLogoUrl.toLocalFile());
    if (!channelLogo.remove()) {
        QFileInfo fi(channelLogo);
        qWarning() << "Could not remove cached logo for channel:" << channel.title() << fi.absoluteFilePath();
    }


    // Finally remove the channel from the model and the cache.
    m_channelsModel->removeChannel(&channel);

    m_channelsCache.remove(channel.channelDbId());

    /*
    // Finally delete the memory reserved for the channel
    delete channel;
    */

}

void PodcastManager::deleteAllDownloadedPodcasts(PodcastChannel& channel)
{
    PodcastEpisodesModel *episodesModel = m_episodeModelFactory->episodesModel(channel);
    QList<PodcastEpisode *> episodes = episodesModel->episodes();

    foreach(PodcastEpisode *episode, episodes) {
        if (!episode->playFilename().isEmpty()) {
            episode->deleteDownload();
            episodesModel->refreshEpisode(episode);
        }
    }
}

bool PodcastManager::isDownloading()
{
    //return m_isDownloading;
    return m_currentEpisodeDownloads.length() > 0;
}



void PodcastManager::onAutodownloadOnChanged()
{
    qDebug() << "Setting changed: autodl: " << QVariant(m_autoDlConf->value()).toBool();

    m_autodownloadOnSettings = QVariant(m_autoDlConf->value()).toBool();

    foreach (PodcastChannel *channel, m_channelsModel->channels()) {
        channel->setAutoDownloadOn(m_autodownloadOnSettings);
    }

    m_channelsModel->setAutoDownloadToDB(m_autodownloadOnSettings);
}

void PodcastManager::onAutodownloadNumChanged()
{
    qDebug() << "Setting changed: autodownload number of episodes: " << QVariant(m_autoDlNumConf->value()).toInt();

    m_autodownloadNumSettings = QVariant(m_autoDlNumConf->value()).toInt();
}

void PodcastManager::onAutodelDaysChanged()
{
    qDebug() << "Setting changed: autodelete after days: " << QVariant(m_keepNumEpisodesConf->value()).toInt();

    m_keepNumEpisodesSettings = QVariant(m_keepNumEpisodesConf->value()).toInt();
}

void PodcastManager::onAutodelUnplayedChanged()
{
    qDebug() << "Setting changed: autodelete unplayed episodes: " << QVariant(m_autoDelUnplayedConf->value()).toBool();

    m_autoDelUnplayedSettings = QVariant(m_autoDelUnplayedConf->value()).toBool();
}

void PodcastManager::cleanupEpisodes()
{
    if (m_keepNumEpisodesSettings == 0) {
        // Keep all episodes.
        return;
    }

    m_cleanupChannels = m_channelsModel->channels();
    if (m_cleanupChannels.isEmpty()) {
        return;
    }

    PodcastEpisodesModel *episodesModel = m_episodeModelFactory->episodesModel(*m_cleanupChannels.takeLast());

    QFuture<void> future = QtConcurrent::run(episodesModel,
                                             &PodcastEpisodesModel::cleanOldEpisodes,
                                             m_keepNumEpisodesSettings,
                                             m_autoDelUnplayedSettings);

    m_futureWatcher.setFuture(future);
    connect(&m_futureWatcher, SIGNAL(finished()),
            this, SLOT(onCleanupEpisodeModelFinished()));
}

void PodcastManager::onCleanupEpisodeModelFinished()
{
    if (m_cleanupChannels.isEmpty()) {
        // All channels cleaned up.
        return;
    }

    PodcastEpisodesModel *episodesModel = m_episodeModelFactory->episodesModel(*(m_cleanupChannels.takeLast()));

    QFuture<void> future = QtConcurrent::run(episodesModel,
                                             &PodcastEpisodesModel::cleanOldEpisodes,
                                             m_keepNumEpisodesSettings,
                                             m_autoDelUnplayedSettings);

    m_futureWatcher.setFuture(future);
}

void PodcastManager::updateAutoDLSettingsFromCache()
{
    QSettings settings("harbour-podcatcher", "Podcatcher");

    if (!settings.contains("autoDlOn")) {
        settings.setValue("autoDlOn", m_autodownloadOnSettings);
        onAutodownloadOnChanged();
    } else {
        bool lastKnownAutoDlOn = settings.value("autoDlOn").toBool();

        if (lastKnownAutoDlOn != m_autodownloadOnSettings) {
            onAutodownloadOnChanged();
            settings.setValue("autoDlOn", m_autodownloadOnSettings);
        }
    }
}

void PodcastManager::compareEpisodes(QList<PodcastEpisode *> *list1, QList<PodcastEpisode *> *list2)
{
    if(list1->length() != list2->length()){
        qWarning() << "Episode list have not the same length!";
    }

    for (int i=1; i<std::min(list1->length(), list2->length()); i++){
        PodcastEpisode* e1 = list1->at(i);
        PodcastEpisode* e2 = list2->at(i);

        qDebug() << "\nComparing Episode "<< e1->title() <<"\n";

        if(e1->title() != e2->title()){
            qWarning() << "Epsisodes have diffrent titles: "<< e1->title()<< "and"  << e2->title();
        }

        if(e1->downloadLink() != e2->downloadLink()){
            qWarning() << "Epsisodes have diffrent links: "<< e1->downloadLink()<< e2->downloadLink();
        }

        if(e1->pubTime() != e2->pubTime()){
            qWarning() << "Epsisodes have diffrent pubDates: "<< e1->pubTime() << "and" << e2->pubTime();
        }

        if(e1->description() != e2->description()){
            qWarning() << "Epsisodes have diffrent descriptions: "<< e1->description() << "\nand\n"  << e2->description();
        }

    }

}

void PodcastManager::fetchSubscriptionsFromGPodder(const QString& gpodderUsername, const QString& gpodderPassword) {

    if (gpodderUsername.isEmpty() ||
            gpodderPassword.isEmpty()) {
        emit showInfoBanner(tr("gPodder.net authentication information required."));
        return;
    }

    m_gpodderUsername = gpodderUsername;
    m_gpodderPassword = gpodderPassword;

    m_gpodderQNAM = new QNetworkAccessManager();

    connect(m_gpodderQNAM, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)),
            this, SLOT(onGPodderAuthRequired(QNetworkReply *, QAuthenticator *)));

    QString gpodderUrl = QString("http://gpodder.net/subscriptions/%1.xml").arg(m_gpodderUsername);

    qDebug() << "Sending request to gPodder.net: " << gpodderUrl;

    QNetworkReply *reply = m_gpodderQNAM->get(QNetworkRequest(QUrl(gpodderUrl)));
    connect(reply, SIGNAL(finished()),
            this, SLOT(onGPodderRequestFinished()));
}

void PodcastManager::onGPodderAuthRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    Q_UNUSED(reply);

    if (m_gpodderUsername.isEmpty() ||
            m_gpodderPassword.isEmpty()) {

        qDebug() << "Could not authenticate user with gPodder.net.";

        // This means that we are here a second time and the credentials the user gave are not correct.
        emit showInfoBanner(tr("gPodder.net credentials not accepted. Try again."));

        // Clean up the resources. This ends here...
        QNetworkAccessManager *qnam = reply->manager();
        disconnect(reply, SIGNAL(finished()), this,
                   SLOT(onGPodderRequestFinished()));
        disconnect(qnam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this,
                   SLOT(onGPodderAuthRequired(QNetworkReply *, QAuthenticator *)));

        qnam->deleteLater();
        reply->close();
        reply->deleteLater();

        return;
    }

    qDebug() << "Sending HTTP AUTH to gPodder.net";

    auth->setUser(m_gpodderUsername);
    auth->setPassword(m_gpodderPassword);

    m_gpodderUsername = "";
    m_gpodderPassword = "";
}

void PodcastManager::onDestroyingNetworkReply(QObject * /*obj*/)
{
    //QNetworkReply *reply = qobject_cast<QNetworkReply *>(obj);
    qDebug() << "Destroying NetworkReply ";// << reply->url();
}

void PodcastManager::onSessionConnected()
{
    qDebug() << "Network Session connected";
}

void PodcastManager::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility na)
{
    qDebug() << "Network Accessibility changed to" << na;
}

void PodcastManager::onRequestFinished(QNetworkReply *reply)
{
    qDebug() << "request finished:" << reply->url();
}



void PodcastManager::onGPodderRequestFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    QNetworkAccessManager *gpodderQNAM = reply->manager();

    disconnect(reply, SIGNAL(finished()), this,
               SLOT(onGPodderRequestFinished()));
    disconnect(gpodderQNAM, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this,
               SLOT(onGPodderAuthRequired(QNetworkReply *, QAuthenticator *)));
    QByteArray xml = reply->readAll();

    qDebug() << "Response from gpodder: " << xml;

    reply->deleteLater();
    gpodderQNAM->deleteLater();

    QList<QString> subscriptionUrls = PodcastRSSParser::parseGPodderSubscription(xml);
    if (subscriptionUrls.empty()) {
        emit showInfoBanner(tr("No subscriptions found from gPodder.net"));
        return;
    }

    emit showInfoBanner(tr("Getting subscriptions from gPodder.net..."));

    foreach(const QString& url, subscriptionUrls) {
        requestPodcastChannelFromGPodder(QUrl(url));
    }

}

void PodcastManager::requestPodcastChannelFromGPodder(const QUrl &rssUrl)
{
    qDebug() << "Requesting Podcast channel" << rssUrl;

    if (!rssUrl.isValid()) {
        qWarning() << "Provided podcast channel URL is not valid.";
        return;
    }

    auto *channel = new PodcastChannel(this);
    channel->setUrl(rssUrl.toString());

    if (m_channelsModel->channelAlreadyExists(channel)) {
        qDebug() << "Channel is already in DB. Not doing anything.";
        delete channel;
        return;
    }

    if (channelRequestMap.contains(rssUrl.toString())) {
        qDebug() << "Duplicate URL from gPodder.net. Not requesting the same podcast again.";
        return;
    }

    channelRequestMap.insert(rssUrl.toString(), channel);

    QNetworkRequest request;
    request.setUrl(rssUrl);

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, SIGNAL(finished()),
            this, SLOT(onPodcastChannelCompleted()));

    // m_networkManager->get(request);
}



