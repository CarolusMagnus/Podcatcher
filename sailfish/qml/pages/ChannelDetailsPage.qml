/**
 * This file is part of Podcatcher for Sailfish OS.
 * Authors: Moritz Carmesin (carolus@carmesinus.de)
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
//import QtWebKit 3.0

Page {

    id: channelDescPage

    allowedOrientations: Orientation.All

    property string channelName
    property string channelDescriptionText
    property string episodePublished
    property bool editingFeedURL

    property var sortByFields: ["dbid","published", "state", "title"]

    onStatusChanged: {
        console.log("State changed to " + status )
        if (status == PageStatus.Activating){
            cbBy.currentIndex = sortByFields.indexOf(channel.sortBy);
            cbOrder.currentIndex = channel.sortDescending?0:1;
        }
    }

    SilicaFlickable{
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: channelPageColumn.height

        VerticalScrollDecorator {}


        Column {
            id: channelPageColumn
            anchors.bottomMargin: Theme.paddingMedium
            //spacing: Theme.paddingMedium
            width: parent.width

            PageHeader{
                id: mainPageTitle
                title: channelDescPage.channelName
                wrapMode: Text.WordWrap
            }

            Rectangle {
                id: channelRect
                smooth: true
                width: parent.width
                height: Math.max(channelLogo.height, channelDescription.height) + Theme.paddingMedium
                color: "transparent"


                PodcastChannelLogo{
                    id: channelLogo
                    channelLogo: channel.logo
                    anchors.left: channelRect.left
                    anchors.top: channelRect.top
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.topMargin: Theme.paddingMedium
                    width: 130
                    height: 130
                }

                Label {
                    id: channelDescription
                    text: channelDescriptionText
                    anchors.left: channelLogo.right
                    anchors.leftMargin: Theme.paddingMedium
                    anchors.rightMargin: Theme.horizontalPageMargin
                    width: parent.width - channelLogo.width - Theme.paddingMedium -2*Theme.horizontalPageMargin
                    height: Text.paintedHeight
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                }

            }


            Separator{
                id:sep
                width: parent.width
            }


            SectionHeader{
                text: qsTr("Download")
            }

            TextSwitch{
                id: autoDownloadSwitch
                width: parent.width


                text: qsTr("Auto-download")
                checked: channel.isAutoDownloadOn

                onCheckedChanged: {
                    appWindow.autoDownloadChanged(channel.channelId, checked);
                }
            }

            SectionHeader{
                text: qsTr("Sortation")
            }

            ComboBox{
                id: cbBy
                label: qsTr("Sort by")

                menu: ContextMenu{
                    MenuItem{
                        text: qsTr("ID")
                    }
                    MenuItem{
                        text: qsTr("publication date")
                    }

                    MenuItem{
                        text: qsTr("state")
                    }

                    MenuItem{
                        text: qsTr("title")
                    }
                }

                onCurrentIndexChanged:{
                    channel.sortBy = sortByFields[cbBy.currentIndex];
                }
            }

            ComboBox{
                id: cbOrder
                label: qsTr("Sort order")

                menu: ContextMenu{
                    MenuItem{
                        text: qsTr("descending")
                    }

                    MenuItem{
                        text: qsTr("ascending")
                    }
                }

                onCurrentIndexChanged: {
                    channel.sortDescending = (cbOrder.currentIndex <1);
                }

            }

            SectionHeader{
                text: qsTr("Feed")
            }

            Row{
                width: parent.width-Theme.horizontalPageMargin
                height: txURL.height
                x: Theme.horizontalPageMargin

                LinkedLabel{
                    id: txURL
                   // label: qsTr("URL")
                    text: channel.url
                    width: parent.width-btnEditFeed.width
                    height: Text.paintedHeight
                }

                IconButton{
                    id: btnEditFeed
                    icon.source:  "image://theme/icon-m-edit"
                    onClicked: {
                        var dialog = pageStack.push(Qt.resolvedUrl("URLEditDialog.qml"),
                                                                         {"url": channel.url})

                    }
                }
            }

        }
    }

}
