import QtQuick 2.15
import QtQuick.Controls 2.15
import FluentUI 1.0

FluWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("File Extractor")

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: qsTr("Select a Drive to Extract Files:")
        }

        ComboBox {
            id: driveSelector
            width: 200
            model: fileExtractor.getSystemDrives()  // 动态获取驱动器列表
        }

        FluFilledButton{
            text:"Start Extraction"
            onClicked: {
                fileExtractor.startExtraction(driveSelector.currentText)
            }
        }

        ProgressBar {
            id: progressBar
            width: 300
            from: 0
            to: 100
            value: 0
        }

        TextArea {
            id: logArea
            width: 400
            height: 200
            readOnly: true
        }
    }

    // 连接信号和槽，将 C++ 的日志显示到界面上
    Connections {
        target: fileExtractor
        onExtractionLog: logArea.text += log + "\n"
        onExtractionProgress: progressBar.value = progress
        onExtractionCompleted: progressBar.value = 100
    }
}

