/**
 * This file is part of Podcatcher for Sailfish OS.
 * Author: Johan Paul (johan.paul@gmail.com)
 *         Moritz Carmesin
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
#include <QHash>
#include <QVariant>
#include <QDateTime>
#include <utility>


#include <QtDebug>

#include "podcastmanager.h"
#include "podcastepisodesmodel.h"

PodcastEpisodesModel::PodcastEpisodesModel(PodcastChannel &channel, QObject *parent) :
    QAbstractListModel(parent),
    m_channel(channel)
{
    m_roles[DbidRole] = "dbid";
    m_roles[TitleRole] = "title";
    m_roles[PubRole] = "published";
    m_roles[DescriptionRole] = "description";
    m_roles[StateRole] = "episodeState";
    m_roles[TotalDownloadRole] = "totalDownloadSize";
    m_roles[AlreadyDownloaded] = "alreadyDownloadedSize";
    m_roles[LastTimePlayedRole] = "lastTimePlayed";
    m_roles[PublishedTimestamp] = "timestamp";
    m_roles[FinishedRole] = "finished";
    m_roles[NewRole] = "episodeNew";
    //setRoleNames(roles);

    m_sqlmanager = PodcastSQLManagerFactory::sqlmanager();

    connect(&m_channel, SIGNAL(sortByChanged(QString)),
            this, SLOT(onSortByChanged(QString)));

    connect(&m_channel,  SIGNAL(sortDescendingChanged(bool)),
            this, SLOT(onSortDescendingChanged(bool)));

}

PodcastChannel& PodcastEpisodesModel::channel()
{
    return m_channel;
}

PodcastEpisodesModel::~PodcastEpisodesModel() = default;

int PodcastEpisodesModel::rowCount(const QModelIndex &) const
{
    return m_episodes.size();
}

QVariant PodcastEpisodesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_episodes.count())
        return QVariant();

    PodcastEpisode *episode = m_episodes.at(index.row());

    switch(role) {
    case TitleRole:
        return episode->title();

    case PubRole:
        return episode->pubTime().toString(tr("dd.MM.yyyy"));

    case PublishedTimestamp:
        return episode->pubTime().toTime_t();

    case DbidRole:
        return episode->dbid();

    case DescriptionRole:
        return episode->description();

    case StateRole:
        return episode->episodeState();

    case TotalDownloadRole:
        return episode->downloadSize();

    case AlreadyDownloaded:
        return episode->alreadyDownloaded();

    case LastTimePlayedRole:
        //if (episode->episodeState() == "played") {
        if (episode->lastPlayed().isValid()) {
            return QString(tr("Last played: %1")).arg(episode->lastPlayed().toString(tr("dd.MM.yyyy hh:mm")));
        } else  {
            return QString();
        }
    case NewRole:
        return episode->episodeNew();
    case FinishedRole:
        return episode->finished();
    default:
        return QVariant();
    }

}

void PodcastEpisodesModel::addEpisode(PodcastEpisode *episode)
{
    QList<PodcastEpisode *> episodes;
    episodes << episode;
    addEpisodes(episodes);

    sortEpisodes();
}

void PodcastEpisodesModel::addEpisodes(const QList<PodcastEpisode *>& episodes)
{
    if (episodes.isEmpty()) {
        return;
    }

    QList<PodcastEpisode *>sortedEpisodes = m_episodes;

    if (!(m_channel.sortBy() == "published" && m_channel.sortDescending())){
        sortEpisodes(sortedEpisodes, "published", true);
    }

    QDateTime modelsLatestEpisode;
    if (m_episodes.isEmpty()) {
        modelsLatestEpisode = QDateTime();
    } else {
        modelsLatestEpisode = sortedEpisodes.first()->pubTime();
    }


    QDateTime dbsLatestEpisode = m_sqlmanager->latestEpisodeTimestampInDB(m_channel.channelDbId());
    PodcastEpisode *episode;
    QList<PodcastEpisode *> newEpisodesToAdd;

    // Add the episode to the UI model if its timestamp is > modelsLatestEpisode. By default the modelsLatestEpisode
    // is the result of the most recent model population - with or without episodes added to the DB.
    // If the episode also has a timestmap > dbsLatestEpisode, then also add it to the DB.
    // Last query the latest timestamp of all channel's episodes after the operation.
    for(int i=episodes.size()-1; i>=0; i--) {
        episode = episodes.at(i);                           // Take the last episode in the new model.
        if (episode->pubTime() > modelsLatestEpisode) {     // If this episodes has a more recent timestamp, add to model.            qDebug() << "Adding to UI...";
            sortedEpisodes.prepend(episode);
            //beginInsertRows(QModelIndex(), 0, 0);
            //m_episodes.prepend(episode);                    // Since we took that last item from the new episodes model, we add this item first to the view.
            //endInsertRows();                                // When we do this for new episodes not yet in the model, all new episodes (episode->pubTime() > modelsLatestEpisode)
            // ends up on top of the list. Note that the QModelIndex is also updated accordingly.
            episode->setChannel(&m_channel);

            connect(episode, SIGNAL(episodeChanged()),
                    this, SLOT(onEpisodeChanged()));

            if (episode->pubTime() > dbsLatestEpisode) {
                newEpisodesToAdd << episode;
            }
        }else{
            episode->deleteLater(); // Free unused episodes
        }
    }

    if (!newEpisodesToAdd.isEmpty()) {
        qDebug() << "Adding new episodes to DB: " << newEpisodesToAdd.size();
        m_sqlmanager->podcastEpisodesToDB(newEpisodesToAdd,
                                          m_channel.channelDbId());
        m_latestEpisodeTimestamp = m_sqlmanager->latestEpisodeTimestampInDB(m_channel.channelDbId());
        m_channel.setHasNew(true);
    } else {
        qDebug() << "No new episodes to be added to the DB.";
    }

    beginResetModel();
    m_episodes = sortedEpisodes;
    sortEpisodes(m_episodes, m_channel.sortBy(), m_channel.sortDescending());
    endResetModel();
}

void PodcastEpisodesModel::delEpisode(PodcastEpisode *episode)
{
    qDebug() << "Deleting episode with name: " << episode->title() << ", pub date:" << episode->pubTime();
    for (int i=0; i<m_episodes.length(); i++) {
        if (episode == m_episodes.at(i)) {
            qDebug() << "Remove from model, index: " << i;
            beginRemoveRows(QModelIndex(), i, i);
            m_episodes.removeAt(i);
            endRemoveRows();

            m_sqlmanager->removePodcastFromDB(episode);
        }
    }

    delete episode;
}

void PodcastEpisodesModel::delEpisode(int index, PodcastEpisode *episode)
{
    qDebug() << "Deleting episode with name: " << episode->title() << ", pub date:" << episode->pubTime();

    beginRemoveRows(QModelIndex(), index, index);
    m_episodes.removeAt(index);
    endRemoveRows();

    m_sqlmanager->removePodcastFromDB(episode);
    delete episode;
}

PodcastEpisode * PodcastEpisodesModel::episode(int index)
{
    PodcastEpisode *episode = nullptr;
    if (index < 0 || index > m_episodes.count())
        return episode;

    episode = m_episodes.at(index);
    return episode;
}

void PodcastEpisodesModel::onEpisodeChanged()
{
    auto *episode  = qobject_cast<PodcastEpisode *>(sender());
    if (episode == nullptr) {
        return;
    }

    int episodeIndex = m_episodes.indexOf(episode);
    if (episodeIndex != -1) {
        QModelIndex modelIndex = createIndex(episodeIndex, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void PodcastEpisodesModel::onSortDescendingChanged(bool /*descending*/)
{
    sortEpisodes();
}

