import QtQuick 2.7
import QtQuick.Controls 2.2

//Rectangle {
Item {
    id: root
    width: 480
    height: 272


    //    color: "red"
    //    color: Qt.transparent

    //    BusyIndicator{
    //        id: busyIndicator
    //        anchors.centerIn: parent
    //        running: true

    ////        onc
    //    }

    BusyIndicator {
        id: control
        objectName: "busy"
        anchors.centerIn: parent

        running: false

        onVisibleChanged: {
            running = control.visible
            console.log("running = " + running)
        }

        contentItem: Item {
            implicitWidth: 64
            implicitHeight: 64

            Item {
                id: item
                x: parent.width / 2 - 32
                y: parent.height / 2 - 32
                width: 64
                height: 64
//                width: 32
//                height: 32
                opacity: control.running ? 1 : 0

                Behavior on opacity {
                    OpacityAnimator {
                        duration: 250
                    }
                }

                RotationAnimator {
                    target: item
                    running: control.visible && control.running
                    from: 0
                    to: 360
                    loops: Animation.Infinite
                    duration: 1250
                }

                Repeater {
                    id: repeater
                    model: 7

                    Rectangle {
                        x: item.width / 2 - width / 2
                        y: item.height / 2 - height / 2
                        implicitWidth: 10
                        implicitHeight: 10
                        radius: 5
//                        color: "#21be2b"
                        color: "darkBlue"
                        transform: [
                            Translate {
                                y: -Math.min(item.width, item.height) * 0.5 + 5
                            },
                            Rotation {
                                angle: index / repeater.count * 360
                                origin.x: 5
                                origin.y: 5
                            }
                        ]
                    }
                }
            }
        }
    }
}
