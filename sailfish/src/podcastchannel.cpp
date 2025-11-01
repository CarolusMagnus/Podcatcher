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

#include <utility>
#include "podcastchannel.h"

PodcastChannel::PodcastChannel(QObject *parent) :
    QObject(parent)
{
    m_isRefreshing = false;
    m_isDownloading = false;
    m_autoDownloadOn = false;
    m_unplayedEpisodes = 0;
    m_sortDescending = true;
    m_sortBy = PUBLISHED;
    m_id = -1;
    m_hasNew = false;
}

void PodcastChannel::setId(int id)
{
    m_id = id;
}

int PodcastChannel::channelDbId() const
{
    return m_id;
}

void PodcastChannel::setTitle(const QString &title)
{
    m_title = title;
}

void PodcastChannel::setLogoUrl(const QString &logo)
{
    m_logoUrl = logo;
}

QString PodcastChannel::title() const
{
    return m_title;
}

QString PodcastChannel::logoUrl() const
{
    return m_logoUrl;
}

QString PodcastChannel::url() const
{
    return m_url;
}

void PodcastChannel::setUrl(const QString &url)
{
    m_url = url;
}

void PodcastChannel::setLogo(const QString &logo)
{
    m_logo = logo;
}

QString PodcastChannel::logo() const
{
    return m_logo;
}

void PodcastChannel::dumpInfo() const
{
    qDebug() << "Channel info: "
             << m_id
             << m_title
             << m_logo
             << m_logoUrl;
}

void PodcastChannel::setDescription(const QString &desc)
{
    m_description = desc;
}

QString PodcastChannel::description() const
{
    return m_description;
}

void PodcastChannel::setXml(QByteArray xml)
{
    m_xml = std::move(xml);
}

QByteArray PodcastChannel::xml() const
{
    return m_xml;
}

void PodcastChannel::addCredentials(PodcastEpisode* episode)
{
    QUrl channelUrl(url());
    if(!channelUrl.userName().isEmpty()){
        episode->setCredentails(channelUrl.userName(), channelUrl.password());
    }
}

void PodcastChannel::setTrialUrl(const QString &url)
{
    qDebug() << "Setting Trial URL to" << url;
    m_trialURL = url;
    m_trialURLFailed = false;
}

void PodcastChannel::setIsRefreshing(bool refreshing)
{
    if (m_isRefreshing != refreshing) {
        m_isRefreshing = refreshing;
        emit channelChanged();
    }
}

bool PodcastChannel::isRefreshing() const
{
    return m_isRefreshing;
}

int PodcastChannel::unplayedEpisodes() const
{
    return m_unplayedEpisodes;
}

void PodcastChannel::setUnplayedEpisodes(int unplayed)
{
    if (unplayed != m_unplayedEpisodes) {
        m_unplayedEpisodes = unplayed;
        emit channelChanged();
    }
}

void PodcastChannel::setIsDownloading(bool downloading)
{
    if (downloading != m_isDownloading) {
        m_isDownloading = downloading;
        emit channelChanged();
        emit downloadingChanged();
    }
}

bool PodcastChannel::isDownloading() const
{
    return m_isDownloading;
}

void PodcastChannel::setAutoDownloadOn(bool autoDownloadOn) {
    if (m_autoDownloadOn != autoDownloadOn) {
        m_autoDownloadOn = autoDownloadOn;
        emit autoDownloadOnChanged();
    }
}

void PodcastChannel::trialFailed()
{
    qWarning() << "Test with URL " << m_trialURL << "failed!";
    m_trialURLFailed = true;
}

bool PodcastChannel::isAutoDownloadOn() const {
    qDebug() << "Channel autodownload: " << m_autoDownloadOn;
    return m_autoDownloadOn;
}

QString PodcastChannel::trialURL() const
{
    return m_trialURL;
}


bool PodcastChannel::operator<(const PodcastChannel &other) const
{
    return m_title < other.title();

}

QString PodcastChannel::sortBy() const
{
    switch (m_sortBy) {
    case TITLE:
        return "title";
    case STATE:
        return "state";
    case DBID:
        return "dbid";
    case PUBLISHED:
        return "published";
    }

    return "???";
}


void PodcastChannel::setSortBy(const QString& sortBy)
{

    PodcastChannelSortField old = m_sortBy;

    if (sortBy == "published")
        m_sortBy = PUBLISHED;
    else if (sortBy == "title") {
        m_sortBy = TITLE;
    }else if (sortBy == "state") {
        m_sortBy = STATE;
    }else if (sortBy == "dbid") {
        m_sortBy = DBID;
    }


    if (m_sortBy == old)
        return;

    emit channelChanged();
    emit sortByChanged(sortBy);

}

void PodcastChannel::setSortDescending(bool sortDescending)
{
    if (m_sortDescending == sortDescending)
        return;

    m_sortDescending = sortDescending;

    emit channelChanged();
    emit sortDescendingChanged(m_sortDescending);
}

void PodcastChannel::trialSucceeded()
{
    qDebug() << "Test with URL " << m_trialURL << "succeed! Updating channel URL from " << m_url << "to" << m_trialURL;
    m_url = m_trialURL;
    m_trialURL = "";
    emit urlChanged(m_url);
    emit channelChanged();
}

bool PodcastChannel::hasNew() const
{
    return m_hasNew;
}

void PodcastChannel::setHasNew(bool newHasNew)
{
    if (m_hasNew == newHasNew)
        return;
    m_hasNew = newHasNew;
    emit channelChanged();
}

bool PodcastChannel::archived() const
{
    return m_archived;
}

void PodcastChannel::setArchived(bool newArchived)
{
    m_archived = newArchived;
    emit channelChanged();
}
