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
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>

#include <QtDebug>

#include "podcastglobals.h"
#include "podcastsqlmanager.h"


PodcastSQLManager* PodcastSQLManagerFactory::m_instance = nullptr;
PodcastSQLManagerFactory::PodcastSQLManagerFactory()
= default;

PodcastSQLManager* PodcastSQLManagerFactory::sqlmanager() {
    // Database connection is already open, so just return it.
    if (m_instance == nullptr) {
        m_instance = new PodcastSQLManager;
    }
    return m_instance;
}


PodcastSQLManager::PodcastSQLManager(QObject *parent) :
    QObject(parent)
{
    QString databasePath;
    //databasePath = PODCATCHER_PATH;
    databasePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    // Create the database in config dir
    QDir dirpath(databasePath);
    if (!dirpath.exists()) {
        dirpath.mkpath(databasePath);
    }

    m_connection = QSqlDatabase::addDatabase("QSQLITE");
    m_connection.setDatabaseName(databasePath + "/" + "podcatcher.sql");

    if (!m_connection.open()) {
        qWarning() << "Could not open database with path " << databasePath;
        return;
    }

    createTables();

}

int PodcastSQLManager::podcastChannelToDB(PodcastChannel *channel)
{
    if (channel == nullptr) {
        return 0;
    }

    mutex.lock();
    if (!m_connection.isOpen()) {
        qWarning() << "SQL connection not open. Returning.";
        mutex.unlock();
        return 0;
    }

    mutex.unlock();  // isChannelInDB() is also locking...
    if (isChannelInDB(channel)) {
        return 0;
    }
    mutex.lock();

    QSqlQuery q(m_connection);

    // Item not found in database. Go ahead and insert it.
    m_connection.transaction();

    q.prepare("INSERT INTO channels(rssurl, title, description, logo, autoDownloadOn) VALUES (:url, :title, :desc, :logo, :autoDownloadOn)");
    q.bindValue(":url", channel->url());
    q.bindValue(":desc", channel->description());
    q.bindValue(":title", channel->title());
    q.bindValue(":logo", channel->logo());
    q.bindValue(":autoDownloadOn", channel->isAutoDownloadOn());
    q.exec();

    m_connection.commit();

    mutex.unlock();

    // Update the channel with the id it got in DB
    channel->setId(q.lastInsertId().toInt());

    return q.numRowsAffected();
}

bool PodcastSQLManager::isChannelInDB(PodcastChannel *channel)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    // Find out if the channel is already in our DB.
    // Do not add if the channel is already there.
    q.prepare("SELECT COUNT(id) FROM channels WHERE rssurl=:url");
    q.bindValue(":url", channel->url());
    if (!q.exec()) {
        qDebug() << Q_FUNC_INFO << q.lastError().text();
    }

    if (!q.next()) {
        return false; // DB is probably empty.
    }

    if (q.value(0).toInt() > 0) {   // Channel already exists in the DB - do nothing.
        return true;
    }

    return false;
}

QList<PodcastChannel *> PodcastSQLManager::channelsInDB(bool archived)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    QList<PodcastChannel *> channels;

    qDebug() << "Returning Podcast channels from DB:";

    q.prepare("SELECT id, title, description, logo, rssurl, "
              "(SELECT COUNT(id) FROM episodes WHERE episodes.channelid = channels.id AND episodes.lastPlayed = 0 AND episodes.playLocation <> ''), "
              "autoDownloadOn, sortby, sortdescending "
              "FROM channels WHERE archived=:archived"
              " ORDER BY channels.title");
    q.bindValue(":archived", archived);

    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "Last query: " << q.lastQuery();
    }

    while (q.next()) {
        auto *channel = new PodcastChannel;
        channel->setId(q.value(0).toInt());
        channel->setTitle(q.value(1).toString());
        channel->setDescription(q.value(2).toString());
        channel->setLogo(q.value(3).toString());
        channel->setUrl(q.value(4).toString());
        channel->setUnplayedEpisodes(q.value(5).toInt());
        channel->setAutoDownloadOn(q.value(6).toBool());
        channel->setSortBy(q.value(7).toString());
        channel->setSortDescending(q.value(8).toBool());
        channel->setArchived(archived);

        channels.append(channel);
    }

    return channels;
}

