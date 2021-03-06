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
#include <QObject>

#include "podcastsqlmanager.h"
#include "podcastepisodesmodelfactory.h"

PodcastEpisodesModelFactory * PodcastEpisodesModelFactory::instance = nullptr;

PodcastEpisodesModelFactory::PodcastEpisodesModelFactory()
{
    m_sqlmanager = PodcastSQLManagerFactory::sqlmanager();
}

PodcastEpisodesModel * PodcastEpisodesModelFactory::episodesModel(PodcastChannel& channel)
{

    // If the model is already fetched from the DB, just return it.
    // Otherwise fetch data from DB an create the model.
    if (m_modelCache.contains(channel.channelDbId())) {
        return m_modelCache.value(channel.channelDbId());
    }

    auto *model = new PodcastEpisodesModel(channel);
    QList<PodcastEpisode *> episodes = m_sqlmanager->episodesInDB(&channel);
    model->addEpisodes(episodes);

    // Cache the constructed model
    m_modelCache.insert(channel.channelDbId(), model);

    return model;
}

PodcastEpisodesModelFactory * PodcastEpisodesModelFactory::episodesFactory()
{
    if (instance == nullptr) {
        instance = new PodcastEpisodesModelFactory;
    }
    return instance;
}

void PodcastEpisodesModelFactory::removeFromCache(PodcastChannel &channel)
{
    if (m_modelCache.contains(channel.channelDbId())) {
        m_modelCache.remove(channel.channelDbId());
    }
}
