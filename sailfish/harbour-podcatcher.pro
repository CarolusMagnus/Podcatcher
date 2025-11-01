# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-podcatcher

DEFINES += PODCATCHER_VERSION=203
QT += sql xml concurrent

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG += sailfishapp

CONFIG += link_pkgconfig
PKGCONFIG += sailfishapp mlite5
#PKGCONFIG +=taglib
#PKGCONFIG += contentaction5
QMAKE_CXXFLAGS += -std=c++17


DEFINES += MAKE_TAGLIB_LIB
DEFINES += TAGLIB_STATIC=1

SOURCES += src/Podcatcher.cpp \
    ../taglib/bindings/c/tag_c.cpp \
    ../taglib/taglib/ape/apefile.cpp \
    ../taglib/taglib/ape/apefooter.cpp \
    ../taglib/taglib/ape/apeitem.cpp \
    ../taglib/taglib/ape/apeproperties.cpp \
    ../taglib/taglib/ape/apetag.cpp \
    ../taglib/taglib/asf/asfattribute.cpp \
    ../taglib/taglib/asf/asffile.cpp \
    ../taglib/taglib/asf/asfpicture.cpp \
    ../taglib/taglib/asf/asfproperties.cpp \
    ../taglib/taglib/asf/asftag.cpp \
    ../taglib/taglib/audioproperties.cpp \
    ../taglib/taglib/dsdiff/dsdiffdiintag.cpp \
    ../taglib/taglib/dsdiff/dsdifffile.cpp \
    ../taglib/taglib/dsdiff/dsdiffproperties.cpp \
    ../taglib/taglib/dsf/dsffile.cpp \
    ../taglib/taglib/dsf/dsfproperties.cpp \
    ../taglib/taglib/ebml/ebmlelement.cpp \
    ../taglib/taglib/ebml/ebmlfile.cpp \
    ../taglib/taglib/ebml/matroska/ebmlmatroskaaudio.cpp \
    ../taglib/taglib/ebml/matroska/ebmlmatroskafile.cpp \
    ../taglib/taglib/fileref.cpp \
    ../taglib/taglib/flac/flacfile.cpp \
    ../taglib/taglib/flac/flacmetadatablock.cpp \
    ../taglib/taglib/flac/flacpicture.cpp \
    ../taglib/taglib/flac/flacproperties.cpp \
    ../taglib/taglib/flac/flacunknownmetadatablock.cpp \
    ../taglib/taglib/it/itfile.cpp \
    ../taglib/taglib/it/itproperties.cpp \
    ../taglib/taglib/mod/modfile.cpp \
    ../taglib/taglib/mod/modfilebase.cpp \
    ../taglib/taglib/mod/modproperties.cpp \
    ../taglib/taglib/mod/modtag.cpp \
    ../taglib/taglib/mp4/mp4atom.cpp \
    ../taglib/taglib/mp4/mp4coverart.cpp \
    ../taglib/taglib/mp4/mp4file.cpp \
    ../taglib/taglib/mp4/mp4item.cpp \
    ../taglib/taglib/mp4/mp4properties.cpp \
    ../taglib/taglib/mp4/mp4tag.cpp \
    ../taglib/taglib/mpc/mpcfile.cpp \
    ../taglib/taglib/mpc/mpcproperties.cpp \
    ../taglib/taglib/mpeg/id3v1/id3v1genres.cpp \
    ../taglib/taglib/mpeg/id3v1/id3v1tag.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/attachedpictureframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/chapterframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/commentsframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/eventtimingcodesframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/ownershipframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/podcastframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/popularimeterframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/privateframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/relativevolumeframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/synchronizedlyricsframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/tableofcontentsframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/textidentificationframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/unknownframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp \
    ../taglib/taglib/mpeg/id3v2/frames/urllinkframe.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2extendedheader.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2footer.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2frame.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2framefactory.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2header.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2synchdata.cpp \
    ../taglib/taglib/mpeg/id3v2/id3v2tag.cpp \
    ../taglib/taglib/mpeg/mpegfile.cpp \
    ../taglib/taglib/mpeg/mpegheader.cpp \
    ../taglib/taglib/mpeg/mpegproperties.cpp \
    ../taglib/taglib/mpeg/xingheader.cpp \
    ../taglib/taglib/ogg/flac/oggflacfile.cpp \
    ../taglib/taglib/ogg/oggfile.cpp \
    ../taglib/taglib/ogg/oggpage.cpp \
    ../taglib/taglib/ogg/oggpageheader.cpp \
    ../taglib/taglib/ogg/opus/opusfile.cpp \
    ../taglib/taglib/ogg/opus/opusproperties.cpp \
    ../taglib/taglib/ogg/speex/speexfile.cpp \
    ../taglib/taglib/ogg/speex/speexproperties.cpp \
    ../taglib/taglib/ogg/vorbis/vorbisfile.cpp \
    ../taglib/taglib/ogg/vorbis/vorbisproperties.cpp \
    ../taglib/taglib/ogg/xiphcomment.cpp \
    ../taglib/taglib/riff/aiff/aifffile.cpp \
    ../taglib/taglib/riff/aiff/aiffproperties.cpp \
    ../taglib/taglib/riff/rifffile.cpp \
    ../taglib/taglib/riff/wav/infotag.cpp \
    ../taglib/taglib/riff/wav/wavfile.cpp \
    ../taglib/taglib/riff/wav/wavproperties.cpp \
    ../taglib/taglib/s3m/s3mfile.cpp \
    ../taglib/taglib/s3m/s3mproperties.cpp \
    ../taglib/taglib/tag.cpp \
    ../taglib/taglib/tagunion.cpp \
    ../taglib/taglib/tagutils.cpp \
    ../taglib/taglib/toolkit/taglib.cpp \
    ../taglib/taglib/toolkit/tbytevector.cpp \
    ../taglib/taglib/toolkit/tbytevectorlist.cpp \
    ../taglib/taglib/toolkit/tbytevectorstream.cpp \
    ../taglib/taglib/toolkit/tdebug.cpp \
    ../taglib/taglib/toolkit/tdebuglistener.cpp \
    ../taglib/taglib/toolkit/tfile.cpp \
    ../taglib/taglib/toolkit/tfilestream.cpp \
    ../taglib/taglib/toolkit/tiostream.cpp \
    ../taglib/taglib/toolkit/tpicture.cpp \
    ../taglib/taglib/toolkit/tpicturemap.cpp \
    ../taglib/taglib/toolkit/tpropertymap.cpp \
    ../taglib/taglib/toolkit/trefcounter.cpp \
    ../taglib/taglib/toolkit/tstring.cpp \
    ../taglib/taglib/toolkit/tstringhandler.cpp \
    ../taglib/taglib/toolkit/tstringlist.cpp \
    ../taglib/taglib/toolkit/tzlib.cpp \
    ../taglib/taglib/trueaudio/trueaudiofile.cpp \
    ../taglib/taglib/trueaudio/trueaudioproperties.cpp \
    ../taglib/taglib/wavpack/wavpackfile.cpp \
    ../taglib/taglib/wavpack/wavpackproperties.cpp \
    ../taglib/taglib/xm/xmfile.cpp \
    ../taglib/taglib/xm/xmproperties.cpp \
    src/dbhelper.cpp \
    src/mediametadataextractor.cpp \
    src/podcastchannel.cpp \
    src/podcastchannelsmodel.cpp \
    src/podcastepisode.cpp \
    src/podcastepisodesmodel.cpp \
    src/podcastepisodesmodelfactory.cpp \
    src/podcastmanager.cpp \
    #src/podcastplaylist.cpp \
    src/podcastrssparser.cpp \
    src/podcastrssparser2.cpp \
    src/podcastsqlmanager.cpp \
    src/podcatcherui.cpp \