PodcastChannel * PodcastSQLManager::channelInDB(int channelId, PodcastChannel *channel)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    qDebug() << "Returning Podcast channel from DB with id" << channelId;

    q.prepare("SELECT title, description, logo, rssurl, "
              "(SELECT COUNT(id) FROM episodes WHERE episodes.channelid = channels.id AND episodes.lastPlayed = 0 AND episodes.playLocation <> ''), "
              "autoDownloadOn, sortby, sortdescending, archived "
              "FROM channels WHERE channels.id = :id");
    q.bindValue(":id", channelId);
    q.exec();
    if (!q.next()) {
        qWarning() << "SQL error: " << q.lastError();
        qWarning() << "Last query:" << q.lastQuery();
        return nullptr;
    }

    if (channel == nullptr) {
        channel = new PodcastChannel;
    }
    channel->setId(channelId);
    channel->setTitle(q.value(0).toString());
    channel->setDescription(q.value(1).toString());
    channel->setLogo(q.value(2).toString());
    channel->setUrl(q.value(3).toString());
    channel->setUnplayedEpisodes(q.value(4).toInt());
    channel->setAutoDownloadOn(q.value(5).toBool());
    channel->setSortBy(q.value(6).toString());
    channel->setSortDescending(q.value(7).toBool());
    channel->setArchived(q.value(8).toBool());

    return channel;
}



int PodcastSQLManager::podcastEpisodesToDB(QList<PodcastEpisode *> parsedEpisodes, int channel_id)
{
    qDebug() << "Got" << parsedEpisodes.length() << "episodes for channel" << channel_id;

    mutex.lock();
    if (!m_connection.isOpen()) {
        qWarning() << "SQL connection not open. Returning.";
        mutex.unlock();
        return 0;
    }

    QSqlQuery q(m_connection);
    m_connection.transaction();

    foreach(PodcastEpisode* episode, parsedEpisodes) {
        q.prepare("INSERT INTO episodes (title, channelid, downloadLink, playLocation, description, published, duration, downloadSize, lastPlayed, hasBeenCanceled, finished, playPosition) VALUES "
                  "(:title, :channelid, :downloadLink, :playLocation, :description, :published, :duration, :downloadSize, :lastPlayed, :hasBeenCanceled, :finished, :playPosition)");
        q.bindValue(":title", episode->title());
        q.bindValue(":channelid", channel_id);
        q.bindValue(":downloadLink", episode->downloadLink());
        q.bindValue(":playLocation", episode->playFilename());
        q.bindValue(":description", episode->description());
        q.bindValue(":published", episode->pubTime().toTime_t());  // NOTE: We save the seconds since EPOC for easier handling.
        q.bindValue(":duration", episode->duration());
        q.bindValue(":downloadSize", episode->downloadSize());
        q.bindValue(":lastPlayed", episode->lastPlayed().isValid() ? episode->lastPlayed().toTime_t() : 0);  // NOTE: We save the seconds since EPOC for easier handling.
        q.bindValue(":hasBeenCanceled", episode->hasBeenCanceled());
        q.bindValue(":finished", episode->finished());
        q.bindValue(":playPosition", episode->playPosition());

        if (!q.exec()) {
            qDebug() << "Last query: " << q.lastQuery();
            qDebug() << "Error: " << q.lastError();
        }

        episode->setDbId(q.lastInsertId().toInt());
        qDebug() << "Giving episode a DB ID:" << episode->dbid();
    }

    m_connection.commit();
    mutex.unlock();

    return q.numRowsAffected();
}

