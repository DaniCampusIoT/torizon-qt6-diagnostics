import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width:   480
    height:  320
    title:   "Torizon Diagnostics"
    color:   "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Text {
            text:  "Torizon Diagnostics"
            font { pixelSize: 22; bold: true }
            color: "#cdd6f4"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                height: 120
                radius: 12
                color:  diagnostics.spiOk ? "#1e3a2f" : "#3a1e1e"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 6
                    Text {
                        text: "SPI"
                        font { pixelSize: 13; bold: true }
                        color: "#a6e3a1"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text:  diagnostics.spiOk ? "✔  LOOPBACK OK" : "✘  FAILED"
                        font.pixelSize: 18
                        color: diagnostics.spiOk ? "#a6e3a1" : "#f38ba8"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text:  "/dev/apalis-spi1-cs0"
                        font.pixelSize: 11
                        color: "#6c7086"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 120
                radius: 12
                color:  "#1e2a3a"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 6
                    Text {
                        text:  "I2C  •  TMP75C @ 0x4F"
                        font { pixelSize: 13; bold: true }
                        color: "#89b4fa"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text:  diagnostics.temperature.toFixed(4) + " °C"
                        font.pixelSize: 28
                        color: "#cdd6f4"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text:  "i2c-4"
                        font.pixelSize: 11
                        color: "#6c7086"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        Text {
            visible: diagnostics.errorMsg !== ""
            text:    "⚠  " + diagnostics.errorMsg
            color:   "#f38ba8"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "Refresh"
                onClicked: diagnostics.refresh()
            }
            Item { Layout.fillWidth: true }
            Text {
                text:  "Last update: " + diagnostics.lastUpdate
                color: "#6c7086"
                font.pixelSize: 12
            }
        }
    }
}