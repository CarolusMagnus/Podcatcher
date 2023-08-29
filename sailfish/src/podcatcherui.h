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
#ifndef PODCATCHERUI_H
#define PODCATCHERUI_H

#include <QObject>
#include <QList>
#include <sailfishapp.h>

#include <MGConfItem>

#include "podcastmanager.h"
#include "podcastchannelsmodel.h"
#include "mediametadataextractor.h"
//#include "podcastplaylist.h"

struct EpisodeIndex {int channelID;
                     int index;};

class PodcatcherUI : QObject
{
    Q_OBJECT
public:
    explicit PodcatcherUI();

    Q_INVOKABLE void addPodcast(const QString &rssUrl, const QString &logoUrl = QString());
    Q_INVOKABLE void deletePodcasts(int channelId);
    //Q_INVOKABLE bool isDownloading();
    Q_INVOKABLE bool isLiteVersion();
    Q_INVOKABLE void refreshChannels();
    Q_INVOKABLE QString versionString();
    Q_INVOKABLE void importFromGPodder(const QString &username, const QString &password);
    Q_INVOKABLE void changeFeedURLIfValid(int channelId, const QString& url);
    Q_INVOKABLE bool isConnectedToWiFi();

    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY isDownloadingChanged)

    void downloadNewEpisodes(QString channelId);

signals:
    void showInfoBanner(QString text);
    void downloadedBytesUpdated(int bytes);
    void downloadingPodcasts(bool downloading);
    void streamingUrlResolved(QString streamUrl, QString streamTitle);
    void playFileWithInternalPlayer(QString fileName);

    void isDownloadingChanged(bool isDownloading);

public slots:

private slots:
    void onShowChannel(const QString &channelId);
    void onRefreshEpisodes(int channelId);
    void onDownloadPodcast(int channelId, int index);
    void onPlayPodcast(int channelId, int index);
    void onAddToPlayList(int channelId, int index);
    void onDownloadingPodcast(bool isDownloading);
    void onCancelDownload(int channelId, int index);
    void onCancelQueueing(int channelId, int index);
    void onDeleteChannel(const QString &channelId);
    void onAllListened(const QString &channelId);
    void onAllFinished(const QString &channelId);
    void onDeletePodcast(int channelId, int index);
    void onMarkAsUnplayed(int channelId, int index);
    void onMarkAsFinished(int channelId, int index);
    void onMarkAsUnFinished(int channelId, int index);
    void onStartStreaming(int channelId, int index);
    void onStreamingUrlResolved(const QString &streamUrl, const QString &streamTitle);
    void onAutoDownloadChanged(int channelId, bool autoDownload);
    void onMediaPlayerChanged();
    void onEndOfMedium(const QString& name);


private:
    PodcastManager m_pManager;
    MediaMetaDataExtractor m_mMDE;
    PodcastChannelsModel *m_channelsModel;
    PodcastChapterModel *m_chapterModel;
    PodcastEpisodesModelFactory *modelFactory;
    //PodcastPlaylist* m_playlist;

    QMap<QString, QString> logoCache;
    //QMap<QString, EpisodeIndex> indexCache;
    QQuickView* view;

    PodcastEpisode* m_playingEpisode;

    MGConfItem *m_mediaPlayerConf;
    MGConfItem *m_autoSyncConf;
    MGConfItem *m_autoPlayNext;
    QString m_mediaPlayerPath;

    bool isDownloading();
};

#endif // PODCATCHERUI_H