bool PodcastSQLManager::podcastEpisodeToDB(PodcastEpisode *episode, int channel_id)
{
    if (episode == nullptr) {
        return false;
    }

    mutex.lock();
    if (!m_connection.isOpen()) {
        qWarning() << "SQL connection not open. Returning.";
        mutex.unlock();
        return false;
    }

    QSqlQuery q(m_connection);
    q.prepare("INSERT INTO episodes(title, channelid, downloadLink, playLocation, description, published, duration, downloadSize, lastPlayed, hasBeenCanceled, finished, playPosition) VALUES "
              "(:title, :channelid, :downloadLink, :playLocation, :description, :published, :duration, :downloadSize, :lastPlayed, :hasBeenCanceled, :finished, :playPosition)");
    q.bindValue(":title", episode->title());
    q.bindValue(":channelid", channel_id);
    q.bindValue(":downloadLink", episode->downloadLink());
    q.bindValue(":playLocation", episode->playFilename());
    q.bindValue(":description", episode->description());
    q.bindValue(":published", episode->pubTime().toTime_t());  // NOTE: We save the seconds since EPOC for easier handling.
    q.bindValue(":duration", episode->duration());
    q.bindValue(":downloadSize", episode->downloadSize());
    q.bindValue(":finished", episode->finished());
    q.bindValue(":playPosition", episode->playPosition());

    QDateTime lastPlayed = episode->lastPlayed();
    if (lastPlayed.isValid()) {
        q.bindValue(":lastPlayed", episode->lastPlayed().toTime_t());
    } else {
        q.bindValue(":lastPlayed", 0);
    }

    q.bindValue(":hasBeenCanceled", episode->hasBeenCanceled());

    if (!q.exec()) {
        qDebug() << "Last query: " << q.lastQuery();
        qDebug() << "Error: " << q.lastError();
        mutex.unlock();
        return false;
    }

    mutex.unlock();
    return true;
}

QList<PodcastEpisode *> PodcastSQLManager::episodesInDB(PodcastChannel* channel)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    QList<PodcastEpisode *> episodes;

    int channelId = channel->channelDbId()? channel->channelDbId():-1;

    qDebug() << "Returning Podcast episodes from DB for channel:" << channelId;

    q.prepare("SELECT id, title, downloadLink, playLocation, description, published, duration, downloadSize, channelid, lastPlayed, hasBeenCanceled, finished, playPosition "
              "FROM episodes WHERE episodes.channelid = :chanId ORDER BY episodes.published DESC");
    q.bindValue(":chanId", channelId);


    if (!q.exec()) {
        qWarning() << "Fetching episodes, SQL error: " << q.lastError();
        qWarning() << "Last query:" << q.lastQuery();
        return episodes;
    }

    while (q.next()) {
        auto *episode = new PodcastEpisode;
        episode->setDbId(q.value(0).toInt());
        episode->setTitle(q.value(1).toString());
        episode->setDownloadLink(q.value(2).toString());
        episode->setPlayFilename(q.value(3).toString());
//      episode->setDescription(q.value(4).toString());
        episode->setPubTime(QDateTime::fromTime_t(q.value(5).toUInt()));
        episode->setDuration(q.value(6).toString());
        episode->setDownloadSize(q.value(7).toInt());
        if (q.value(9).toInt() > 0) {
            episode->setLastPlayed(QDateTime::fromTime_t(q.value(9).toUInt()));
        }
        episode->setHasBeenCanceled(q.value(10).toBool());
        episode->setFinished(q.value(11).toBool());
        episode->setPlayPosition(q.value(12).toInt());

        // Since we requested channels for this channel, we might as well be sure the value is what we requested as parameter.
        episode->setChannel(channel);

        episodes.append(episode);
    }

    qDebug() << "Fetched" << episodes.size() << "episodes.";
    return episodes;
}

QDateTime PodcastSQLManager::latestEpisodeTimestampInDB(int channelId)
{
    QDateTime latestDate = QDateTime();
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    q.prepare("SELECT published FROM episodes WHERE episodes.channelid = :chanId ORDER BY episodes.published DESC LIMIT 1");
    q.bindValue(":chanId", channelId);

    if (q.exec()) {
        q.next();
        uint timestamp = q.value(0).toUInt();
        latestDate = QDateTime::fromTime_t(timestamp);

        qDebug() << "Got latest episode timestamp: " << latestDate;

    } else {
        qWarning() << "SQL error: " << q.lastError();
        qWarning() << "SQL query: " << q.lastQuery();
    }

    return latestDate;
}

