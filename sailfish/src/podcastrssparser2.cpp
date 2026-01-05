/**
 * This file is part of Podcatcher for Sailfish OS.
 * Author: Moritz Carmesin (carolus@carmesinus.de)
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

#include <QXmlStreamReader>

#include <QDateTime>

#include <QtDebug>

#include "podcastrssparser2.h"


PodcastRSSParser2::PodcastRSSParser2(QObject *parent) :
    QObject(parent)
{
}


bool PodcastRSSParser2::populateChannelFromChannelXML(PodcastChannel *channel, const QByteArray& xmlReply)
{
    qDebug() << "Parsing XML for channel URL" << channel->url();

    if (channel == nullptr) {
        return false;
    }

    if (xmlReply.size() < 10) {
        return false;
    }

    QXmlStreamReader xml(xmlReply);

    bool valid = false;

    if (xml.readNextStartElement()){
        qDebug() << "Feed root element: " << xml.name();
        while(xml.readNextStartElement()){
            if (xml.name() == "channel"){
                valid = parseRSSChannel(channel, xml);
            }else if(xml.name() == "feed"){
                valid = parseAtomChannel(channel, xml);
            }else {
                xml.skipCurrentElement();
            }
        }
    }
    if (!valid)
        return false;

    channel->dumpInfo();

    channel->xml().clear();
    channel->xml().squeeze();
    qDebug() << "Cleared XML data from channel " << channel->title();

    return true;

}


QList<PodcastEpisode *>* PodcastRSSParser2::populateEpisodesFromChannelXML(const QByteArray& xmlReply, QObject *episodeParent)
{
    qDebug() << "Parsing XML for episodes";

    if (xmlReply.size() < 10) {
        return nullptr;
    }

    QXmlStreamReader xml (xmlReply);

    if (xml.readNextStartElement()){
        qDebug() << "Feed root element: " << xml.name();
        while (xml.readNextStartElement())
            if (xml.name() == "channel")
                return episodesFromRSS(xml, episodeParent);
            else if(xml.name() == "feed"){
                return episodesFromAtom(xml,episodeParent);
            }
            else {
                xml.skipCurrentElement();
            }

    }

    return nullptr;
}


bool PodcastRSSParser2::isValidPodcastFeed(const QByteArray& xmlReply)
{
    qDebug() << "Checking if podcast feed is valid.";
    if (xmlReply.size() < 10) {
        qDebug() << "Not valid!";
        return false;
    }


    QXmlStreamReader xml (xmlReply);

    bool itemWithEnclosureFound = false;

    if (xml.readNextStartElement()){
        qDebug() << "Feed root element: " << xml.name();
    }else{
        return false;
    }

    if (xml.readNextStartElement())
    {
        if (!(xml.name()== "channel" || xml.name() == "feed" ))
            return false;

        else
            return true;
        /*
        while (xml.readNextStartElement()){
            if (xml.name() == "item"){
                while (!itemWithEnclosureFound && xml.readNextStartElement()){
                    if(xml.name() == "pubdate" || xml.name()=="published" ||
                            xml.name() == "date"){

                        QDateTime pubdate = parsePubDate(xml.readElementText());
                        if (!pubdate.isValid()){
                            qWarning() << "Could not parse pubdate";
                            return false;
                        }
                    }else if(xml.name() == "enclosure"){
                        itemWithEnclosureFound = true;
                        xml.skipCurrentElement();
                    }
                    else{
                        xml.skipCurrentElement();
                    }
                }

            }else
                xml.skipCurrentElement();

        }

        if (!itemWithEnclosureFound){
            qDebug() << "Podcast feed does not contain the <enclosure> tag.";
        }

        return itemWithEnclosureFound;
        */
    }


    return false;
}

bool PodcastRSSParser2::parseRSSChannel(PodcastChannel *channel, QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("title"))
            channel->setTitle(xml.readElementText());
        else if (xml.name() == QLatin1String("description"))
            channel->setDescription(xml.readElementText());
        else if (xml.name() == QLatin1String("image"))
            parseRSSImage(channel, xml);
        else
            xml.skipCurrentElement();
    }
    return true;
}


bool PodcastRSSParser2::parseRSSImage(PodcastChannel *channel, QXmlStreamReader &xml){
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("image"));

    bool itunes = xml.namespaceUri() == "http://www.itunes.com/dtds/podcast-1.0.dtd";

    if (itunes){
        auto attr = xml.attributes();
        if (attr.hasAttribute("href")){
            channel->setLogoUrl(attr.value("href").toString());
        }
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("url")){
            channel->setLogoUrl(xml.readElementText());
        }else
            xml.skipCurrentElement();
    }

    return true;
}

bool PodcastRSSParser2::parseAtomChannel(PodcastChannel *channel, QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("title"))
            channel->setTitle(xml.readElementText());
        else if (xml.name() == QLatin1String("subtitle"))
            channel->setDescription(xml.readElementText());
        else if (xml.name() == QLatin1String("logo"))
            channel->setLogoUrl(xml.readElementText());
        else
            xml.skipCurrentElement();
    }

    return true;
}

