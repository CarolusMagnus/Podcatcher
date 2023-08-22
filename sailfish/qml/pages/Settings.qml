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

import  org.nemomobile.configuration 1.0

Dialog {

    property variant downloadNumbers: ['1', '5', '0'];
    property variant keepDays: ['5', '10', '0'];
    property variant players: ['', /*'/usr/bin/jolla-mediaplayer', '/usr/bin/harbour-unplayer',*/ 'internal']

    allowedOrientations: Orientation.All

    ConfigurationValue{
        id:autoSyncConf
        key: "/apps/ControlPanel/Podcatcher/autosync"
        defaultValue: true
    }

    ConfigurationValue{
        id:autoDownloadConf
        key: "/apps/ControlPanel/Podcatcher/autodownload"
        defaultValue: true
    }

    ConfigurationValue{
        id:autoDownloadNumConf
        key: "/apps/ControlPanel/Podcatcher/autodownload_num"
        defaultValue: 1
    }

    ConfigurationValue{
        id: keepEpisodesConf
        key: "/apps/ControlPanel/Podcatcher/keep_episodes"
        defaultValue: 0
    }

    ConfigurationValue{
        id: keepUnplayedConf
        key: "/apps/ControlPanel/Podcatcher/keep_unplayed"
        defaultValue: 0
    }

    ConfigurationValue{
        id: mediaPlayerConf
        key: "/apps/ControlPanel/Podcatcher/mediaplayer"
        defaultValue: 0
    }


    ConfigurationValue{
        id: showPopularConf
        key: "/apps/ControlPanel/Podcatcher/showPopular"
        defaultValue: 0
    }

    ConfigurationValue{
        id: saveOnSDCardConf
        key: "/apps/ControlPanel/Podcatcher/saveOnSDCard"
        defaultValue: 0
    }

    SilicaFlickable{
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: col.height

        VerticalScrollDecorator {}

        Column{
            id:col
            width: parent.width
            //anchors.fill: parent

            DialogHeader{
                id:header
                title: qsTr("Settings")
            }

            TextSwitch{
                id: autoSync
                text: qsTr("Sync feeds on startup")
                description: qsTr("Download podcast feeds on startup. This may take some time.")
            }

            TextSwitch{
                id: autoDownload
                text: qsTr("Auto-download podcasts")
                description: qsTr("Should Podcatcher automatically download new episodes when the device is connected to the WiFi.")
            }

            ComboBox{
                id: autoDownloadNum

                label: qsTr("Episodes to auto-download")
                description: qsTr("The number of podcast episodes that should be automatically downloaded.")

                menu: ContextMenu{
                    Repeater{
                        model: downloadNumbers

                        MenuItem{
                            text: modelData
                        }

                    }
                }
            }

            ComboBox{
                id: keepEpisodes
                label: qsTr("Remove old episodes")
                description: qsTr("Remove podcast episodes that are older than the number of days specified here. 0 means do not remove any.")

                menu: ContextMenu{
                    Repeater{
                        model: keepDays

                        MenuItem{
                            text: modelData
                        }

                    }
                }
            }

            TextSwitch{
                id: keepUnplayed
                text: qsTr("Keep unplayed episodes")
            }


            TextSwitch{
                id: saveOnSD
                text: qsTr("Save episodes on SD card")
            }

            ComboBox{
                id: mediaplayer
                label: qsTr("Use mediaplayer ")
                description: qsTr("Which external program should be used for playing the downloaded podcasts?")

                menu: ContextMenu{
                    MenuItem{
                        text: qsTr("System Default")
                    }

                    /* Sailjail doesn't allow starting other apps directly
                    MenuItem{
                        text: qsTr("Jolla Mediaplayer")
                    }

                    MenuItem{
                        text: qsTr("Unplayer")
                    }
                    */

                    MenuItem{
                        text: qsTr("Internal Player")
                    }
                }
            }




            TextSwitch{
                id: showPopular
                text: qsTr("Show popular podcasts page")
            }
        }
    }

    onOpened: {
        autoSync.checked = autoSyncConf.value;
        autoDownload.checked = autoDownloadConf.value;
        autoDownloadNum.currentIndex = downloadNumbers.indexOf(autoDownloadNumConf.value)
        keepEpisodes.currentIndex = keepDays.indexOf(keepEpisodesConf.value)
        keepUnplayed.checked = keepUnplayedConf.value;
        mediaplayer.currentIndex = players.indexOf(mediaPlayerConf.value)
        showPopular.checked = showPopularConf.value;
        saveOnSD.checked = saveOnSDCardConf.value;
    }

    onAccepted: {
        autoSyncConf.value = autoSync.checked;
        autoDownloadConf.value = autoDownload.checked;
        autoDownloadNumConf.value = autoDownloadNum.value;
        keepEpisodesConf.value = keepEpisodes.value;
        keepUnplayedConf.value = keepUnplayed.checked;
        mediaPlayerConf.value = players[mediaplayer.currentIndex];
        showPopularConf.value = showPopular.checked;
        saveOnSDCardConf.value = saveOnSD.checked;
    }


}

