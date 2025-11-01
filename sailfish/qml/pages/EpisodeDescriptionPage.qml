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
//import QtWebKit 3.0

Page {

    id: epDescPage

    allowedOrientations: Orientation.All

    property string episodeDescriptionText
    property string episodePublished
    property string episodeName

    property string style : "<style type='text/css'>
        body{
        color: " +Theme.primaryColor +";
        padding: " + Theme.paddingSmall  + "px;
        }

        a{
        color: "+Theme.highlightColor + "
        }
</style>";

    /*
    function findAllMethods(obj) {
        return Object.getOwnPropertyNames(obj).filter(function(property) {
            return typeof obj[property] == "function";
        });
        }

     function all_properties(obj){
        return Object.getOwnPropertyNames(obj);
     }
     */

    onEpisodeDescriptionTextChanged: {
        //console.log("Loading html: "+episodeDescriptionText);
        //episodeDescription.loadHtml(style + episodeDescriptionText);
        //console.log(findAllMethods(episodeDescription._webPage));
        //console.log(all_properties(episodeDescription._webPage));
    }


    SilicaFlickable{
        anchors.fill: parent
        contentHeight: mainPageTitle.height + podcastEpisodeRect.height+ episodeDescription.height + sep.height + 4*Theme.paddingMedium

        Column {
            id: episodesPageColumn
            //width:parent.width
            anchors.fill: parent
            anchors.bottomMargin: Theme.paddingMedium
            spacing: Theme.paddingMedium

            PageHeader{
                id: mainPageTitle
                title: episodeDescriptionPage.episodeName
                wrapMode: Text.WordWrap
            }

            Rectangle {
                id: podcastEpisodeRect
                smooth: true
                color:  "transparent"
                width: parent.width
                height: channelLogo.height + 2*Theme.paddingMedium


                PodcastChannelLogo{
                    id: channelLogo
                    channelLogo: channel.logo
                    anchors.left: podcastEpisodeRect.left
                    anchors.top: podcastEpisodeRect.top
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.topMargin: Theme.paddingMedium
                    width: 130
                    height: 130
                }

                Label {
                    id: channelPublished
                    anchors.left: channelLogo.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.bottom: channelLogo.bottom
                    text: qsTr("Published: ") + episodeDescriptionPage.episodePublished
                    font.pixelSize: Theme.fontSizeSmall
                    width: episodeDescriptionPage.width - channelLogo.width - 2*Theme.horizontalPageMargin - Theme.paddingMedium
                }

            }

            Separator{
                id:sep
                width: parent.width
            }


            Label{
                id: episodeDescription

                text: episodeDescriptionText
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall

                padding: Theme.paddingMedium

                width: parent.width
                height: Text.paintedHeight

                onLinkActivated: {
                    console.log(link);
                    if (link != "about:blank"){
                        //request.action = WebView.IgnoreRequest;
                        Qt.openUrlExternally(link);
                    }else{
                        //request.action = WebView.AcceptRequest;
                    }
                }


                //height: parent.height - mainPageTitle.height -  podcastEpisodeRect.height - 3*Theme.paddingMedium - sep.height

            }

            /*SilicaWebView {
                id: episodeDescription

                width: parent.width
                height: parent.height - mainPageTitle.height -  podcastEpisodeRect.height - 3*Theme.paddingMedium - sep.height



                onNavigationRequested: {
                    console.log(request.navigationType);
                    console.log(request.url);
                    if (request.url != "about:blank"){
                        request.action = WebView.IgnoreRequest;
                        Qt.openUrlExternally(request.url);
                    }else{
                        request.action = WebView.AcceptRequest;
                    }
                }

                experimental.transparentBackground: true
                experimental.preferences.defaultFontSize: Theme.fontSizeSmall
                experimental.preferences.standardFontFamily: Theme.fontFamily
                experimental.preferences.javascriptEnabled: false


                VerticalScrollDecorator{
                }

            }*/

        }
    }

}
