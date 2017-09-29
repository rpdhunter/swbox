import QtQuick 2.7
import QtQuick.VirtualKeyboard 2.1
import QtQuick.Controls 2.0
//import an.qt.DataTransfer 1.0

Rectangle {
    width: 480
    height: 272
    color: "#F6F6F6"
//    color: "red"

//    MouseArea {
//        anchors.fill: parent
//        onClicked: {
//            console.log("receive:\t" + mouseX + '\t' + mouseY)
//            mouse.accepted = false
//        }
//    }

    InputPanel {
        id: inputPanel
        y:  parent.height - inputPanel.height
        anchors.left: parent.left
        anchors.right: parent.right

//        Rectangle {
        Image {
            id: pointer
            x: 30
            y: 6
            width: 30 / 3
            height: 28 / 2
//            color: "green"
            source: "qrc:/widgetphoto/wifi/pointer.png"
        }
    }

    Text {
        id: preText
        color: "#565758"
        height: 30
        text: qsTr("请输入wifi密码")
        anchors.horizontalCenter: parent.horizontalCenter
    }

    TextInput {
        id: textInput
        x: 10
        y: 10
        width: parent.width
        height: 30
        focus: true
        font.pixelSize: 30

        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhPreferLowercase | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText

        onTextChanged: {
            preText.visible = false
//            console.log("input text:\t" + textInput.text)
        }
        onEditingFinished: {
//            console.log("edit finished:\t" + textInput.text)
            dt.editFinished(textInput.text)
        }
    }

    Connections {
        target: dt
//        onMouseClicked: {
//            pointer.x = mouseX - pointer.width/2
//            pointer.y = mouseY - pointer.height/2 -30
////            console.log("recv:\t" + mouseX + '\t' + mouseY)
//        }
        onMouseMoved: {
            pointer.x = mouseX - pointer.width/2
            pointer.y = mouseY - pointer.height/2 -30 + 10
//            console.log("recv:\t" + pointer.x + '\t' + pointer.y)
        }
        onInputClear: {
            textInput.text = ""
            preText.visible = true
        }
    }
}
















