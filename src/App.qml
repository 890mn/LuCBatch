import QtQuick 2.15
import QtQuick.Controls 2.15
import QtMultimedia
import FluentUI

ApplicationWindow {
    id: window
    visible: true
    width: 650
    height: 200
    title: qsTr("LuCBatch-withQT By Hinar")

    property string showFolder: "This will Detected after Press LOCATE"
    property string selectedFolder: ""
    property bool isLargeWindow: false
    property bool isListening: false
    property bool isPress: false
    property int progressValue: 0

    Video {
        source: "qrc:/nova.mp4"
        anchors.fill: parent
        autoPlay: true
        fillMode: VideoOutput.PreserveAspectCrop
        loops: MediaPlayer.Infinite
        z: -1

        MouseArea {
            anchors.fill: parent
            onClicked: {
                Video.play();
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        opacity: 0.8

        Row {
            spacing: 10
            padding: 10

            FluText {
                id: pathLabel
                text: qsTr("Where is N0vaDesktop: ")
                color: "#1c2127"
                font.pixelSize: 18
                x: parent.x
            }

            TextField {
                id: folderPath
                color: "#ffffff"
                text: showFolder
                font.pixelSize: 16
                readOnly: true
                background: Rectangle {
                    color: "#74c8b1"
                    radius: 5
                    opacity: 0.6
                }
                padding: 10
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 15
            opacity: 0.8

            FluProgressButton {
                id: rescanButton
                text: qsTr("1 - Locate Path")
                onClicked: {
                    fileExtractor.rescanFolder()
                    selectedFolder = fileExtractor.getDetectedFolder()
                    showFolder = fileExtractor.getShowFolder()
                }
            }


            FluButton {
                id: startButton
                text: qsTr("2 - Listen Start")
                enabled: selectedFolder !== ""
                onClicked: {
                    if (!isPress) {
                        fileExtractor.startExtraction(selectedFolder)
                        isListening = true
                        progressValue = 0
                        isPress = true
                    }

                }
            }

            FluButton {
                id: toggleSizeButton
                text: qsTr("EX - Log Print")
                onClicked: {
                    if (isLargeWindow) {
                        window.width = 650
                        window.height = 200
                        isLargeWindow = false
                    } else {
                        window.width = 650
                        window.height = 700
                        isLargeWindow = true
                    }
                }
            }
        }

        FluProgressBar {
            id: progressBar
            width: parent.width / 7
            anchors.horizontalCenter: parent.horizontalCenter
            visible: isListening
            to: 100
            value: progressValue
        }

        ScrollView {
            id: scrollView
            width: parent.width
            height: parent.height
            x: parent.x + 5
            visible: isLargeWindow

            TextArea {
                id: logArea
                width: parent.width
                height: parent.height
                x: parent.x + 10
                opacity: 0.8
                readOnly: true
                wrapMode: Text.Wrap
                textFormat: TextEdit.PlainText
                padding: 10
                color: "#000000"
                font.pixelSize: 16

                background: {
                    opacity: 0.2
                }
            }

            ScrollBar.vertical: FluScrollBar {}
        }
    }

    Connections {
        target: fileExtractor
        onExtractionLog: {
            logArea.text += log + "\n"
        }
        onExtractionCompleted: {
            logArea.text += "Extraction completed.\n"
            isListening = false
        }
        onProgressUpdate: {
            progressValue = progress
        }
    }
}
