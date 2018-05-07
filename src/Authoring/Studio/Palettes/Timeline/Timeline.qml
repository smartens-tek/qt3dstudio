/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.8
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt3DStudio 1.0
import "../controls"

Rectangle {
    id: root

    property real splitterPos: 300
    property int itemHeight: 20
    property int itemExtraHeight: 80

    color: _backgroundColor

    ScrollBar {
        id: scrollBar
    }

    RowLayout {
        anchors.fill: parent

        spacing: 5

        ColumnLayout {
            anchors.fill: parent

            spacing: 0
            Layout.minimumWidth: root.splitterPos
            Layout.maximumWidth: root.splitterPos
            Layout.preferredWidth: root.width

            Item {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: itemHeight

                Row {
                    anchors.right: parent.right
                    StyledToggleButton {
                        width: 20
                        height: 20

                        enabledImage: "Toggle-Shy.png"
                        checkedImage: "Toggle-Shy.png"
                        toolTipText: checked ? qsTr("Show shy objects")
                                             : qsTr("Hide shy objects")

                        checked: _timelineView.hideShy
                        onClicked: _timelineView.hideShy = checked
                    }

                    StyledToggleButton {
                        width: 20
                        height: 20

                        enabledImage: "filter-toggle-eye-up.png"
                        checkedImage: "filter-toggle-eye-down.png"
                        toolTipText: checked ? qsTr("Show inactive objects")
                                             : qsTr("Hide inactive objects")

                        checked: _timelineView.hideHidden
                        onClicked: _timelineView.hideHidden = checked
                    }

                    StyledToggleButton {
                        width: 20
                        height: 20

                        checkedImage: "Toggle-Lock.png"
                        enabledImage: "Toggle-Lock.png"
                        toolTipText: checked ? qsTr("Show locked objects")
                                             : qsTr("Hide locked objects")

                        checked: _timelineView.hideLocked
                        onClicked: _timelineView.hideLocked = checked
                    }
                }

            }

            ListView {
                id: browserList

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 80
                Layout.preferredHeight: count * itemHeight
                Layout.preferredWidth: root.width

                ScrollBar.vertical: scrollBar

                model: _timelineView.objectModel
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                currentIndex: _timelineView.selection

                delegate: TimelineTreeDelegate {
                    id: delegateItem

                    property int extraHeight: model.propertyExpanded ? itemExtraHeight : 0

                    splitterPos: root.splitterPos
                    width: parent.width
                    height: model.parentExpanded && model.visible ? itemHeight + extraHeight : 0

                    visible: height > 0

                    Behavior on height {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.OutQuad
                        }
                    }

                    onDoubleClicked: {
                        if (model.isProperty)
                            model.propertyExpanded = !model.propertyExpanded;
                    }
                }

                onCurrentIndexChanged: _timelineView.selection = currentIndex

                Connections {
                    target: _timelineView
                    onSelectionChanged: {
                        if (browserList.currentIndex !== _timelineView.selection)
                        browserList.currentIndex = _timelineView.selection;
                    }
                }
            }
        }

        Flickable {
            id: timelineFlickable

            property bool movingPlayHead: false

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 80
            Layout.preferredHeight: (timelineItemsList.count + 1) * itemHeight
            Layout.preferredWidth: root.width
            boundsBehavior: Flickable.StopAtBounds

            contentHeight: height
            contentWidth: 2000
            clip: true
            interactive: !movingPlayHead

            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            Connections {
                target: _timelineView
                onCurrentTimePosChanged: {
                    // -10 compensates the width of the playhead
                    var pos = _timelineView.currentTimePos - 10;
                    if (pos < timelineFlickable.contentX ||
                        pos > timelineFlickable.contentX + timelineFlickable.width)
                        timelineFlickable.contentX = pos;
                }
            }

            Item {
                anchors.fill: parent

                ColumnLayout {
                    anchors.fill: parent

                    spacing: 0

                    TimeMeasureItem {
                        Layout.fillWidth: true
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: itemHeight
                        timeRatio: _timelineView.timeRatio

                        MouseArea {
                            id: timeMeasureItemMouseArea

                            anchors.fill: parent

                            onPressed: {
                                timelineFlickable.movingPlayHead = true;
                                _timelineView.setNewTimePosition(mouse.x)
                            }
                            onPositionChanged: _timelineView.setNewTimePosition(mouse.x)
                            onReleased: timelineFlickable.movingPlayHead = false;
                        }
                    }

                    ListView {
                        id: timelineItemsList

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: count * itemHeight
                        Layout.preferredWidth: root.width

                        ScrollBar.vertical: scrollBar

                        model: browserList.model
                        boundsBehavior: Flickable.StopAtBounds
                        clip: true
                        currentIndex: browserList.currentIndex

                        delegate: Rectangle {
                            id: timelineItemsDelegateItem

                            property int extraHeight: model.propertyExpanded ? itemExtraHeight : 0

                            width: parent.width
                            height: model.parentExpanded && model.visible ? itemHeight  + extraHeight
                                                                          : 0

                            color: model.selected ? _selectionColor : "#404244"
                            border.color: _backgroundColor

                            visible: height > 0

                            MouseArea {
                                id: timelineItemsDelegateArea

                                anchors.fill: parent
                                onClicked: _timelineView.select(model.index, mouse.modifiers)
                            }

                            Component {
                                id: standardTimelineItem

                                TimelineItem {
                                    height: parent ? parent.height : 0
                                    visible: timeInfo.endPosition > timeInfo.startPosition

                                    timeInfo: model.timeInfo
                                    color: model.itemColor
                                    borderColor: root.color
                                    selected: model.selected
                                    selectionColor: model.selectedColor
                                }
                            }

                            Component {
                                id: propertyTimelineItem

                                TimePropertyItem {
                                    height: parent ? parent.height : 0
                                    width: parent ? parent.width : 0
                                    timeRatio: _timelineView.timeRatio
                                    timelineRow: model.timelineRow
                                }
                            }

                            Loader {
                                anchors.fill: parent
                                sourceComponent: model.propertyExpanded ? propertyTimelineItem
                                                                        : standardTimelineItem
                            }

                            Keyframes {
                                anchors {
                                    top: parent.top
                                    topMargin: 10
                                }
                                keyframes: model.keyframes
                            }
                        }
                    }
                }

                PlayHead {
                    x: _timelineView.currentTimePos - width / 2
                    height: parent.height - scrollBar.height
                }
            }
        }
    }

    Rectangle {
        color: root.color
        x: splitterPos
        width: 5
        height: parent.height

        MouseArea {
            anchors {
                fill: parent
                margins: -3 // small trick to avoid the cursor changing back to arrow when dragging
            }

            hoverEnabled: true
            cursorShape: containsMouse ? Qt.SplitHCursor : Qt.ArrowCursor
            onPositionChanged: {
                if (mouse.buttons === Qt.LeftButton)
                    root.splitterPos = mapToItem(root, mouse.x, mouse.y).x
            }
        }
    }
}