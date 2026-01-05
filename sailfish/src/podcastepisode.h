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
#ifndef PODCASTEPISODE_H
#define PODCASTEPISODE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QImage>

#include <MGConfItem>

class PodcastEpisode;

#include "podcastchannel.h"

struct PodcastChapter{
    QString name;
    QString description;
    int start{};
    QString url;
    QImage picture;
};

class PodcastChannel;


class PodcastEpisode : public QObject
{
    Q_OBJECT
public:
    enum EpisodeStates {
        GetState = 0,
        QueuedState,
        DownloadingState,
        DownloadedState,
        CanceledState,
        PlayedState,
        FinishedState,
        NewState
    };

    explicit PodcastEpisode(QObject *parent = nullptr);

    ~PodcastEpisode();

    void downloadEpisode();

    void setDbId(int id);
    void setChannel(PodcastChannel* channel);
    void setTitle(const QString &title);
    void setDownloadLink(const QString &downloadLink);
    void setPlayFilename(const QString &playFilename);
    void setDescription(const QString &desc);
    void setPubTime(const QDateTime &pubDate);
    void setDuration(const QString &duration);
    void setDownloadSize(qint64 downloadSize);
    void setState(EpisodeStates newState);
    void setDownloadManager(QNetworkAccessManager *qnam);
    void setLastPlayed(const QDateTime &lastPlayed);
    void setHasBeenCanceled(bool canceled);
    void setFinished(bool finished);
    void setPlayPosition(int playPosition);
    void setNew(bool episodeNew);

    void setCredentails(const QString& user, const  QString& password);

    int dbid() const;
    int channelid() const;

    PodcastChannel* channel();

    QString title() const;
    QString downloadLink() const;
    QString playFilename() const;
    QString description() const;
    QDateTime pubTime() const;
    QString duration() const;
    qint64 downloadSize() const;
    qint64 alreadyDownloaded();
    QString episodeState() const;
    bool finished() const;
    QDateTime lastPlayed() const;
    bool hasBeenCanceled() const;
    void getAudioUrl();
    QString errorMessage() const;
    int playPosition() const;
    bool episodeNew() const;


    void cancelCurrentDownload();
    void deleteDownload();
    void setAsPlayed();
    void setAsUnplayed();
    void setAsFinished();
    void setAsUnFinished();

signals:
    void episodeChanged();
    void podcastEpisodeDownloaded(PodcastEpisode *episode);
    void podcastEpisodeDownloadFailed(PodcastEpisode *episode);
    void downloadedBytesUpdated(int bytes);
    void streamingUrlResolved(QString url, QString title);
    void playPositionChanged(int playPosition);

public slots:

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadReadyRead();
    void onPodcastEpisodeDownloadCompleted();
    void onAudioUrlMetadataChanged();
    void onDownloadError(QNetworkReply::NetworkError err);
    void onMetaDataChanged();

private:
    bool isValidAudiofile(QNetworkReply *reply) const;
    QString getDownloadDir();
    QString getSDPath();

    //bool isOnlyWebsiteUrl() const;

    int m_dbid{};
    PodcastChannel* m_channel{};
    QString m_title;
    QString m_downloadLink;
    QString m_playFilename;
    QString m_description;
    QDateTime m_pubDateTime;
    QString m_duration;
    EpisodeStates m_state;
    qint64 m_bytesDownloaded;
    qint64 m_downloadSize{};
    QDateTime m_lastPlayed;
    bool m_hasBeenCanceled;
    bool m_finished;
    int  m_playPosition;
    bool m_new;

    QString m_user;
    QString m_password;

    QNetworkAccessManager *m_dlNetworkManager {nullptr};
    QNetworkReply *m_currentDownload;

    QNetworkAccessManager *m_streamResolverManager{};
    int m_streamResolverTries{};

    QFile* m_downloadFile;
    QString m_errorMessage;

   MGConfItem *m_saveOnSDCOnf;
};

#endif // PODCASTEPISODE_H
