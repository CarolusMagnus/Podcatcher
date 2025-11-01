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
#include <QtDebug>

#include "podcastchannelsmodel.h"
#include "dbhelper.h"

PodcastChannelsModel::PodcastChannelsModel(QObject *parent, bool archived) :
    QAbstractListModel(parent)
{
    m_roles[ChannelIdRole] = "channelId";
    m_roles[TitleRole] = "title";
    m_roles[DescriptionRole] = "description";
    m_roles[LogoRole] = "logo";
    m_roles[IsRefreshingRole] = "isRefreshing";
    m_roles[IsDownloadingRole] = "isDownloading";
    m_roles[UnplayedEpisodesRole] = "unplayedEpisodes";
    m_roles[AutoDownloadOnRole] = "autoDownloadOn";
    m_roles[SortByRole] = "sortBy";
    m_roles[SortDescendingRole] = "sortDescending";
    m_roles[HasNewRole]="hasNew";

    DBHelper dbhelper;
    dbhelper.createAutoDownloadFieldChannels();

    m_sqlmanager = PodcastSQLManagerFactory::sqlmanager();

    m_sqlmanager->migrateChannelLogos();

    foreach(PodcastChannel *channel, m_sqlmanager->channelsInDB(archived)) {
        connect(channel, SIGNAL(channelChanged()),
                this, SLOT(onChannelChanged()));

        connect(channel, SIGNAL(sortByChanged(QString)),
                this, SLOT(onSortByChanged(QString)));

        connect(channel, SIGNAL(sortDescendingChanged(bool)),
                this, SLOT(onSortDescendingChanged(bool)));

        connect(channel, SIGNAL(urlChanged(QString)),
                this, SLOT(onUrlChanged(QString)));

        m_channels << channel;
    }
}

PodcastChannelsModel::~PodcastChannelsModel() {
    foreach(PodcastChannel *channel, m_channels) {
        delete channel;
    }
}

QVariant PodcastChannelsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_channels.count())
        return QVariant();

    PodcastChannel *channel = m_channels.at(index.row());

    switch(role) {
    case ChannelIdRole:
        return channel->channelDbId();   // FIXME: Do not expose this.

    case TitleRole:
        return channel->title();

    case DescriptionRole:
        return channel->description();

    case LogoRole:
        return channel->logo();

    case IsRefreshingRole:
        return channel->isRefreshing();

    case IsDownloadingRole:
        return channel->isDownloading();

    case UnplayedEpisodesRole:
        return channel->unplayedEpisodes();

    case AutoDownloadOnRole:
        return channel->isAutoDownloadOn();

    case SortByRole:
        return channel->sortBy();

    case SortDescendingRole:
        return channel->sortDescending();

    case HasNewRole:
        return channel->hasNew();
    }

    return QVariant();
}

int PodcastChannelsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_channels.count();
}

bool channelsLessThan(const PodcastChannel *c1, const PodcastChannel *c2)
{
    return *c1 < *c2;
}


bool PodcastChannelsModel::addChannel(PodcastChannel *channel)
{
     if (m_sqlmanager->podcastChannelToDB(channel) > 0) {
         m_channels.append(channel);

         qSort(m_channels.begin(),
               m_channels.end(),
               channelsLessThan);

         int index = m_channels.indexOf(channel);
         beginInsertRows(QModelIndex(), index, index);
         endInsertRows();

         connect(channel, SIGNAL(channelChanged()),
                 this, SLOT(onChannelChanged()));

         connect(channel, SIGNAL(sortByChanged(QString)),
                 this, SLOT(onSortByChanged(QString)));

         connect(channel, SIGNAL(sortDescendingChanged(bool)),
                 this, SLOT(onSortDescendingChanged(bool)));

         connect(channel, SIGNAL(urlChanged(QString)),
                 this, SLOT(onUrlChanged(QString)));

         return true;
     } else {
         return false;
     }
}

