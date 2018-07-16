import QtQuick 2.7
import QtQuick.VirtualKeyboard 2.1
import QtQuick.Controls 2.0
//import an.qt.DataTransfer 1.0

Rectangle {
    property int h: 0

    width: 480
    height: 272-h
    color: "#F6F6F6"

//    MouseArea {
//        z: 99
//        anchors.fill: parent
//        enabled: true
//        onClicked: console.log("MouseArea X:"+ mouseX + "   Y:" +  mouseY)

//    }

    MyKeyboard {
//        id: keyboard
        id: inputPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Image {
            id: pointer
            x: 30
            y: 6
            z: 5
            width: 30 / 3
            height: 28 / 2
            source: "qrc:/widgetphoto/wifi/pointer.png"
        }
    }

//    MyInputPanel {
//        id: inputPanel
//        y:  parent.height - inputPanel.height
//        anchors.left: parent.left
//        anchors.right: parent.right
////        focus: true

//        Image {
//            id: pointer
//            x: 30
//            y: 6
//            width: 30 / 3
//            height: 28 / 2
//            source: "qrc:/widgetphoto/wifi/pointer.png"
//        }
//    }

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

        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhPreferLowercase

        onTextChanged: {
            preText.visible = false
        }
        onEditingFinished: {
            dt.editFinished(textInput.text)
        }
    }

    Connections {
        target: dt
        onMouseMoved: {
            pointer.x = mouseX - pointer.width/2
            pointer.y = mouseY - pointer.height/2 - (272-h-inputPanel.height)

//            inputPanel.focus = true
//            console.log("recv:\t" + pointer.x + '\t' + pointer.y)

        }
        onInputClear: {
            textInput.text = str1
            preText.text = str2
            preText.visible = true
        }
        onViewInit: {
            h = h_space
            dt.qml_view_init(272-h-inputPanel.height)
        }
    }
}
















