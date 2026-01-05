#include "mediametadataextractor.h"
#include <QtDebug>

#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <tmap.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <tableofcontentsframe.h>
#include <chapterframe.h>
#include <textidentificationframe.h>

#include <mp4coverart.h>
#include <mp4file.h>



#include <vorbisfile.h>
#include <opusfile.h>
#include <xiphcomment.h>
#include <flacpicture.h>

using namespace TagLib;

#include <iostream>
#include <iomanip>

using namespace std;

MediaMetaDataExtractor::MediaMetaDataExtractor(QObject *parent) :
    QObject(parent), imageProvider(*this){

}

void MediaMetaDataExtractor::inspect(QString url)
{
    qDebug() << "Inspecting metadata from file " << url;
    reset();
    url.replace("file://","");

    if (url.endsWith("mp3",Qt::CaseInsensitive)){
        cout << "MP3" << endl;
        inspectMP3(url);

    }else if (url.endsWith("m4a", Qt::CaseInsensitive)) {
        cout << "MP4 Audio" << endl;
        inspectM4A(url);
    }else if (url.endsWith("ogg", Qt::CaseInsensitive)) {
        cout << "OGG" << endl;
        inspectOGG(url);
    }else if (url.endsWith("opus", Qt::CaseInsensitive)) {
        cout << "OPUS" << endl;
        inspectOPUS(url);
    }


    FileRef f(url.toLocal8Bit());

    if(!f.isNull() && f.tag()) {

        TagLib::Tag *tag = f.tag();
        m_title = QString::fromStdString(tag->title().to8Bit(true));
        emit titleChanged(m_title);

        m_podcast = QString::fromStdString(tag->album().to8Bit(true));
        emit podcastChanged(m_podcast);

        m_artist = QString::fromStdString(tag->artist().to8Bit(true));
        emit artistChanged(m_artist);

    }else{
        return;
    }

}

void MediaMetaDataExtractor::reset()
{
    m_title = "";
    m_podcast = "";
    m_artist = "";

    m_cover = QImage();

    m_chapters.clear();
    emit chaptersChanged(&m_chapters);

    emit titleChanged(m_title);
    emit podcastChanged(m_podcast);
    emit artistChanged(m_artist);
}

void MediaMetaDataExtractor::inspectMP3(const QString& url)
{
    MPEG::File mf(url.toLocal8Bit());
    if(!mf.isValid())
        return;

    if (mf.hasID3v2Tag()){
        ID3v2::Tag * tag = mf.ID3v2Tag();
        ID3v2::FrameList fl = tag->frameListMap()["APIC"];


        for (auto it = fl.begin(); it != fl.end() ; it++)        {
            auto * pictureFrame = dynamic_cast<ID3v2::AttachedPictureFrame *> (*it);
            qDebug() <<"MIME type of frame "<< pictureFrame->mimeType().toCString();
            if(pictureFrame->type() == ID3v2::AttachedPictureFrame::Type::Media ||
                    pictureFrame->type() == ID3v2::AttachedPictureFrame::Type::FrontCover){
                if(m_cover.loadFromData((const uchar*) pictureFrame->picture().data(), pictureFrame->picture().size()))
                    break;
            }
            if (m_cover.isNull() && pictureFrame->type() == ID3v2::AttachedPictureFrame::Type::Other){
                m_cover.loadFromData((const uchar*) pictureFrame->picture().data(), pictureFrame->picture().size());
            }

        }

        fl = tag->frameListMap()["CTOC"];

        ByteVectorList chapterIDS;

        for (auto & it : fl){
            auto* toc = dynamic_cast< ID3v2::TableOfContentsFrame*> (it);

            if(!toc->isTopLevel())
                continue;

            qDebug() <<toc->elementID().toUInt32LE(0) <<"entry Count" << toc->entryCount() << "toString "<<toc->toString().toCString(true);

            chapterIDS = toc->childElements();

            for (uint i = 0; i < toc->entryCount(); i++){
                ByteVector& child = toc->childElements()[i];
                QString childName = QString::fromLocal8Bit(child.data(), child.size());
                qDebug() <<childName;
            }
        }



        for (uint i = 0; i<chapterIDS.size(); i++){
            ID3v2::ChapterFrame* cf = ID3v2::ChapterFrame::findByElementID(tag,chapterIDS[i]);

            qDebug() <<cf->toString().toCString();

            PodcastChapter chap;
            chap.start = cf->startTime();

            ID3v2::FrameList efl = cf->embeddedFrameListMap()["TIT2"];

            if(!efl.isEmpty()){
                auto* titleFrame = dynamic_cast<ID3v2::TextIdentificationFrame*>(efl.front());
                qDebug() << titleFrame->fieldList().toString("|").toCString(true);
                chap.name = titleFrame->fieldList().toString("|").toCString(true);
            }

            efl = cf->embeddedFrameListMap()["APIC"];

            if(!efl.isEmpty()){
                auto * pictureFrame = dynamic_cast<ID3v2::AttachedPictureFrame *> (efl.front());
                qDebug() <<"MIME type of frame "<< pictureFrame->mimeType().toCString();
                chap.picture.loadFromData((const uchar*) pictureFrame->picture().data(), pictureFrame->picture().size());
            } else{
                chap.picture = m_cover;
            }

            m_chapters << chap;
        }

        emit chaptersChanged(&m_chapters);

        //        fl = tag->frameListMap()["CHAP"];

        //        for (ID3v2::FrameList::ConstIterator it= fl.begin();it != fl.end() ; it++){
        //            ID3v2::ChapterFrame* cf = static_cast< ID3v2::ChapterFrame*> (*it);

        //            cf->findByElementID()

        //        }

    }
}

