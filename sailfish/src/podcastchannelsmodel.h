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
#ifndef PODCASTCHANNELSMODEL_H
#define PODCASTCHANNELSMODEL_H

#include <QAbstractListModel>

#include "podcastchannel.h"
#include "podcastsqlmanager.h"

class PodcastManager;
class PodcastChannelsModel : public QAbstractListModel
{
    Q_OBJECT

    enum ChannelRoles {
        ChannelIdRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        LogoRole,
        IsRefreshingRole,
        IsDownloadingRole,
        UnplayedEpisodesRole,
        AutoDownloadOnRole,
        SortByRole,
        SortDescendingRole,
        HasNewRole,
    };

public:
    ~PodcastChannelsModel();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool addChannel(PodcastChannel *channel);
    bool addChannelToModel(PodcastChannel *channel);
    bool removeChannel(PodcastChannel *channel);
    bool removeChannelFromModel(PodcastChannel *channel);
    QList<PodcastChannel *> channels();

    PodcastChannel * podcastChannelById(int id);
    bool channelAlreadyExists(PodcastChannel *channel);
    void refreshChannel(int id);
    void setAutoDownloadToDB(bool autoDownload);
    void updateChannel(PodcastChannel *channel);
    QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private slots:
    void onChannelChanged();
    void onSortByChanged(const QString &sortBy);
    void onSortDescendingChanged(bool sortDescending);
    void onUrlChanged(QString url);

private:
    explicit PodcastChannelsModel(QObject *parent = nullptr, bool archived=false);  // Do not allow instantiation of this class...
    QList<PodcastChannel *> m_channels;
    PodcastSQLManager *m_sqlmanager;
    QHash<int, QByteArray> m_roles;

    friend class PodcastManager;                         // ..except for PodcastManager;
};

#endif // PODCASTCHANNELSMODEL_H
