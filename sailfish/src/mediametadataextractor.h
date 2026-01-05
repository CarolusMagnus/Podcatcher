#ifndef MEDIAMETADATAEXTRACTOR_H
#define MEDIAMETADATAEXTRACTOR_H

#include <QObject>
#include <qquickimageprovider.h>
#include <QAbstractListModel>
#include <QImage>

#include "podcastepisode.h"

#include <xiphcomment.h>

class MediaMetaDataExtractor;

class CoverImageProvider: public QQuickImageProvider{
public:
    CoverImageProvider(MediaMetaDataExtractor& mmde):
        QQuickImageProvider(QQuickImageProvider::Image),
        m_mmde(mmde)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    MediaMetaDataExtractor& m_mmde;
};


class PodcastChapterModel: public QAbstractListModel{
    Q_OBJECT

    int m_currentChapter;

    int m_position;

public:

    PodcastChapterModel(QObject *parent = Q_NULLPTR);

    enum ChapterRoles {
        TitleRole = Qt::UserRole + 1,
        DescriptionRole,
        StartRole,
        URLRole
    };


    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(int currentChapter READ currentChapter NOTIFY currentChapterChanged)
    Q_PROPERTY(int position READ position WRITE setPosition)

    void setChapters(QList<PodcastChapter>* chapters);

    int count() const
    {
        return m_chapters->count();
    }

    int currentChapter() const;

    int position() const;
    void setPosition(int newPosition);

protected:
     QHash<int, QByteArray> m_roleNames;
     QList<PodcastChapter>* m_chapters{};

     void updateCurrentChapter();

public slots:
     void onChaptersChanged(QList<PodcastChapter>* chapters);
signals:
     void countChanged(int count);
     void currentChapterChanged();
};

class MediaMetaDataExtractor : public QObject
{
    Q_OBJECT
    QString m_title;
    QString m_artist;

    QString m_podcast;

    QImage m_cover;

    QList<PodcastChapter> m_chapters;

public:
    explicit MediaMetaDataExtractor(QObject *parent = nullptr);

    Q_INVOKABLE void inspect(QString url);
    Q_PROPERTY(QString artist READ artist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString podcast READ podcast NOTIFY podcastChanged)

    CoverImageProvider imageProvider;
    QImage getCover();
    QList<PodcastChapter>* chapters();

protected:
    void reset();

    void inspectMP3(const QString &url);
    void inspectM4A(const QString &url);
    void inspectOGG(const QString &url);
    void inspectOPUS(const QString &url);

    void parseXiphComments(TagLib::Ogg::XiphComment *tag);

     const QString & artist() const
    {
        return m_artist;
    }


    QString title() const
    {
        return m_title;
    }

    QString podcast() const
    {
        return m_podcast;
    }


signals:

    void titleChanged(QString title);

    void podcastChanged(QString podcast);

    void chaptersChanged(QList<PodcastChapter> * chapters);

    void artistChanged(QString artist);

public slots:

};

#endif // MEDIAMETADATAEXTRACTOR_H