void MediaMetaDataExtractor::inspectM4A(const QString &url)
{
    MP4::File mf(url.toLocal8Bit());
    if(!mf.isValid() || !mf.hasMP4Tag())
        return;

    MP4::Tag* tag = mf.tag();
    qDebug() << tag->toString().toCString();
    MP4::ItemListMap itemsListMap = tag->itemListMap();
    MP4::Item coverItem = itemsListMap["covr"];
    MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
    if (!coverArtList.isEmpty()) {
        MP4::CoverArt coverArt = coverArtList.front();
        m_cover.loadFromData((const uchar *) coverArt.data().data(),coverArt.data().size());
    }


    //    for (auto it =itemsListMap.begin(); it !=itemsListMap.end(); it++){
    //        qDebug()<< it->first.toCString(true) << ": " <<  it->second.toStringList().toString('|').toCString(true);
    //    }
}

void MediaMetaDataExtractor::inspectOGG(const QString &url)
{
    Ogg::Vorbis::File of (url.toLocal8Bit());

    if(!of.isValid())
        return;

    parseXiphComments(of.tag());
}

void MediaMetaDataExtractor::inspectOPUS(const QString &url)
{
    Ogg::Opus::File of (url.toLocal8Bit());

    if(!of.isValid())
        return;

    parseXiphComments(of.tag());
}

void MediaMetaDataExtractor::parseXiphComments(TagLib::Ogg::XiphComment *tag)
{
    if(!tag)
        return;

    List<FLAC::Picture *> pl = tag->pictureList();

    if (!pl.isEmpty()){
        FLAC::Picture* pic = pl.front();
        m_cover.loadFromData((const uchar *) pic->data().data(), pic->data().size());
    }

    if(tag->contains("CHAPTER001")){
        qDebug() << "Found chapter";

        for (int i=1; i<1000; i++){
            QString id = QString("CHAPTER%1").arg(QString::number(i),3,'0');
            QString name = id+"NAME";
            QString url = id+"URL";
            if(!tag->contains(id.toLocal8Bit().data()))
                break;

            PodcastChapter chap;

            if(tag->contains(name.toLocal8Bit().data()))
                chap.name = QString::fromUtf8(tag->fieldListMap()[name.toLocal8Bit().data()].front().toCString(true));

            if(tag->contains(url.toLocal8Bit().data()))
                chap.url = QString::fromUtf8(tag->fieldListMap()[url.toLocal8Bit().data()].front().toCString(true));


            QString posString = QString::fromUtf8(tag->fieldListMap()[id.toLocal8Bit().data()].front().toCString(true));

            int hours = 0;
            int minutes = 0;
            float seconds = 0;

            QStringList parts = posString.split(':');
            if (parts.length() == 3){
                hours = parts[0].toInt();
                minutes = parts[1].toInt();
                seconds = parts[2].toFloat();
            }
            else{
                qDebug() << "Invalid Chapter definition: " << posString;
            }

            chap.start = seconds*1000+minutes*60*1000+ hours*60*60*1000;
            chap.picture = m_cover;

            m_chapters << chap;

            qDebug() << "Adding chapter "<< chap.name << " at " << chap.start;

        }

       emit chaptersChanged(&m_chapters);

    }
}

