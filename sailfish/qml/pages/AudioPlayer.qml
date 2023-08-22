/**
 * This file is part of Podcatcher for N9.
 * Author: Johan Paul (johan.paul@gmail.com)
 *
 * Podcatcher for N9 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Podcatcher for N9 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Podcatcher for N9.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.6
import QtMultimedia 5.6

import Sailfish.Silica 1.0
import  org.nemomobile.configuration 1.0

//import org.nemomobile.mpris 1.0
import Amber.Mpris 1.0

Page {
    id: playerPage

    signal playStream(string url, string title)
    signal playFile(string fileName)
    signal stopStream(string url)

    property string title : mediaMetaDataExtractor.title
    property bool playing : audioPlayer.playbackState === Audio.PlayingState
    property bool paused: audioPlayer.playbackState === Audio.PausedState
    property bool active: paused || playing
    property bool waitForSeeking: false


    ConfigurationValue{
        id: lastPosition
        key: ""
        defaultValue: 0
    }

    function durationText(curPos) {
        var curPosSecs = Math.floor((parseInt(curPos) / 1000));

        var curPlayTimeSecs = (curPosSecs % 60);
        var curPlayTimeMins = Math.floor((curPosSecs / 60));
        var curPlayTimeHours = Math.floor((curPosSecs / 3600));

        if (curPlayTimeHours == 0) {
            curPlayTimeHours = ""
        } else if (curPlayTimeHours < 10) {
            curPlayTimeHours = "0" + curPlayTimeHours.toString();
        }

        if (curPlayTimeHours > 0) {
            curPlayTimeMins = curPlayTimeMins % 60;
        }

        if (curPlayTimeSecs < 10) {
            curPlayTimeSecs = "0" + curPlayTimeSecs;
        }

        if (curPlayTimeMins < 10) {
            curPlayTimeMins = "0" + curPlayTimeMins;
        }

        if (curPlayTimeHours.toString().length > 0) {
            curPlayTimeHours = curPlayTimeHours + " : ";
        }

        var playtimeString = curPlayTimeHours.toString() + curPlayTimeMins.toString() + " : " + curPlayTimeSecs.toString();

        return playtimeString;
    }

    onStatusChanged:{
        if(status ===PageStatus.Activating)
            chapterPanel.hide(true);
    }

    PageHeader{
        id: header
        title: qsTr("Player")
    }


    Column {
        id: buttonGroup
        spacing: Theme.paddingMedium
        width: parent.width
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: Theme.paddingMedium
        anchors.bottomMargin: Theme.paddingLarge

        Image {
            id: cover

            width: parent.width/*+2*Theme.paddingMedium*/
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: Page.isLandscape ? false : true
            cache: false
        }


        Label {
            id: titleLabel
            text: title
            width: parent.width -2*Theme.horizontalPageMargin
            height: Text.paintedHeight
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode:Text.WrapAtWordBoundaryOrAnywhere
            //elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeSmall
        }


        Slider{

            property int position: audioPlayer.position

            id: playbackPosition
            width: parent.width
            //height: Theme.iconSizeMedium
            minimumValue: 0
            maximumValue: audioPlayer.duration?audioPlayer.duration:1
            value: audioPlayer.position
            //enabled: audioPlayer.seekable
            //handleVisible: true


            onPositionChanged: {
                if (!pressed) {
                    value = position
                }
            }
            onReleased: {
                if (audioPlayer.seekable) {
                    audioPlayer.seek(value);
                }
            }
        }

        Row {
            id: btnRow
            spacing: Theme.paddingSmall
            anchors.horizontalCenter: parent.horizontalCenter


            IconButton {
                id: rewBtn
                icon.source: "image://theme/icon-m-left?" + (pressed
                                                             ? Theme.highlightColor
                                                             : Theme.primaryColor)
                onClicked: {
                    console.log("Setting audio position to " + audioPlayer.position - 10000 + "s")
                    audioPlayer.seek(audioPlayer.position - 10000);
                }

                enabled: audioPlayer.playbackState == Audio.PlayingState ||
                         audioPlayer.playbackState == Audio.PausedState
            }


            IconButton {
                id: playBtn
                icon.source: (audioPlayer.playbackState !== Audio.PlayingState)?
                                 ("image://theme/icon-m-play?" + (pressed
                                                                  ? Theme.highlightColor
                                                                  : Theme.primaryColor)):
                                 ("image://theme/icon-m-pause?" + (pressed
                                                                   ? Theme.highlightColor
                                                                   : Theme.primaryColor))

                onClicked: {
                    if (audioPlayer.playbackState !== Audio.PlayingState)
                        audioPlayer.play();
                    else
                        audioPlayer.pause();
                }
            }




            IconButton {
                id:ffBtn
                icon.source: "image://theme/icon-m-right?" + (pressed
                                                              ? Theme.highlightColor
                                                              : Theme.primaryColor)

                enabled: audioPlayer.playbackState === Audio.PlayingState ||
                         audioPlayer.playbackState === Audio.PausedState


                onClicked: {
                    console.log("Setting audio position to " + audioPlayer.position + 10000 + "s")
                    //audioPlayer.position = audioPlayer.position + 10000
                    audioPlayer.seek(audioPlayer.position + 10000);
                }
            }

            IconButton{
                id: chapBtn

                icon.source: "image://theme/icon-m-events?" + (pressed
                                                               ? Theme.highlightColor
                                                               : Theme.primaryColor)

                onClicked: {
                    chapterPanel.open?chapterPanel.hide():chapterPanel.show();
                    console.log(channelsModel.count)
                }

                visible: chapterModel.count > 0;
            }

//            IconButton{
//                id: speedBtn
//                icon.source: "image://theme/icon-m-timer?" + (pressed
//                                                               ? Theme.highlightColor
//                                                               : Theme.primaryColor)
//                onClicked: {console.log(btnRow.implicitHeight);
//                    console.log(btnRow.height);
//                    console.log(buttonGroup.implicitHeight);
//                     console.log(buttonGroup.height);
//                    speedMenu.open(btnRow);}
//            }
        }
