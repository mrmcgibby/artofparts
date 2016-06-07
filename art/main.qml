import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtMultimedia 5.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

ApplicationWindow {
    id: app_window
    title: qsTr("Art of Parts")
    visible: true

    property size imageSize: "1265x645"
    property int time1: 17000
    property int time2: 2000
    property date start_date: "2000-01-01"
    property date end_date: "2030-01-01"
    property var colors: []

    Settings {
        property alias s_final_color: final_color.color
        property alias s_label_color: label_color.color
        property alias s_border_color: border_color.color
        property alias s_highlight_color: highlight_color.color
        property alias s_spreadsheet_id: spreadsheet_id.text
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Load Orders from Spreadsheet")
                onTriggered: load_sheet();
            }
            MenuItem {
                text: qsTr("&Load Orders from Shop")
                onTriggered: load();
            }
            MenuItem {
                text: qsTr("Final Color")
                onTriggered: final_color.open();
            }
            MenuItem {
                text: qsTr("Label Color")
                onTriggered: label_color.open();
            }
            MenuItem {
                text: qsTr("Border Color")
                onTriggered: border_color.open();
            }
            MenuItem {
                text: qsTr("Highlight Color")
                onTriggered: highlight_color.open();
            }
            MenuItem {
                text: "Settings"
                onTriggered: settings.open();
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    ColorDialog {
        id: final_color
        color: "black"
    }

    ColorDialog {
        id: label_color
        color: "white"
    }

    ColorDialog {
        id: border_color
        color: "white"
    }

    ColorDialog {
        id: highlight_color
        color: "white"
    }

    Dialog {
        id: settings
        title: "Settings"
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        
        GridLayout {
            columns: 2
            Label { text: "Spreadsheet ID" }
            TextField {
                id: spreadsheet_id
                text: "1bUNb5hSefnILVJKG16X3cqgCXXU1Tkyn6eHzHpyP7F8"
            }
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
            Label { text: "Show Image" }
            CheckBox {
                id: show_image
                checked: false
            }
        }
    }

    MessageDialog {
        id: noDataDialog
        title: "No orders found"
        text: "No orders found"
    }

    MessageDialog {
        id: spreadsheetLoadFailedDialog
        title: "spreadsheet failed to load"
        text: "spreadsheet failed to load"
    }

    SoundEffect {
        id: machine
        source: "machine.wav"
    }

    function load_sheet() {
	balance.size = imageSize;
	var endpoint = "https://spreadsheets.google.com/feeds/list/"+spreadsheet_id.text+"/od6/public/basic?alt=json";
        console.log(endpoint);
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
	    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                balance.clear();
                colors = {};

		var data = JSON.parse(xmlhttp.responseText);
		var entry = data.feed.entry;
                if (entry.length === 0) {
                    noDataDialog.open();
                    return;
                }

		for (var i = 0; i < entry.length; i += 1) {
		    var obj = {
			name: entry[i].title.$t
		    };
		    var row = entry[i].content.$t;
		    row = row.replace(/\s*/g,"");
		    row = row.split(",");
		    for (var j = 0; j < row.length; j += 1) {
			var cell = row[j].split(":");
			obj[cell[0]] = cell[1].replace(/^\$/,"");
		    }
		    console.log(JSON.stringify(obj));
		    balance.addBox(obj.name, parseFloat(obj.amount));
                    colors[obj.name] = obj.color;

		    function add(x, y) {
			if (obj[x] || obj[y]) {
			    balance.addDesired(Qt.point(
				parseFloat(obj[x]),
				parseFloat(obj[y])));
			    console.log(obj[x]);
			    console.log(obj[y]);
			} else {
			    balance.addDesired(Qt.point(
				Math.random()*imageSize.width,
				Math.random()*imageSize.height));
			}
		    }
		    add("x1", "y1");
		    add("x2", "y2");
		    add("x3", "y3");
		}
		console.log(JSON.stringify(colors));
		balance.optimize();
		machine.play();
	    }
	}
	xmlhttp.open("GET", endpoint, true);
	xmlhttp.send();
    }

    function load() {
        balance.size = imageSize;
        var endpoint = "https://artofparts.com/blog/wc-api/v2/orders?";
        endpoint = endpoint + "filter[meta]=true&"
        if (start_date_text.length > 0) {
            endpoint = endpoint + "filter[created_at_min]=" + start_date_text.text + "&";
        }
        if (end_date_text.length > 0) {
            endpoint = endpoint + "filter[created_at_max]=" + end_date_text.text + "&";
        }
        endpoint = endpoint + "filter[limit]=500&"
        endpoint = endpoint + "consumer_key=ck_7d156b157a9b71ed486a929a1ef13d16&"
        endpoint = endpoint + "consumer_secret=cs_4262136394a425b2871b01dfce1bc27b"
        getData(endpoint);
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
        if (data.orders.length === 0) {
            noDataDialog.open();
            return;
        }

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

    function get_color(name, index) {
        if (colors[name]) {
            return colors[name];
        } else {
            return Qt.hsla(0, 0, (index % 8) / 8, 1);
        }
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
            visible: show_image.checked
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
                z: 0
                property real ratio: parent.ratio
                x: parent.imageRect.x + parent.ratio * display.x
                y: parent.imageRect.y + parent.ratio * display.y
                width: parent.ratio * display.width
                height: parent.ratio * display.height
                border.color: border_color.color

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
                color: mouseArea.containsMouse ? highlight_color.color : get_color(name, index)

                Behavior on x { SmoothedAnimation { duration: balance.interval } }
                Behavior on y { SmoothedAnimation { duration: balance.interval } }
                Behavior on width { SmoothedAnimation { duration: balance.interval } }
                Behavior on height { SmoothedAnimation { duration: balance.interval } }

                Label {
                    z: 10
                    text: name
                    color: "transparent"
                    anchors.centerIn: parent

                    SequentialAnimation on color {
                        PauseAnimation { duration: time1 }
                        ColorAnimation {
                            to: label_color.color
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
                border.color: border_color.color

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
                        from: "white"
                        to: "white"
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