bool PodcastChannelsModel::addChannelToModel(PodcastChannel *channel)
{
         m_channels.append(channel);

         qSort(m_channels.begin(),
               m_channels.end(),
               channelsLessThan);

         int index = m_channels.indexOf(channel);
         beginInsertRows(QModelIndex(), index, index);
         endInsertRows();

         connect(channel, SIGNAL(channelChanged()),
                 this, SLOT(onChannelChanged()));

         connect(channel, SIGNAL(sortByChanged(QString)),
                 this, SLOT(onSortByChanged(QString)));

         connect(channel, SIGNAL(sortDescendingChanged(bool)),
                 this, SLOT(onSortDescendingChanged(bool)));

         connect(channel, SIGNAL(urlChanged(QString)),
                 this, SLOT(onUrlChanged(QString)));

         return true;
}


bool PodcastChannelsModel::removeChannel(PodcastChannel *channel)
{
    if (channel == nullptr) {
        qWarning() << "Channel is NULL.";
        return false;
    }

    int dbid = channel->channelDbId();
    // Remove from DB
    m_sqlmanager->removeChannelFromDB(dbid);

    // Remove from model
    for (int i=0; i<m_channels.size(); i++) {
        PodcastChannel *channel = m_channels.at(i);
        if (channel->channelDbId() == dbid) {
            beginRemoveRows(QModelIndex(), i, i);
            m_channels.removeAt(i);
            endRemoveRows();

            return true;
        }
    }

    return false;
}

 bool PodcastChannelsModel::removeChannelFromModel(PodcastChannel *channel){
     if (channel == nullptr) {
         qWarning() << "Channel is NULL.";
         return false;
     }

     int dbid = channel->channelDbId();

     // Remove from model
     for (int i=0; i<m_channels.size(); i++) {
         PodcastChannel *channel = m_channels.at(i);
         if (channel->channelDbId() == dbid) {
             beginRemoveRows(QModelIndex(), i, i);
             m_channels.removeAt(i);
             endRemoveRows();

             return true;
         }
     }

     return false;
 }

QList<PodcastChannel *> PodcastChannelsModel::channels()
{
    return m_channels;
}

bool PodcastChannelsModel::channelAlreadyExists(PodcastChannel *channel)
{
    return m_sqlmanager->isChannelInDB(channel);
}

PodcastChannel * PodcastChannelsModel::podcastChannelById(int id)
{
    foreach(PodcastChannel *channel, m_channels) {
        if (channel->channelDbId() == id) {
            return channel;
        }
    }

    return nullptr;
}

void PodcastChannelsModel::refreshChannel(int id)
{

    PodcastChannel *channel = podcastChannelById(id);
    if (channel == nullptr) {
        qWarning() << "Could not refresh PodcastChannel. Got NULL!";
        return;
    }

    m_sqlmanager->channelInDB(channel->channelDbId(), channel);
}


void PodcastChannelsModel::onChannelChanged()
{
    qDebug() << "Podcast channel changed. Refreshing from SQL to UI...";
    auto *channel  = qobject_cast<PodcastChannel *>(sender());
    if (channel == nullptr) {
        return;
    }

    int channelIndex = m_channels.indexOf(channel);
    if (channelIndex != -1) {
        QModelIndex modelIndex = createIndex(channelIndex, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void PodcastChannelsModel::onSortByChanged(const QString& /*sortBy*/)
{
    auto *channel  = qobject_cast<PodcastChannel *>(sender());
    if (channel == nullptr) {
        return;
    }

    m_sqlmanager->updateChannelInDB(channel);
}

void PodcastChannelsModel::onSortDescendingChanged(bool /*descending*/)
{
    auto *channel  = qobject_cast<PodcastChannel *>(sender());
    if (channel == nullptr) {
        return;
    }

    m_sqlmanager->updateChannelInDB(channel);
}

void PodcastChannelsModel::onUrlChanged(QString url)
{
    auto *channel  = qobject_cast<PodcastChannel *>(sender());
    if (channel == nullptr) {
        return;
    }

    qDebug() << "saving new URL to DB:" << url;
    m_sqlmanager->updateChannelInDB(channel);
}


void PodcastChannelsModel::setAutoDownloadToDB(bool autoDownload)
{
    m_sqlmanager->updateChannelAutoDownloadToDB(autoDownload);
}

void PodcastChannelsModel::updateChannel(PodcastChannel *channel)
{
    m_sqlmanager->updateChannelInDB(channel);
}

QHash<int, QByteArray> PodcastChannelsModel::roleNames() const
{
    return m_roles;
}



