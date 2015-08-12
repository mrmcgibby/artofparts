import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtMultimedia 5.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

ApplicationWindow {
    title: qsTr("Art of Parts")
    visible: true

    property size imageSize: "400x200"
    property int time1: 17000
    property int time2: 2000
    property date start_date: "2000-01-01"
    property date end_date: "2030-01-01"

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Load Orders")
                onTriggered: load();
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: "settings"
                onTriggered: settings.open();
            }
        }
    }

    Dialog {
        id: settings
        title: "Settings"
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        GridLayout {
            columns: 2
            Label { text: "Start Date" }
            TextField {
                id: start_date_text
                placeholderText: "2015-01-01"
            }
            Label { text: "End Date" }
            TextField {
                id: end_date_text
                placeholderText: "2015-01-01"
            }
        }

        onAccepted: {
            start_date = new Date(start_date_text.text);
            end_date = new Date(end_date_text.text);
        }
    }

    SoundEffect {
        id: machine
        source: "machine.wav"
    }

    function load() {
        balance.size = imageSize;
        getData("https://artofparts.com/blog/wc-api/v2/orders?filter[meta]=true&consumer_key=ck_7d156b157a9b71ed486a929a1ef13d16&consumer_secret=cs_4262136394a425b2871b01dfce1bc27b");
    }

    function getData(url) {
        var xmlhttp = new XMLHttpRequest();

        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                parseData(xmlhttp.responseText);
            }
        }
        xmlhttp.open("GET", url, true);
        xmlhttp.send();
    }

    function parseData(response) {
        var data = JSON.parse(response);
        var prices = {};

        balance.clear();
        for (var i = 0; i < data.orders.length; i += 1) {
            var order = data.orders[i];
            var first = order.customer.first_name;
            var last = order.customer.last_name;
            var name = first.substring(0,1) + last;
            if (name == "") {
                name = order.customer.username
            }

            var created = new Date(order.created_at);
            if (created < start_date || created > end_date) {
                continue;
            }

            if (!prices[name]) {
                prices[name] = 0;
            }
            for (var j = 0; j < order.line_items.length; j += 1) {
                prices[name] += parseFloat(order.line_items[j].price);
            }
            balance.addBox(name, prices[name]);

            if (order.order_meta) {
                var om = order.order_meta;
                balance.addDesired(Qt.point(om.myfld1, om.myfield2));
                balance.addDesired(Qt.point(om.myfield3, om.myfield4));
                balance.addDesired(Qt.point(om.myfield5, om.myfield6));
            } else {
                balance.addDesired(Qt.point(Math.random()*imageSize.width, Math.random()*imageSize.height));
                balance.addDesired(Qt.point(Math.random()*imageSize.width, Math.random()*imageSize.height));
                balance.addDesired(Qt.point(Math.random()*imageSize.width, Math.random()*imageSize.height));
            }
        }
        balance.optimize();
        machine.play();
    }

    function gray(index) {
        return Qt.hsla(0, 0, (index % 8) / 8)
    }

    Item {
        id: main
        anchors.fill: parent

        Rectangle {
            id: background
            anchors.fill: parent
            color: "black"
        }

        Image {
            anchors.fill: image
            source: "sample.jpg"
        }

        Rectangle {
            id: image
            property bool bars: parent.width/parent.height > imageSize.width/imageSize.height
            width: bars ? parent.height*imageSize.width/imageSize.height : parent.width
            height: bars ? parent.height : parent.width*imageSize.height/imageSize.width
            anchors.centerIn: parent
            color: "#000000FF"
        }

        property bool bars: parent.width/parent.height > imageSize.width/imageSize.height
        property size actualSize: bars ?
            Qt.size(parent.height*imageSize.width/imageSize.height, parent.height) :
            Qt.size(parent.width, parent.width*imageSize.height/imageSize.width)
        property real ratio: bars ? parent.height / imageSize.height : parent.width / imageSize.width
        property point imageRect: Qt.point((parent.width - actualSize.width)/2, (parent.height - actualSize.height)/2)

        Repeater {
            model: balance
            Rectangle {
                property real ratio: parent.ratio
                x: parent.imageRect.x + parent.ratio * display.x
                y: parent.imageRect.y + parent.ratio * display.y
                width: parent.ratio * display.width
                height: parent.ratio * display.height
                border.color: "white"

                Behavior on x { SmoothedAnimation { duration: balance.interval } }
                Behavior on y { SmoothedAnimation { duration: balance.interval } }
                Behavior on width { SmoothedAnimation { duration: balance.interval } }
                Behavior on height { SmoothedAnimation { duration: balance.interval } }

                ColorAnimation on color {
                    from: gray(index)
                    to: "#BBEEEEEE"
                    duration: time1
                }

                Label {
                    text: name
                    color: "transparent"
                    anchors.centerIn: parent

                    SequentialAnimation on color {
                        PauseAnimation { duration: time1 }
                        ColorAnimation {
                            to: "black"
                            duration: time2
                        }
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    radius: 5
                    color: "black"
                    SequentialAnimation on color {
                        PauseAnimation { duration: time1 }
                        ColorAnimation {
                            to: "transparent"
                            duration: time2
                        }
                    }
                }
            }
        }

        Repeater {
            model: balance
            Rectangle {
                visible: desired.x !== 0 && desired.y !== 0
                x: parent.imageRect.x + parent.ratio * desired.x - radius
                y: parent.imageRect.y + parent.ratio * desired.y - radius
                width: 14
                height: width
                radius: width/2
                border.color: "white"

                Behavior on x { SmoothedAnimation { duration: balance.interval } }
                Behavior on y { SmoothedAnimation { duration: balance.interval } }

                SequentialAnimation on border.color {
                    PauseAnimation { duration: time1 }
                    ColorAnimation {
                        to: "transparent"
                        duration: time2
                    }
                }

                SequentialAnimation on color {
                    ColorAnimation {
                        from: gray(index)
                        to: "#BBEEEEEE"
                        duration: time1
                    }
                    ColorAnimation {
                        to: "transparent"
                        duration: time2
                    }
                }
            }
        }
    }
}
