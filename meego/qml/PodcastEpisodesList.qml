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
import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.0


Item {
    id: podcastsEpisodesList    

    signal selectedEpisodeDescription(string desc)

    property int channelId

    function downloadingStatusText(alreadyDownloadedSize, totalDownloadSize) {
        var downloadedStatus = parseInt(alreadyDownloadedSize) / 1024;  // to kB
        var totalDownloadStatus = parseInt(totalDownloadSize) / 1024;   // to kB

        var downloaded_mbs = Math.round(downloadedStatus / 1024);  // 1MB = 1024kB
        var total_mbs = Math.round(totalDownloadStatus / 1024); // 1MB = 1024kB

        var download_statusText = "Downloaded ";

        if (downloaded_mbs == 0) {
            download_statusText += Math.round(downloadedStatus % 1024) + " kB";
        } else {
            download_statusText += (downloadedStatus / 1024).toFixed(1) + " MB";
        }

        var total;
        if (total_mbs == 0) {
             total = Math.round(totalDownloadStatus % 1024);
            if (total > 0 ) {
                download_statusText += " of total " + total + " kB";
            }
        } else {
            total = Math.round(totalDownloadStatus / 1024);
            if (total > 0 ) {
                download_statusText += " of total " + total + " MB";
            }
        }

        return download_statusText;
    }

    Rectangle {
        id: podcastEpisodesInfoRect

        smooth: true
        color: "#F0F1F2"
        width: parent.width
        height:  parent.height - 75

        ListView {
            id: podcastEpisodesList
            anchors.fill: podcastEpisodesInfoRect
            model: episodesModel
            clip: true
            spacing: 20
            anchors.top:  podcastEpisodesInfoRect.top
            anchors.leftMargin: 5
            cacheBuffer: parent.height

            delegate: Item {
                id: podcastItem
                state: episodeState
                height: episodeRow.height
                width: parent.width

                MouseArea {
                    id: itemMouseArea
                    anchors.fill: parent
                    onClicked: {
                        episodeDescriptionPage.episodeDescriptionText = description;
                        episodeDescriptionPage.episodePublished = published
                        episodeDescriptionPage.episodeName = title
                        appWindow.pageStack.push(episodeDescriptionPage);
                    }

                    onPressAndHold: {
                        console.log("Long pressed. State:" + episodeState);
                        if (episodeState == "downloaded" ||
                            episodeState == "played") {
                            console.log("Press and hold, index: " + index);
                            podcastEpisodesList.currentIndex = index
                            contextMenu.open();
                        }

                        if (episodeState == "get") {
                            console.log("Press and hold, streaming index: " + index);
                            podcastEpisodesList.currentIndex = index
                            streamingContextMenu.open();

                        }
                    }

                    states: State {
                        name: "pressed"
                        when: (itemMouseArea.pressed)
                        PropertyChanges { target: listItemBackground; color: "#9501C5" }
                    }


                }

                Rectangle {
                    id: downloadedIndicator
                    width: 10
                    height: parent.height
                    color: "#9501C5"
                    anchors.left: podcastItem.left
                    visible: false
                }

                Rectangle {
                    id: listItemBackground
                    width: parent.width
                    height: episodeRow.height
                    color: "transparent"
                    anchors.left: downloadedIndicator.right;
                }

                Row {
                    id: episodeRow
                    width: parent.width - downloadProgress.width
                    height: episodeName.height + 30
                    anchors.left: downloadedIndicator.right
                    anchors.leftMargin: 5;
                    anchors.top: parent.top
                    anchors.topMargin: 5

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        Label {
                            id: episodeName
                            text: title;
                            font.pixelSize: 21
                            font.bold: true

                            width: podcastItem.width - downloadedIndicator.width - downloadProgress.width - 30
                            height: Text.paintedHeight
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            id: channelPublished
                            font.pixelSize: 16
                            text: published
                            height: Text.paintedHeight
                        }

                        Label {
                            id: lastPlayed
                            anchors.left: episodeName.left
                            font.pixelSize: 16
                            text: lastTimePlayed
                            height: Text.paintedHeight
                        }

                        Label {
                            id: downloadBytesText
                            anchors.left: episodeName.left
                            font.pixelSize: 16
                            text: downloadingStatusText(alreadyDownloadedSize, totalDownloadSize);
                            height: Text.paintedHeight
                            visible: false;
                        }
                    }
                }

                Rectangle {
                    id: separatorLine
                    width: podcastItem.width - 15
                    height: 1
                    anchors.top: episodeRow.bottom
                    color:  "#d8d8d9"
                }


                Button {
                    id: downloadButton
                    text: "GET"
                    anchors.right: podcastItem.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    width: 100
                    visible: true

                    onClicked: {
                        appWindow.downloadPodcast(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }

                QueueButton {
                    id: queueing
                    visible: false
                    anchors.right: podcastItem.right
                    anchors.rightMargin: 15
                    anchors.top:  parent.top
                    anchors.verticalCenter: parent.verticalCenter
                }

                Button {
                    id: playButton
                    text: "PLAY"
                    anchors.right: podcastItem.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    width: 100
                    visible: false

                    onClicked: {
                        appWindow.playPodcast(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }

                Button {
                    id: webButton
                    text: "WEB"
                    anchors.right: podcastItem.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    width: 100
                    visible: false

                    onClicked: {
                        appWindow.openWeb(channelId, index);  // Channel id = which model to use, index = row in the model.
                    }
                }

                Label {
                    id: errorDownloadingLabel
                    visible: false
                    text: "No media"
                    anchors.right: podcastItem.right
                    font.pointSize: 20
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    width: 170
                }

                PodcastDownloadingProgress {
                    id: downloadProgress
                    anchors.right: podcastItem.right
                    anchors.rightMargin: 15
                    width: 170
                    visible: false
                    anchors.verticalCenter: parent.verticalCenter
                }

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
                            target: downloadProgress
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
                            target: queueing
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
                            visible: true
                        }
                    },
                    State {
                        name: "downloading"
                        PropertyChanges {
                            target: queueing
                            visible: false
                        }
                        PropertyChanges {
                            target: downloadProgress
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
                    },
                    State {
                        name: "downloaded"
                        PropertyChanges {
                            target: downloadProgress
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
                            color:  "#d8d8d9"
                        }
                        PropertyChanges {
                            target: playButton
                            visible: true
                        }
                        PropertyChanges {
                            target: channelPublished
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

            ScrollBar {
                scrollArea: parent
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 5
                anchors.bottomMargin: 5
                anchors.rightMargin: 5
            }
        }
    }

    Menu { id: contextMenu; visualParent: pageStack;
        MenuLayout {
            MenuItem {
                text: "Delete downloaded podcast"
                onClicked: {
                    console.log("Emiting deleteDownloaded() "+ podcastsEpisodesList.channelId + podcastEpisodesList.currentIndex);
                    appWindow.deleteDownloaded(podcastsEpisodesList.channelId, podcastEpisodesList.currentIndex);
                }
            }
         }
    }

    Menu { id: streamingContextMenu; visualParent: pageStack;
        MenuLayout {
            MenuItem {
                text: "Start streaming the podcast"
                onClicked: {
                    appWindow.startStreaming(podcastsEpisodesList.channelId, podcastEpisodesList.currentIndex);
                }

            }
         }
    }

    Connections {
        target: ui
        onStreamingUrlResolved: {
            if (streamUrl.length < 5) {
                mainPage.infoBanner.text = "Unable to stream podcast.";
                mainPahe.infoBanner.show();
            } else {
                console.log("Streaming " + streamUrl + streamUrl);
                mainPage.audioStreamer.playStream(streamUrl, streamTitle);

                if (episodesPage.pageStack.depth > 1)  {
                    episodesPage.pageStack.pop();
                }
            }
        }
    }

}