QList<PodcastEpisode *> *PodcastRSSParser2::episodesFromRSS(QXmlStreamReader& xml, QObject *episodeParent)
{
    auto *episodes = new QList<PodcastEpisode *>();

    while (xml.readNextStartElement()){
        //qDebug()<<"Next element in channel is a "<< xml.name();
        if (xml.name() == "item"){
            bool empty = true;
            auto *episode = new PodcastEpisode();
            episode->moveToThread(episodeParent->thread());

            QString desc = "";
            QString title = "";

            while(xml.readNextStartElement()){
                //qDebug() << "Next element in item is" << xml.name();

                if(xml.name() == "title"){
                    QString text = xml.readElementText();
                    if(title.length() < text.length())
                        title = text;
                }

                else if(xml.name() == "pubdate" || xml.name() =="pubDate" || xml.name()=="published" ||
                        xml.name() == "date"){

                    QDateTime pubdate = parsePubDate(xml.readElementText());
                    if (!pubdate.isValid()){
                        qWarning() << "Could not parse pubdate";
                        episode->setDownloadLink("");
                        break;
                    }
                    episode->setPubTime(pubdate);
                }

                else if(xml.name() == "description" || xml.name() == "summary" ||
                        (xml.name() == "encoded" /*&& xml.namespaceUri()=="http://purl.org/rss/1.0/modules/content/"*/)){
                    QString text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    if (text.length() > desc.length())
                        desc = text;
                    //episode->setDescription(xml.readElementText(QXmlStreamReader::IncludeChildElements));
                }

                else if (xml.name() == "duration" && xml.namespaceUri() == "http://www.itunes.com/dtds/podcast-1.0.dtd")
                    episode->setDuration(xml.readElementText());

                else if(xml.name() == "enclosure"){
                    auto attr = xml.attributes();
                    if (attr.hasAttribute("url"))
                        episode->setDownloadLink(attr.value("url").toString());
                    if (attr.hasAttribute("length"))
                        episode->setDownloadSize(attr.value("length").toUInt());
                    xml.skipCurrentElement();
                }else{
                    //qDebug() << "Skipping " << xml.name();
                    xml.skipCurrentElement();
                }
            }

            empty = episode->downloadLink().isEmpty();

            if (empty){
                delete episode;
            }else{
                episode->setTitle(title);
                episode->setDescription(desc);
                episode->setNew(true);
                episodes->append(episode);
            }
        }else
            xml.skipCurrentElement();
    }


    return episodes;
}

QList<PodcastEpisode *> *PodcastRSSParser2::episodesFromAtom(QXmlStreamReader &xml, QObject *episodeParent)
{
    auto *episodes = new QList<PodcastEpisode *>();

    while( xml.readNextStartElement()){
        if (xml.name() == "entry"){
            auto *episode = new PodcastEpisode();
            episode->moveToThread(episodeParent->thread());

            QString desc = "";
            QString title = "";

            while(xml.readNextStartElement()){
                if(xml.name() == "title"){
                    QString text = xml.readElementText();
                    if(title.length() < text.length())
                        title = text;
                }

                else if(xml.name() == "pubdate" || xml.name()=="published" ||
                        xml.name() == "date"){

                    QDateTime pubdate = parsePubDate(xml.readElementText());
                    if (!pubdate.isValid()){
                        qWarning() << "Could not parse pubdate";
                        episode->setDownloadLink("");
                        break;
                    }

                    episode->setPubTime(pubdate);
                }

                else if(xml.name() == "description" || xml.name() == "summary" ||
                        (xml.name() == "encoded" /*&& xml.namespaceUri()=="http://purl.org/rss/1.0/modules/content/"*/)){
                    QString text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    if (text.length() > desc.length())
                        desc = text;
                }

                else if (xml.name() == "duration" && xml.namespaceUri() == "http://www.itunes.com/dtds/podcast-1.0.dtd")
                    episode->setDuration(xml.readElementText());

                else if(xml.name() == "enclosure"){
                    auto attr = xml.attributes();
                    if (attr.hasAttribute("url"))
                        episode->setDownloadLink(attr.value("url").toString());
                    if (attr.hasAttribute("length"))
                        episode->setDownloadSize(attr.value("length").toUInt());
                    xml.skipCurrentElement();
                }
                else{
                    xml.skipCurrentElement();
                }

            }

            if (episode->downloadLink().isEmpty()){
                delete episode;
            }else{
                episode->setTitle(title);
                episode->setDescription(desc);
                episode->setNew(true);
                episodes->append(episode);
            }

        }else{
            xml.skipCurrentElement();
        }

    }

    return episodes;
}

QDateTime PodcastRSSParser2::parsePubDate(const QString &pubDateRef)
{
    if (pubDateRef.isEmpty()) {
        qDebug() << "Could not find pubDate attribute. Giving up...";
        return QDateTime();
    }

    QString pubDateString = trimPubDate(pubDateRef);

    QLocale loc(QLocale::C);
    QString tryParseDate = pubDateString.left(QString("dd MMM yyyy HH:mm:ss").length());  // QDateTime cannot parse RFC 822 time format, so remove the timezone information from it.
    QDateTime pubDate = loc.toDateTime(tryParseDate,
                                       "dd MMM yyyy HH:mm:ss");
    //qDebug() << "we parse from" <<tryParseDate <<": " <<pubDate;


    if (!pubDate.isValid()) {
        // We probably could not parse the date which in some broken podcast feeds is
        // defined only by one integer instead of two (like "2 Jul" instead of "02 Jul")
        // I am looking at you Skeptics Guide to the Universe!

        qDebug() << "Could not parse pubDate. Trying with just one date integer.";

        tryParseDate = pubDateString.left(QString("d MMM yyyy HH:mm:ss").length());
        pubDate = loc.toDateTime(tryParseDate,
                                 "d MMM yyyy HH:mm:ss");
    }

    if (!pubDate.isValid()) {
        // Let's try just once more just to please Hacker Public Radio
        qDebug() << "Could not parse pubDate. Trying with an odd format from Hacker Public Radio";

        tryParseDate = pubDateString.left(QString("yyyy-MM-dd").length());
        pubDate = loc.toDateTime(tryParseDate,
                                 "yyyy-MM-dd");
    }

    pubDate.setTimeSpec(Qt::UTC); // Interprete Time as UTC, since we do not parse the timezone

    //FIXME: Add parsing of timezone

    return pubDate;
}