void PodcastEpisodesModel::sortEpisodes(QList<PodcastEpisode*>& episodes, const QString& sortBy, bool descending)
{
    if(episodes.isEmpty())
        return;

    QStringList states = {"undownloadable","get", "queued", "downloading","downloaded", "played"};

    auto cmp = [=]  (const PodcastEpisode* a, const PodcastEpisode* b){
        if (sortBy == "title"){
            return descending? (a->title() > b->title()) : (a->title() < b->title());
        }else if (sortBy == "dbid") {
            return descending? (a->dbid() > b->dbid()) : (a->dbid() < b->dbid());
        } else if (sortBy == "state"){
            int stateA = states.indexOf(a->episodeState());
            int stateB = states.indexOf(b->episodeState());
            if (stateA != stateB){
                return descending? (stateA > stateB) : (stateA < stateB);
            }
        }/*if (sortBy == "published")*/

        return descending? (a->pubTime() > b->pubTime()) : (a->pubTime() < b->pubTime());

    };

    std::sort(episodes.begin(), episodes.end(), cmp);
}

void PodcastEpisodesModel::sortEpisodes()
{
    bool descending = m_channel.sortDescending();
    QString sortBy = m_channel.sortBy();
    qDebug() << "Sording Episodes by" << sortBy << "descending " <<descending;

    emit layoutAboutToBeChanged();

    sortEpisodes(m_episodes, sortBy, descending);

    emit layoutChanged();
}



void PodcastEpisodesModel::onSortByChanged(const QString& /*sortBy*/)
{
    sortEpisodes();
}