QImage MediaMetaDataExtractor::getCover()
{
    return m_cover;
}

QList<PodcastChapter> *MediaMetaDataExtractor::chapters()
{
    return &m_chapters;
}


QImage CoverImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    qDebug() << "Image " << id << "requested";

    if (id == "cover"){

    QImage cover = m_mmde.getCover();
    size->setHeight(cover.size().height());
    size->setWidth(cover.size().width());
    return cover;
    } else if (id.startsWith("chapter_")){
        QStringList parts = id.split("_");

        if (parts.length() != 2){
            qWarning() << "invalid chapter index"  << id;

            return QImage();
        }

        bool ok;
        int index = parts[1].toInt(&ok);

        if (ok && index >= 0){
            QImage pic = m_mmde.chapters()->at(index).picture;

            if (pic.isNull())
                pic = m_mmde.getCover();

            size->setHeight(pic.size().height());
            size->setWidth(pic.size().width());
            return pic;
        }else {
            qWarning() << "invalid chapter index"  << id;

            return QImage();
        }

    }
    else {
        return QImage();
    }
}

PodcastChapterModel::PodcastChapterModel(QObject *parent):
    QAbstractListModel(parent)
{
    m_roleNames[TitleRole] = "title";
    m_roleNames[DescriptionRole] = "description";
    m_roleNames[StartRole] = "start";
    m_roleNames[URLRole] = "url";
}

int PodcastChapterModel::rowCount(const QModelIndex &parent) const
{
    if(m_chapters){
        //qDebug() << "Chapter count" <<m_chapters->count();
        return m_chapters->count();
    }

        return 0;
}

QVariant PodcastChapterModel::data(const QModelIndex &index, int role) const
{
    if (!m_chapters || index.row() < 0 || index.row() > m_chapters->count())
        return QVariant();

    switch(role){
    case TitleRole:
        return QVariant((*m_chapters)[index.row()].name);

    case DescriptionRole:
        return QVariant((*m_chapters)[index.row()].description);

    case StartRole:
         return QVariant((*m_chapters)[index.row()].start);

    case URLRole:
         return QVariant((*m_chapters)[index.row()].url);
    default:
        return QVariant();

    }
}

QHash<int, QByteArray> PodcastChapterModel::roleNames() const
{
    return m_roleNames;
}


void PodcastChapterModel::setChapters(QList<PodcastChapter> *chapters)
{
    beginResetModel();
    m_chapters = chapters;
    if(chapters)
        qDebug() << "We found " << m_chapters->size() << "chapters.";
    endResetModel();

    emit countChanged(m_chapters->count());
}

void PodcastChapterModel::onChaptersChanged(QList<PodcastChapter> *chapters)
{
    qDebug() << "setting Chapters in model";
    setChapters(chapters);
}


int PodcastChapterModel::currentChapter() const
{
    return m_currentChapter;
}

int PodcastChapterModel::position() const
{
    return m_position;
}

void PodcastChapterModel::setPosition(int newPosition)
{
    m_position = newPosition;
    updateCurrentChapter();

}

void PodcastChapterModel::updateCurrentChapter()
{
    m_currentChapter = -1;

    for (int i=0; i < m_chapters->length(); i++){
        if (m_chapters->at(i).start >= m_position){

            if (i != m_currentChapter){
                m_currentChapter = i;
                emit currentChapterChanged();
            }

            qDebug() << m_position << "corresponds to chapter" << i;

            break;
        }
    }

}