bool PodcastSQLManager::updateChannelInDB(PodcastChannel *channel) {
    qDebug() << "Updating podcast channel data to DB";
    mutex.lock();
    if (!m_connection.isOpen()) {
        mutex.unlock();
        qWarning() << "SQL connection not open. Returning.";
        return false;
    }

    QSqlQuery q(m_connection);
    mutex.unlock();

    q.prepare(/*"UPDATE channels SET title=:title, description=:description, logo=:logo, rssurl=:rssurl, autoDownloadOn=:autoDownloadOn, sortby=:sortBy, sortdescending=:sortDescending, "*/
              "UPDATE channels SET title=:title, logo=:logo, rssurl=:rssurl, autoDownloadOn=:autoDownloadOn, sortby=:sortBy, sortdescending=:sortDescending, "
              "archived=:archived "
              "WHERE id=:id");
    q.bindValue(":title", channel->title());
//    q.bindValue(":description", channel->description());
    q.bindValue(":logo", channel->logo());
    q.bindValue(":rssurl", channel->url());
    q.bindValue(":autoDownloadOn", channel->isAutoDownloadOn());
    q.bindValue(":id", channel->channelDbId());
    q.bindValue(":sortBy", channel->sortBy());
    q.bindValue(":sortDescending", channel->sortDescending());
    q.bindValue(":archived", channel->archived());

    if (!q.exec()) {
        qDebug() << "Last query: " << q.lastQuery();
        qDebug() << "Error: " << q.lastError();
        mutex.unlock();
        return false;
    }

    mutex.unlock();
    return true;
}

void PodcastSQLManager::updatePodcastInDB(PodcastEpisode *episode)
{
    qDebug() << "Updating episode data to DB";
    mutex.lock();
    if (!m_connection.isOpen()) {
        mutex.unlock();
        qWarning() << "SQL connection not open. Returning.";
        return;
    }

    QSqlQuery q(m_connection);
    mutex.unlock();

    q.prepare(//"UPDATE episodes SET title=:title, downloadLink=:downloadLink, playLocation=:playLocation, description=:description, "
              "UPDATE episodes SET title=:title, downloadLink=:downloadLink, playLocation=:playLocation,"
              "published=:published, duration=:duration, downloadSize=:downloadSize, lastPlayed=:lastPlayed, hasBeenCanceled=:hasBeenCanceled, "
              "finished=:finished, playPosition=:playPosition "
              "WHERE id=:id");
    q.bindValue(":title", episode->title());
    q.bindValue(":downloadLink", episode->downloadLink());
    q.bindValue(":playLocation", episode->playFilename());
    //q.bindValue(":description", episode->description());
    q.bindValue(":published", episode->pubTime().toTime_t());  // NOTE: We save the seconds since EPOC for easier handling.
    q.bindValue(":duration", episode->duration());
    q.bindValue(":downloadSize", episode->downloadSize());
    q.bindValue(":id", episode->dbid());
    q.bindValue(":lastPlayed", episode->lastPlayed().isValid() ? episode->lastPlayed().toTime_t() : 0);  // NOTE: We save the seconds since EPOC for easier handling.
    q.bindValue(":hasBeenCanceled", episode->hasBeenCanceled());
    q.bindValue(":finished", episode->finished());
    q.bindValue(":playPOsition",episode->playPosition());

    if (!q.exec()) {
        qDebug() << "Last query: " << q.lastQuery();
        qDebug() << "Error: " << q.lastError();
        return;
    }

    qDebug() << "Updated episode:" << episode->dbid() << "last played:" << episode->lastPlayed();
}

