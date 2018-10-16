// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

import Launcher 1.0
import "style.js" as Style

Loader {
    id: gridListView

    signal openItem(string filePath, bool fileIsDir)
    signal folderAction(int filesInDir, string filePath)

    property variant foldersModel
    property variant sectionModel
    property real gridItemSize
    property bool showModificationDate: sectionModel

    Component {
        id: gridViewWithSections
        ListView {
            anchors.fill: parent
            model: gridListView.sectionModel
            delegate: FileBrowserGridSection {
                property string section: modelData
                interactive: false
                foldersModel: gridListView.foldersModel
                header: Text {
                    text: section
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: gridItemSize * Style.fileBrowserGridSectionTextRelSize
                    height: 2 * font.pixelSize
                    verticalAlignment: Text.AlignBottom
                }
                sectionFilterModel: ModelFilter {
                    id: sectionFilter
                    sourceModel: foldersModel
                    filterRole: FolderModel.FileModified
                    Component.onCompleted: {
                        setFilterFixedString(section)
                        updateHeight()
                    }
                    function getRootIndex() {
                        return mapFromSource(foldersModel.getRootIndex())
                    }
                }
                function getRowCount() {
                    return sectionFilter.rowCount(model.rootIndex)
                }
                function updateHeight() {
                    var rows = Math.ceil(getRowCount() / 3)
                    height = rows * cellHeight + headerItem.height
                }
            }
            ScrollBar {
            }
        }
    }

    Component {
        id: gridViewWithoutSections
        FileBrowserGridSection {
            anchors.fill: parent
            foldersModel: gridListView.foldersModel
            ScrollBar {
            }
        }
    }

    sourceComponent: sectionModel ? gridViewWithSections : gridViewWithoutSections
}
