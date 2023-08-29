/**
 * This file is part of Podcatcher for Sailfish OS.
 * Authors: Johan Paul (johan.paul@gmail.com)
 *          Moritz Carmesin (carolus@carmesinus.de)
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
import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQml.Models 2.3

Item {
    id: podcastsEpisodesList

    signal selectedEpisodeDescription(string desc)

    property int channelId

    function downloadingStatusText(alreadyDownloadedSize, totalDownloadSize) {
        var downloadedStatus = parseInt(alreadyDownloadedSize) / 1024;  // to kB
        var totalDownloadStatus = parseInt(totalDownloadSize) / 1024;   // to kB

        var downloaded_mbs = Math.round(downloadedStatus / 1024);  // 1MB = 1024kB
        var total_mbs = Math.round(totalDownloadStatus / 1024); // 1MB = 1024kB

        var currSize;

        if (downloaded_mbs == 0) {
            currSize = Math.round(downloadedStatus % 1024) + " kB";
        } else {
            currSize = (downloadedStatus / 1024).toFixed(1) + " MB";
        }

        var total;
        var totalSize;

        if (total_mbs == 0) {
            total = Math.round(totalDownloadStatus % 1024);
            if (total > 0 ) {
                totalSize =  total + " kB";
            }
        } else {
            total = Math.round(totalDownloadStatus / 1024);
            if (total > 0 ) {
                totalSize =  total + " MB";
            }
        }

        var download_statusText=qsTr("Downloaded %1 of total %2.");

        download_statusText = download_statusText.arg(currSize);
        download_statusText = download_statusText.arg(totalSize);

        return download_statusText;
    }


    Rectangle {
        id: podcastEpisodesInfoRect

        smooth: true
        color: "transparent"
        width: parent.width
        height:  parent.height

        SilicaListView {
            id: podcastEpisodesList
            anchors.fill: podcastEpisodesInfoRect
            model: episodesModel
            //model: sortedEpisodesModel
            clip: true
            anchors.top:  podcastEpisodesInfoRect.top

            VerticalScrollDecorator{
                flickable: podcastEpisodesList
            }

            delegate: ListItem {
                id: podcastItem
                state: episodeState
                contentHeight: episodeName.height + lastPlayed.height + Theme.paddingSmall + Theme.paddingMedium
                width: parent.width


                onClicked: {
                    episodeDescriptionPage.episodeDescriptionText = description;
                    episodeDescriptionPage.episodePublished = published
                    episodeDescriptionPage.episodeName = title
                    appWindow.pageStack.push(episodeDescriptionPage);
                }

                RemorseItem{
                    id: episodeRemorse
                }

                menu: ContextMenu{
                    visible: (episodeState == "downloaded" || episodeState == "played" || episodeState == "get" || episodeState == "finished")
                    MenuItem {
                        text: qsTr("Delete downloaded podcast")
                        visible: (episodeState == "downloaded" || episodeState == "played" || episodeState == "finished");
                        onClicked: {
                            episodeRemorse.execute(podcastItem,qsTr("Deleting"),
                                                   function(){
                                                       console.log("Emiting deleteDownloaded() "+ channelId + index);
                                                       appWindow.deleteDownloaded(channelId, index);
                                                   });
                        }
                    }
                    MenuItem{
                        text: qsTr("Mark as unplayed")
                        visible: episodeState == "played"
                        onClicked: {
                            appWindow.markAsUnplayed(channelId,index);
                        }
                    }

                    MenuItem{
                        text: qsTr("Mark as finished")
                        visible: !finished
                        onClicked: {
                            appWindow.markAsFinished(channelId,index);
                        }
                    }

                    MenuItem{
                        text: qsTr("Mark as unfinished")
                        visible: finished
                        onClicked: {
                            appWindow.markAsUnFinished(channelId,index);
                        }
                    }

                    MenuItem {
                        text: qsTr("Start streaming the podcast")
                        visible: (episodeState == "get")
                        onClicked: {
                            appWindow.startStreaming(channelId, index);
                        }

                    }

                }


                Rectangle {
                    id: downloadedIndicator
                    width: 10
                    height: parent.height
                    color: Theme.highlightColor
                    anchors.left: parent.left
                    visible: false
                }


                Item{
                    id: downloadProgress
                    anchors.fill: parent
                    visible: cancelButton.visible
                    Rectangle{

                        anchors.fill: parent
                        color: Theme.highlightBackgroundColor
                        opacity: 0.5
                    }
                }

                OpacityRampEffect {
                    sourceItem:downloadProgress
                    direction: OpacityRamp.LeftToRight
                    slope: 1000
                    offset: (alreadyDownloadedSize/totalDownloadSize)
                }


                Label {
                    id: episodeName
                    text: title;
                    color: podcastItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.family: Theme.fontFamilyHeading
                    font.pixelSize: Theme.fontSizeSmall
                    font.bold:  episodeNew
                    font.italic: finished
                    anchors{
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: playButton.left // playlistAddButton.left
                    }

                    width: podcastItem.width - downloadedIndicator.width - playButton.width /*-playlistAddButton.width*/ - Theme.paddingMedium -2* Theme.horizontalPageMargin
                    height: Text.paintedHeight
                    //height: 2.4*Theme.fontSizeSmall
                    truncationMode: TruncationMode.Elide
                    wrapMode: Text.WordWrap
                }

                Label {
                    id: channelPublished
                    anchors{
                        top: episodeName.bottom
                        topMargin: Theme.paddingSmall
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    color: podcastItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    text: published
                    height: Text.paintedHeight
                }

                Label {
                    id: lastPlayed
                    anchors{
                        top: episodeName.bottom
                        topMargin: Theme.paddingSmall
                        /*left: parent.left
                        leftMargin: Theme.horizontalPageMargin*/
                        right: playButton.left
                        rightMargin: Theme.paddingSmall
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    color: podcastItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    text: lastTimePlayed
                    height: Text.paintedHeight
                }

                Label {
                    id: queued
                    anchors{
                        top: episodeName.bottom
                        topMargin: Theme.paddingSmall
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    color: podcastItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    text: qsTr("Queued")
                    height: Text.paintedHeight
                    visible: false
                }

                Label {
                    id: downloadBytesText
                    anchors{
                        top: episodeName.bottom
                        topMargin: Theme.paddingSmall
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    color: podcastItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    text: downloadingStatusText(alreadyDownloadedSize, totalDownloadSize);
                    height: Text.paintedHeight
                    visible: false;
                }



                IconButton {
                    id: downloadButton
                    icon.source: "image://theme/icon-m-cloud-download"
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin

                    anchors.verticalCenter: parent.verticalCenter
                    visible: true

                    onClicked:{
                        if(ui.isConnectedToWiFi()){
                            appWindow.downloadPodcast(channelId, index);  // Channel id = which model to use, index = row in the model.
                        }else{
                            episodeRemorse.execute(podcastItem,qsTr("Downloading over mobile data"),
                                                   function(){
                                                       console.log("Downloading over modible data "+ channelId + index);
                                                      appWindow.downloadPodcast(channelId, index);  // Channel id = which model to use, index = row in the model.
                                                   });
                        }
                    }
                    /* {

                                        }*/
                }


                IconButton {
                    id: queueButton
                    icon.source: "image://theme/icon-m-remove"
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    visible: false

                    onClicked: {
                        console.log("Cancel queue of: " + channelId + " index: "+index);
                        appWindow.cancelQueue(channelId, index);
                    }
                }


                IconButton {
                    id: cancelButton
                    icon.source: "image://theme/icon-m-reset"
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    visible: false

                    onClicked: {
                        console.log("Cancel download of: " + channelId + "index: "+index);
                        appWindow.cancelDownload(channelId, index);
                    }
                }


                IconButton {
                    id: playButton
                    icon.source: "image://theme/icon-m-play"
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    visible: false

                    onClicked: {
                        appWindow.playPodcast(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }

                /*
                IconButton {
                    id: playlistAddButton
                    icon.source: "image://theme/icon-m-add"
                    anchors.right: playButton.left
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    visible: playButton.visible

                    onClicked: {
                        appWindow.addToPlayList(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }
*/

                IconButton {
                    id: webButton
                    icon.source: "image://theme/icon-m-play"
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    visible: false

                    onClicked: {
                        appWindow.openWeb(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }



                Label {
                    id: errorDownloadingLabel
                    visible: false
                    text: qsTr("No media")
                    anchors.right: parent.right
                    font.pixelSize: Theme.fontSizeSmall
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    width: 170
                }

                // PodcastDownloadingProgress {
                //    id: downloadProgress
                //    anchors.right: parent.right
                //    anchors.rightMargin: Theme.horizontalPageMargin
                //    width: playButton.width
                //    visible: false
                //    anchors.verticalCenter: parent.verticalCenter
                //}

                states: [
                    State {
                        name: "get"
                        PropertyChanges {
                            target: downloadButton
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: true
                        }
                        PropertyChanges {
                            target: cancelButton
                            visible: false
                        }
                    },
                    State {
                        name: "queued"
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }
                        PropertyChanges {
                            target: queueButton
                            visible: true
                        }
                        PropertyChanges {
                            target: queued
                            visible: true
                        }

                        PropertyChanges {
                            target: channelPublished
                            visible: false
                        }
                    },
                    State {
                        name: "downloading"
                        PropertyChanges {
                            target: queueButton
                            visible: false
                        }
                        PropertyChanges {
                            target: queued
                            visible: false
                        }
                        PropertyChanges {
                            target: cancelButton
                            visible: true
                        }
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: false
                        }
                        PropertyChanges {
                            target: downloadBytesText
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: false
                        }
                        PropertyChanges {
                            target: lastPlayed
                            visible: false
                        }
                    },
                    State {
                        name: "downloaded"
                        PropertyChanges {
                            target: cancelButton
                            visible: false
                        }
                        PropertyChanges {
                            target: playButton
                            visible: true
                        }
                        PropertyChanges {
                            target: downloadedIndicator
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: true
                        }
                        PropertyChanges {
                            target: lastPlayed
                            visible: true
                        }
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }
                        PropertyChanges {
                            target: downloadBytesText
                            visible: false
                        }
                    },
                    State {
                        name: "played"
                        PropertyChanges {
                            target: downloadedIndicator
                            visible: true
                            color: Theme.primaryColor
                        }
                        PropertyChanges {
                            target: playButton
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: true//false
                        }
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }

                    },
                    State {
                        name: "undownloadable"
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }
                        PropertyChanges {
                            target: cancelButtonButton
                            visible: false
                        }
                        PropertyChanges {
                            target: playButton
                            visible: false
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: false
                        }
                        PropertyChanges {
                            target: errorDownloadingLabel
                            visible: true
                        }
                        PropertyChanges {
                            target: queued
                            visible: false
                        }

                    },

                    State {
                        name: "website"
                        PropertyChanges {
                            target: downloadButton
                            visible: false
                        }
                        PropertyChanges {
                            target: playButton
                            visible: false
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: false
                        }
                        PropertyChanges {
                            target: webButton
                            visible: true
                        }

                    }
                ]

            }
        }
    }


    Connections {
        target: ui
        onStreamingUrlResolved: {
            if (streamUrl.length < 5) {
                mainPage.infoBanner.text = qsTr("Unable to stream podcast.");
                mainPahe.infoBanner.show();
            } else {
                console.log("Streaming " + streamUrl + streamUrl);
                //mainPage.audioStreamer.playStream(streamUrl, streamTitle);
                player.playStream(streamUrl, streamTitle);

                pageStack.push(player);
            }
        }
    }

    Connections{
        target: ui

        onPlayFileWithInternalPlayer:{
            player.playFile(fileName);
            //player.pause();
            function isPlayer(x){return x==player;};
            if (!pageStack.find(isPlayer))
                pageStack.push(player);
        }

    }

}