void PodcastSQLManager::removeChannelFromDB(int channelId)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    qDebug() << "Deleting all episodes from DB with channel: " << channelId;

    q.prepare("DELETE FROM episodes WHERE episodes.channelId = :chanId");
    q.bindValue(":chanId", channelId);
    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "SQL query:" << q.lastQuery();
    }

    qDebug() << "Deleting the channel from DB with channel: " << channelId;

    q.clear();

    q.prepare("DELETE FROM channels WHERE channels.id = :chanId");
    q.bindValue(":chanId", channelId);
    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "SQL query:" << q.lastQuery();
    }

}

bool PodcastSQLManager::removePodcastFromDB(PodcastEpisode *episode)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    qDebug() << "Deleting episode from DB with id: " << episode->dbid();

    q.prepare("DELETE FROM episodes WHERE episodes.id = :episodeId");
    q.bindValue(":episodeId", episode->dbid());
    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "SQL query:" << q.lastQuery();
        return false;
    }

    return true;
}

void PodcastSQLManager::createTables()
{
    // If database does not contain table "channels", so create it.
    if (!m_connection.tables().contains("channels")) {
        QSqlQuery q(m_connection);
        m_connection.transaction();

        qDebug() << "Creating table 'channels'";

        q.exec("CREATE TABLE channels (id INTEGER PRIMARY KEY, "
                                      "rssurl TEXT, "
                                      "title TEXT, "
                                      "description TEXT, "
                                      "logo TEXT, "
                                      "sortby TEXT DEFAULT 'published',"
                                      "sortdescending BOOLEAN DEFAULT true,"
                                      "autoDownloadOn BOOLEAN)");

        if (!m_connection.commit()) {
            qDebug() << "SQL error: " << m_connection.lastError().text();
        }

        if (!q.isValid()) {
            qDebug() << q.lastError().text();
        }

    }
    else {
        checkAndCreateMissingChannelColumns();
    }

    // If database does not contain the table "episodes", so crate it.
    if (!m_connection.tables().contains("episodes")) {
        QSqlQuery q(m_connection);
        m_connection.transaction();

        qDebug() << "Creating table 'episodes'";

        q.exec("CREATE TABLE episodes (id INTEGER PRIMARY KEY, "
                                      "title TEXT, "
                                      "channelid INTEGER, "
                                      "downloadLink TEXT, "
                                      "lastPlayed INTEGER, "
                                      "playLocation TEXT, "
                                      "description TEXT, "
                                      "published INTEGER, "
                                      "duration TEXT, "
                                      "downloadSize INTEGER, "
                                      "hasBeenCanceled BOOLEAN, "
                                      "finished BOOLEAN, "
                                      "playPosition INTEGER, "
                                      "FOREIGN KEY(channelid) REFERENCES channels(id))");

        if (!m_connection.commit()) {
            qDebug() << "SQL error: " <<  m_connection.lastError().text();
        }

        if (!q.isValid()) {
            qDebug() << q.lastError().text();
        }
    }else{
        checkAndCreateExtendedEpisodeState();
    }

}

void PodcastSQLManager::checkAndCreateExtendedEpisodeState()
{
    QSqlQuery q(m_connection);

    if (!q.exec("SELECT finished FROM episodes")) {
        qDebug() << "SQL: episodes does not contain 'finished'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE episodes ADD COLUMN finished BOOLEAN")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }

    if (!q.exec("SELECT playPosition FROM episodes")) {
        qDebug() << "SQL: episodes does not contain 'playPosition'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE episodes ADD COLUMN playPosition INTEGER")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }

}