QList<PodcastEpisode *> PodcastEpisodesModel::undownloadedEpisodes(int max)
{
    QList<PodcastEpisode *> episodes;

    if (m_episodes.isEmpty()) {
        return episodes;
    }

    QDateTime lastCancelledTime = QDateTime();

    for (int i=0; i<m_episodes.length(); i++) {
        PodcastEpisode *episode = m_episodes.at(i);

        if (episode->hasBeenCanceled() && ((lastCancelledTime.isValid() && episode->pubTime() > lastCancelledTime) ||
                                           !lastCancelledTime.isValid())){
            lastCancelledTime = episode->pubTime();
        }

        if (!episode->downloadLink().isEmpty() &&
                episode->playFilename().isEmpty() &&
                !episode->hasBeenCanceled()) {
            episodes << episode;
        }
    }

    for (int i=0; i< episodes.length(); i++){
        if (episodes.at(i)->pubTime() < lastCancelledTime){
            episodes.removeAt(i);
            i--;
        }
    }

    sortEpisodes(episodes,"published", true);

    while(episodes.length() > max){
        episodes.takeLast();
    }

    return episodes;
}

void PodcastEpisodesModel::refreshModel()
{
    //  PodcastEpisode *latestEpisode = m_episodes.at(0);
}


void PodcastEpisodesModel::refreshEpisode(PodcastEpisode *episode)
{
    qDebug() << "Saving episode to DB. Play filename:" << episode->playFilename();
    m_sqlmanager->updatePodcastInDB(episode);
}

void PodcastEpisodesModel::removeAll()
{
    qDebug()  << "Removing all episodes from the model.";
    foreach(PodcastEpisode *episode, m_episodes) {
        episode->deleteDownload();
        delete episode;
    }

    beginRemoveRows(QModelIndex(), 0, m_episodes.size()-1);
    m_episodes.clear();
    endRemoveRows();
}

QHash<int, QByteArray> PodcastEpisodesModel::roleNames() const
{
    return m_roles;
}

QList<PodcastEpisode *> PodcastEpisodesModel::unplayedEpisodes()
{
    QList<PodcastEpisode *> episodes;

    if (m_episodes.isEmpty()) {
        return episodes;
    }

    for (auto episode : std::as_const(m_episodes)) {
        if (!episode->playFilename().isEmpty() &&
                episode->episodeState() == "downloaded") {
            episodes << episode;
        }
    }

    return episodes;
}

QList<PodcastEpisode *> PodcastEpisodesModel::unfinishedEpisodes()
{
    QList<PodcastEpisode *> episodes;

    if (m_episodes.isEmpty()) {
        return episodes;
    }

    for (auto episode : std::as_const(m_episodes)) {
        if (!episode->finished()) {
            episodes << episode;
        }
    }

    return episodes;
}

QList<PodcastEpisode *> PodcastEpisodesModel::unfinishedDownloadedEpisodes()
{
    QList<PodcastEpisode *> episodes;

    if (m_episodes.isEmpty()) {
        return episodes;
    }

    for (auto episode : std::as_const(m_episodes)) {
        if (!episode->finished() && !episode->playFilename().isEmpty()) {
            episodes << episode;
        }
    }

    return episodes;
}

PodcastEpisode *PodcastEpisodesModel::getNextEpisodeForPlayer(int lastIndex)
{
    qDebug() << "Getting next episode after " << lastIndex;
    auto available = unfinishedDownloadedEpisodes();

    int idx_avail = (lastIndex >= 0)? available.indexOf(m_episodes.at(lastIndex)): -1;

    if  (available.length() > 0){
        if (idx_avail == -1)
            return available.first();

        if (idx_avail == 0 && available.length() > 1)
            return available.at(1);

        if (idx_avail > 0)
            return available.at(idx_avail -1);
        }

    return nullptr;

}


QList<PodcastEpisode *> PodcastEpisodesModel::episodes()
{
    return m_episodes;
}

void PodcastEpisodesModel::cleanOldEpisodes(int keepNumEpisodes, bool keepUnplayed)
{
    if (keepNumEpisodes == 0) {
        return;
    }

    QList<PodcastEpisode *> episodesToDel;
    for (int i=keepNumEpisodes; i<m_episodes.length(); i++) {
        PodcastEpisode *episode = m_episodes.at(i);

        // If we want to keep unplayed episodes...
        if (keepUnplayed) {
            // Check if the episode filename is not empty (we have an episode)
            // and that the last played is not set, then we know we have an unplayed and
            // downloaded episode that we want to keep.
            if (!episode->playFilename().isEmpty() &&
                    !episode->lastPlayed().isValid()) {
                continue;
            }
        }

        // Otherwise, delete the episode and the download
        if (!episode->playFilename().isEmpty()) {
            episode->deleteDownload();
        }

        episodesToDel << episode;
    }

    foreach(PodcastEpisode *episode, episodesToDel) {
        delEpisode(episode);
    }
}

