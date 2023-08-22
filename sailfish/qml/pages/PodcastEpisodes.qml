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

Page {
    id: episodesPage

    allowedOrientations: Orientation.All

    onStatusChanged:{
        if(status ===PageStatus.Active && player.active){
            pageStack.pushAttached(player);
        }
    }

    SilicaFlickable{
        anchors.fill: parent



        PullDownMenu{

            MenuItem {
                text: qsTr("Remove subscription")
                onClicked: {
                    remorseSubscription.execute(qsTr("Remove subscription"), function(){
                        appWindow.deleteChannel(channel.channelId);
                        pageStack.pop(mainPage);
                    }
                    )
                }
            }


            MenuItem {
                text: qsTr("Delete all downloaded podcasts")
                onClicked: {
                    //queryDeletePodcasts.open();
                    remorseDownloads.execute(qsTr("Delete all downloaded podcasts"), function(){
                        ui.deletePodcasts(channel.channelId);
                    }
                    )
                }
            }

            MenuItem {
                text: qsTr("Mark all podcasts as played")
                onClicked: {
                    appWindow.allListened(channel.channelId);
                }
            }

            MenuItem {
                text: qsTr("Mark all episodes as finished")
                onClicked: {
                    appWindow.allFinished(channel.channelId);
                }
            }

            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    appWindow.refreshEpisodes(channel.channelId)
                    refreshingBanner.show()
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }

            /*
            MenuItem{
                text: qsTr("Sort Episodes")
                onClicked: pageStack.push(Qt.resolvedUrl("EpisodesSortDialog.qml"))
                enabled: true
            }
            */

        }



        RemorsePopup{
            id:remorseSubscription
            text: qsTr("Remove subscription")
        }

        RemorsePopup{
            id:remorseDownloads
            text: qsTr("Delete downloaded podcasts")
        }

        Column {
            id: episodesPageColumn
            anchors.fill: parent
            spacing: Theme.paddingMedium

            PageHeader{
                id: chanelTitle
                title: channel.title
                wrapMode: Text.WordWrap
            }


            PodcastEpisodesChannelInfo {
                id: episodeData
                //width: parent.width
                //height: 218
            }


            Separator{
                width:parent.width
            }

            PodcastEpisodesList {
                id: episodesList
                width: parent.width
                height: parent.height - episodeData.height -chanelTitle.height - 3*Theme.paddingMedium
                channelId: channel.channelId
            }
        }


        EpisodeDescriptionPage {
            id: episodeDescriptionPage
        }



        Connections {
            target: ui
            onShowInfoBanner: {
                console.log("Showing banner: "+text);
                uiInfoBanner.text = text
                uiInfoBanner.show();
            }
        }



    }
    InfoBanner {
        id: refreshingBanner
        text:  qsTr("Refreshing episodes...")
        timerShowTime: 1500
    }

    InfoBanner {
        id: uiInfoBanner
    }
}