void PodcastSQLManager::migrateChannelLogos()
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();

    qDebug() << "Checking Podcast channel logos:";

    q.prepare("SELECT id,  logo FROM channels");

    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "Last query: " << q.lastQuery();
    }

    QString newPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    while (q.next()) {
        int id =  q.value(0).toInt();
        QString path;
        QUrl url = q.value(1).toUrl();

        path = url.toLocalFile();

        if (path.isEmpty()){
            path = q.value(1).toString();
        }

        qDebug() << id << path;

        if (path.startsWith(newPath)){
            qDebug() << "already migrated!";
        }else if(path.startsWith(PODCATCHER_PATH_OLD)){
            qDebug() << "old location, migrating…";
            QString filename = url.fileName();
            QString newLogoPath = QString("%1/%2").arg(newPath).arg(filename);
            qDebug() << "Moving file from "<< path <<" to "<< newLogoPath;
            QFile::rename(path, newLogoPath);
            mutex.lock();
            QSqlQuery q2(m_connection);
            mutex.unlock();


            q2.prepare("UPDATE channels SET logo=:logo WHERE id=:id");
            q2.bindValue(":logo", newLogoPath);
            q2.bindValue(":id", id);
            if (!q2.exec()) {
                qWarning() << "SQL error:" << q2.lastError();
                qWarning() << "Last query: " << q2.lastQuery();
            }else{
                qDebug() << "Database updated!";
            }

        }else{
            qWarning() << "unexpected path:"<<path;
        }



    }
}

QString PodcastSQLManager::episodeDescription(PodcastEpisode &episode)
{
    mutex.lock();
    QSqlQuery q(m_connection);
    mutex.unlock();


    qDebug() << "Returning episode description for " << episode.title();

    q.prepare("SELECT description "
              "FROM episodes WHERE id = :id");
    q.bindValue(":id", episode.dbid());

    if (!q.exec()) {
        qWarning() << "SQL error:" << q.lastError();
        qWarning() << "Last query: " << q.lastQuery();
    }


    if (q.next()){
        return q.value(0).toString();
    }else{
        throw std::runtime_error("Could not retrieve episode description from database!");
    }
}



void PodcastSQLManager::checkAndCreateAutoDownload(bool autoDownload)
{
    QSqlQuery q(m_connection);

    if (!q.exec("SELECT autoDownloadOn FROM channels")) {
        qDebug() << "SQL: channels does not contain 'autoDownloadOn'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE channels ADD COLUMN autoDownloadOn BOOLEAN")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }

    // Find out if we have any results from the previous SELECT.
    bool foundAutoDownloads = false;
    while(q.next()) {
        qDebug() << "SQL: Found autodownload values in DB. Not setting default ones.";
        foundAutoDownloads = true;
        break;
    }

    if (!foundAutoDownloads) {
        qDebug() << "SQL: No auto dowload values in DB. Setting defaults (from Settings).";

        q.clear();

        q.prepare("UPDATE channels SET autoDownloadOn=:autoDownloadOn");
        q.bindValue(":autoDownloadOn", autoDownload);

        if (!q.exec()) {
            qDebug()   << "SQL error: " <<  q.lastError().text();
            qWarning() << "SQL query:"  <<  q.lastQuery();
        }
    }

}

void PodcastSQLManager::checkAndCreateMissingChannelColumns()
{
    QSqlQuery q(m_connection);

    if (!q.exec("SELECT sortby FROM channels")) {
        qDebug() << "SQL: channels does not contain 'sortby'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE channels ADD COLUMN sortby TEXT DEFAULT 'published' ")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }

    if (!q.exec("SELECT sortdescending FROM channels")) {
        qDebug() << "SQL: channels does not contain 'sortdescending'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE channels ADD COLUMN sortdescending BOOLEAN DEFAULT true ")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }

    if (!q.exec("SELECT archived FROM channels")) {
        qDebug() << "SQL: channels does not contain 'archived'. Adding column.";

        QSqlQuery q_create(m_connection);

        // Column does not exist. Create it.
        if (!q_create.exec("ALTER TABLE channels ADD COLUMN archived BOOLEAN DEFAULT false ")) {
            qDebug()   << "SQL error: " <<  q_create.lastError().text();
            qWarning() << "SQL query:"  <<  q_create.lastQuery();
        }
    }
}

void PodcastSQLManager::updateChannelAutoDownloadToDB(bool autoDownloadOn)
{
    QSqlQuery q(m_connection);

    q.prepare("UPDATE channels SET autoDownloadOn=:autoDownload");
    q.bindValue(":autoDownload", autoDownloadOn);
    if (!q.exec()) {
        qDebug()   << "SQL error: " <<  q.lastError().text();
        qWarning() << "SQL query:"  <<  q.lastQuery();
    }

}