//        ContextMenu{
//            id:speedMenu
//            MenuItem{
//                text: "normal (1)"
//            }
//        }

        Label {
            id: durationLabel
            text: durationText(audioPlayer.position) + "/" + durationText(audioPlayer.duration);
            font.pixelSize: Theme.fontSizeTiny
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }


    DockedPanel{
        id:chapterPanel

        //        dock: Dock.Right

        //        width: parent.width/2
        //        height: cover.height
        //        anchors.top: cover.top

        dock: Dock.Top
        width: parent.width
        height: cover.height + header.height + Theme.paddingMedium

        open: false
        clip: true

        Rectangle{
            anchors.fill: parent

            //color: Theme.highlightBackgroundColor
            color: Theme.overlayBackgroundColor

        }


        SilicaListView{
            id: chapterList
            model: chapterModel

            anchors.fill: parent

            header: Label{
                text: qsTr("Chapters")
                font.family: Theme.fontFamilyHeading
                font.pixelSize: Theme.fontSizeLarge
                anchors.leftMargin: Theme.horizontalPageMargin
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.left: parent.left
                anchors.right: parent.right
            }


            delegate: ListItem{
                id: chapterItem
                width: parent.width
                contentHeight: chapTitleLabel.height +Theme.paddingMedium
                Label{
                    id: chapTitleLabel
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    text: title
                    color: chapterItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    height: Text.paintedHeight
                    wrapMode: Text.WordWrap
                }

                onClicked: audioPlayer.seek(start)
            }

        }
    }


    onPlayStream: {
        chapterPanel.hide(true);
        audioPlayer.pause();
        audioPlayer.source = url;
        playerPage.title = title;
        audioPlayer.play();
    }


    onPlayFile: {

        chapterPanel.hide(true);

        var hash = Qt.md5(fileName);
        lastPosition.key = "/apps/ControlPanel/Podcatcher/position/"+hash;
        console.log("Last position key: "+lastPosition.key)


        audioPlayer.source = fileName;

        if(lastPosition.value >0 && !audioPlayer.seekable){
            console.log("Seeking not yet possible!");
            waitForSeeking = true;
            audioPlayer.muted = true;
            audioPlayer.play();
        }else{
            audioPlayer.seek(lastPosition.value);
            audioPlayer.play();
        }
    }

    Audio {
        id: audioPlayer

        onPlaying: {
            console.log("Ok, we started playing...");
            console.log("Current position: "+position)
            console.log("Total duration" + duration)
            mpris.playbackStatus = Mpris.Playing
        }

        onPaused: {
            if(!waitForSeeking && seekable){
                lastPosition.value = position
                console.log("remembering position: " + position)
            }
            mpris.playbackStatus = Mpris.Paused
        }

        onStopped: {
            mpris.playbackStatus = Mpris.Stopped

            /*
            We cannot save the position since it is already cleared
            */
        }

        onSourceChanged: {
            mediaMetaDataExtractor.inspect(source);
            console.log("Last position key: "+lastPosition.key);
            console.log("Last position: "+lastPosition.value);
            cover.source = '';
            cover.source = 'image://coverArt/cover';


            /*
            var infos = mpris.metadata;
            infos[Mpris.metadataToString(Mpris.Artist)] = [mediaMetaDataExtractor.artist]
            infos[Mpris.metadataToString(Mpris.Title)] = mediaMetaDataExtractor.title
            mpris.metadata = infos;
            */

//            var artist = "";

//            if (audioPlayer.metaData.albumArtist !== undefined )
//                artist = audioPlayer.metaData.albumArtist;
//            else if (audioPlayer.metaData.contributingArtist !== undefined)
//                artist = audioPlayer.metaData.contributingArtist;
//            else if (audioPlayer.metaData.composer !== undefined)
//                artist = audioPlayer.metaData.composer;

//            console.log("AlbumArtist: "+audioPlayer.metaData.albumArtist);
//            console.log("ContributingArtist: "+audioPlayer.metaData.contributingArtist);
//            console.log("Composer: "+audioPlayer.metaData.composer);

            mpris.metaData.title =  mediaMetaDataExtractor.title;
            mpris.metaData.contributingArtist =mediaMetaDataExtractor.artist;
            mpris.metaData.albumTitle = mediaMetaDataExtractor.podcast;
            mpris.canSeek = audioPlayer.seekable;
        }

        onStatusChanged: {
            var statusNames= ["NoMedia", "Loading", "Loaded", "Buffering", "Stalled", "Buffered", "EndOfMedia", "InvalidMedia", "UnknownStatus"];
            console.log("Status changed: "+ statusNames[status])
            console.log("Playback position: "+ position);
        }


        onSeekableChanged: {
            if (waitForSeeking){
            console.log("Stop muted playback, since we now know the durations and can seek.")
            audioPlayer.pause();
            audioPlayer.muted=false;
            waitForSeeking = false;

            audioPlayer.seek(lastPosition.value);
            audioPlayer.play();
            }
        }
    }

    MprisPlayer {
        id: mpris
        serviceName: "podcatcher"

        identity: "Podcatcher"
        supportedUriSchemes: ["file"]
        supportedMimeTypes: ["audio/x-wav", "audio/x-vorbis+ogg", "audio/mpeg", "audio/mp4a-latm", "audio/x-aiff"]
        // Mpris2 Player Interface
        canControl: true

        canGoNext: audioPlayer.position !== audioPlayer.duration
        canGoPrevious: audioPlayer.position !== 0
        canPause: true
        canPlay: true

        canSeek: false
        hasTrackList: false
        playbackStatus: Mpris.Paused
        loopStatus: Mpris.LoopNone//Mpris.None
        shuffle: false
        volume: 1.0
        onPauseRequested: {
            console.log("MPRIS: Pause Requested");
            audioPlayer.pause();
        }

        onPlayRequested: {
            console.log("MPRIS: Play Requested");
            audioPlayer.play()
        }

        onPlayPauseRequested:{ console.log("MPRIS: PlayPause Requested");
            if (audioPlayer.playbackState===Audio.PlayingState)
                audioPlayer.pause();
            else
                audioPlayer.play();
        }
        onStopRequested: {
            console.log("MPRIS: Stop Requested")
            audioPlayer.stop();
        }
        onNextRequested: {
            audioPlayer.seek(audioPlayer.position + 10000);
        }

        onPreviousRequested: {
            audioPlayer.seek(audioPlayer.position - 10000);
        }

        //metadata handling
        function updateMetaData(){
            /*
            var infos = mpris.metadata;

            infos[Mpris.metadataToString(Mpris.Artist)] = [mediaMetaDataExtractor.artist]
            infos[Mpris.metadataToString(Mpris.Title)] = mediaMetaDataExtractor.title
            mpris.metadata = infos;
            */

            Mpris.metaData.albumArtist = mediaMetaDataExtractor.artist;
            Mpris.metaData.title = mediaMetaDataExtractor.title;

        }
    }

}