OTHER_FILES += qml/Podcatcher.qml \
    qml/cover/CoverPage.qml
    translations/*.ts

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

TRANSLATIONS += translations/harbour-podcatcher-de.ts \
                translations/harbour-podcatcher-es.ts \
                translations/harbour-podcatcher-fr.ts \
                translations/harbour-podcatcher-it.ts \
                translations/harbour-podcatcher-ru.ts \
                translations/harbour-podcatcher-sv.ts \
                translations/harbour-podcatcher-zh_CN.ts


DISTFILES += \
    qml/Utils.js \
    qml/EmptyPage.qml \
    qml/PodcatcherInfoBanner.qml \
    qml/pages/ChannelDetailsPage.qml \
    qml/pages/EpisodesSortDialog.qml \
    qml/pages/MainPage.qml \
    qml/pages/BrowsePodcasts.qml \
    qml/pages/PodcastEpisodes.qml \
    qml/pages/PodcastEpisodesChannelInfo.qml \
    qml/pages/PodcastEpisodesList.qml \
    qml/pages/PodcastChannelLogo.qml \
    qml/pages/PodcastDownloadingProgress.qml \
    qml/pages/EpisodeDescriptionPage.qml \
    qml/pages/SearchPodcasts.qml \
    qml/pages/ImportFromGPodder.qml \
    qml/pages/About.qml \
    qml/pages/Settings.qml \
    qml/pages/ArchivePage.qml \
    qml/pages/URLEditDialog.qml \
    qml/pages/JSONListModel.qml \
    qml/pages/jsonpath.js \
    harbour-podcatcher.desktop \
    rpm/harbour-podcatcher.spec \
    rpm/harbour-podcatcher.yaml \
    qml/pages/InfoBanner.qml \
    rpm/Podcatcher.changes

HEADERS += \
    ../taglib/3rdparty/utf8-cpp/checked.h \
    ../taglib/3rdparty/utf8-cpp/core.h \
    ../taglib/bindings/c/tag_c.h \
    ../taglib/taglib/ape/apefile.h \
    ../taglib/taglib/ape/apefooter.h \
    ../taglib/taglib/ape/apeitem.h \
    ../taglib/taglib/ape/apeproperties.h \
    ../taglib/taglib/ape/apetag.h \
    ../taglib/taglib/asf/asfattribute.h \
    ../taglib/taglib/asf/asffile.h \
    ../taglib/taglib/asf/asfpicture.h \
    ../taglib/taglib/asf/asfproperties.h \
    ../taglib/taglib/asf/asftag.h \
    ../taglib/taglib/asf/asfutils.h \
    ../taglib/taglib/audioproperties.h \
    ../taglib/taglib/dsdiff/dsdiffdiintag.h \
    ../taglib/taglib/dsdiff/dsdifffile.h \
    ../taglib/taglib/dsdiff/dsdiffproperties.h \
    ../taglib/taglib/dsf/dsffile.h \
    ../taglib/taglib/dsf/dsfproperties.h \
    ../taglib/taglib/ebml/ebmlconstants.h \
    ../taglib/taglib/ebml/ebmlelement.h \
    ../taglib/taglib/ebml/ebmlfile.h \
    ../taglib/taglib/ebml/matroska/ebmlmatroskaaudio.h \
    ../taglib/taglib/ebml/matroska/ebmlmatroskaconstants.h \
    ../taglib/taglib/ebml/matroska/ebmlmatroskafile.h \
    ../taglib/taglib/fileref.h \
    ../taglib/taglib/flac/flacfile.h \
    ../taglib/taglib/flac/flacmetadatablock.h \
    ../taglib/taglib/flac/flacpicture.h \
    ../taglib/taglib/flac/flacproperties.h \
    ../taglib/taglib/flac/flacunknownmetadatablock.h \
    ../taglib/taglib/it/itfile.h \
    ../taglib/taglib/it/itproperties.h \
    ../taglib/taglib/mod/modfile.h \
    ../taglib/taglib/mod/modfilebase.h \
    ../taglib/taglib/mod/modfileprivate.h \
    ../taglib/taglib/mod/modproperties.h \
    ../taglib/taglib/mod/modtag.h \
    ../taglib/taglib/mp4/mp4atom.h \
    ../taglib/taglib/mp4/mp4coverart.h \
    ../taglib/taglib/mp4/mp4file.h \
    ../taglib/taglib/mp4/mp4item.h \
    ../taglib/taglib/mp4/mp4properties.h \
    ../taglib/taglib/mp4/mp4tag.h \
    ../taglib/taglib/mpc/mpcfile.h \
    ../taglib/taglib/mpc/mpcproperties.h \
    ../taglib/taglib/mpeg/id3v1/id3v1genres.h \
    ../taglib/taglib/mpeg/id3v1/id3v1tag.h \
    ../taglib/taglib/mpeg/id3v2/frames/attachedpictureframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/chapterframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/commentsframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/eventtimingcodesframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/ownershipframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/podcastframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/popularimeterframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/privateframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/relativevolumeframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/synchronizedlyricsframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/tableofcontentsframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/textidentificationframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/unknownframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.h \
    ../taglib/taglib/mpeg/id3v2/frames/urllinkframe.h \
    ../taglib/taglib/mpeg/id3v2/id3v2extendedheader.h \
    ../taglib/taglib/mpeg/id3v2/id3v2footer.h \
    ../taglib/taglib/mpeg/id3v2/id3v2frame.h \
    ../taglib/taglib/mpeg/id3v2/id3v2framefactory.h \
    ../taglib/taglib/mpeg/id3v2/id3v2header.h \
    ../taglib/taglib/mpeg/id3v2/id3v2synchdata.h \
    ../taglib/taglib/mpeg/id3v2/id3v2tag.h \
    ../taglib/taglib/mpeg/mpegfile.h \
    ../taglib/taglib/mpeg/mpegheader.h \
    ../taglib/taglib/mpeg/mpegproperties.h \
    ../taglib/taglib/mpeg/mpegutils.h \
    ../taglib/taglib/mpeg/xingheader.h \
    ../taglib/taglib/ogg/flac/oggflacfile.h \
    ../taglib/taglib/ogg/oggfile.h \
    ../taglib/taglib/ogg/oggpage.h \
    ../taglib/taglib/ogg/oggpageheader.h \
    ../taglib/taglib/ogg/opus/opusfile.h \
    ../taglib/taglib/ogg/opus/opusproperties.h \
    ../taglib/taglib/ogg/speex/speexfile.h \
    ../taglib/taglib/ogg/speex/speexproperties.h \
    ../taglib/taglib/ogg/vorbis/vorbisfile.h \
    ../taglib/taglib/ogg/vorbis/vorbisproperties.h \
    ../taglib/taglib/ogg/xiphcomment.h \
    ../taglib/taglib/riff/aiff/aifffile.h \
    ../taglib/taglib/riff/aiff/aiffproperties.h \
    ../taglib/taglib/riff/rifffile.h \
    ../taglib/taglib/riff/riffutils.h \
    ../taglib/taglib/riff/wav/infotag.h \
    ../taglib/taglib/riff/wav/wavfile.h \
    ../taglib/taglib/riff/wav/wavproperties.h \
    ../taglib/taglib/s3m/s3mfile.h \
    ../taglib/taglib/s3m/s3mproperties.h \
    ../taglib/taglib/tag.h \
    ../taglib/taglib/taglib_export.h \
    ../taglib/taglib/tagunion.h \
    ../taglib/taglib/tagutils.h \
    ../taglib/taglib/toolkit/taglib.h \
    ../taglib/taglib/toolkit/tbytevector.h \
    ../taglib/taglib/toolkit/tbytevectorlist.h \
    ../taglib/taglib/toolkit/tbytevectorstream.h \
    ../taglib/taglib/toolkit/tdebug.h \
    ../taglib/taglib/toolkit/tdebuglistener.h \
    ../taglib/taglib/toolkit/tfile.h \
    ../taglib/taglib/toolkit/tfilestream.h \
    ../taglib/taglib/toolkit/tiostream.h \
    ../taglib/taglib/toolkit/tlist.h \
    ../taglib/taglib/toolkit/tmap.h \
    ../taglib/taglib/toolkit/tpicture.h \
    ../taglib/taglib/toolkit/tpicturemap.h \
    ../taglib/taglib/toolkit/tpropertymap.h \
    ../taglib/taglib/toolkit/trefcounter.h \
    ../taglib/taglib/toolkit/tsmartptr.h \
    ../taglib/taglib/toolkit/tstring.h \
    ../taglib/taglib/toolkit/tstringhandler.h \
    ../taglib/taglib/toolkit/tstringlist.h \
    ../taglib/taglib/toolkit/tutils.h \
    ../taglib/taglib/toolkit/tzlib.h \
    ../taglib/taglib/trueaudio/trueaudiofile.h \
    ../taglib/taglib/trueaudio/trueaudioproperties.h \
    ../taglib/taglib/wavpack/wavpackfile.h \
    ../taglib/taglib/wavpack/wavpackproperties.h \
    ../taglib/taglib/xm/xmfile.h \
    ../taglib/taglib/xm/xmproperties.h \
    ../taglib/tests/utils.h \
    src/dbhelper.h \
    src/mediametadataextractor.h \
    src/podcastchannel.h \
    src/podcastchannelsmodel.h \
    src/podcastepisode.h \
    src/podcastepisodesmodel.h \
    src/podcastepisodesmodelfactory.h \
    src/podcastglobals.h \
    src/podcastmanager.h \
    #src/podcastplaylist.h \
    src/podcastrssparser.h \
    src/podcastrssparser2.h \
    src/podcastsqlmanager.h \
    src/podcasttester.h \
    src/podcatcherui.h \


RESOURCES += \
    res.qrc

INCLUDEPATH += ../taglib/taglib \
../taglib/taglib/toolkit \
../taglib/taglib/aiff \
../taglib/taglib/ape \
../taglib/taglib/asf \
../taglib/taglib/dsf \
../taglib/taglib/dsdiff \
../taglib/taglib/ebml \
../taglib/taglib/ebml/matroska \
../taglib/taglib/flac \
../taglib/taglib/it \
../taglib/taglib/mpc \
../taglib/taglib/mpeg \
../taglib/taglib/mpeg/id3v1 \
../taglib/taglib/mpeg/id3v2 \
../taglib/taglib/mpeg/id3v2/frames \
../taglib/taglib/mp4 \
../taglib/taglib/mod \
../taglib/taglib/ogg \
../taglib/taglib/ogg/flac \
../taglib/taglib/ogg/opus \
../taglib/taglib/ogg/vorbis \
../taglib/taglib/ogg/speex \
../taglib/taglib/riff \
../taglib/taglib/riff/aiff \
../taglib/taglib/riff/wav \
../taglib/taglib/s3m \
../taglib/taglib/trueaudio\
../taglib/taglib/xm \
../taglib/taglib/wavpack \
../taglib/3rdparty
