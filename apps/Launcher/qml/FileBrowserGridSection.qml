// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

import Launcher 1.0
import "style.js" as Style

GridView {
    id: gridView

    property variant foldersModel
    property variant sectionFilterModel

    anchors.left: parent.left
    anchors.right: parent.right

    cellWidth: gridItemSize * 1.33
    cellHeight: gridItemSize * 1.33

    model: DelegateModel {
        id: gridDelegateModel
        model: sectionFilterModel ? sectionFilterModel : foldersModel
        rootIndex: getRootIndex()

        function getRootIndex() {
            return sectionFilterModel ? sectionFilterModel.getRootIndex(
                                            ) : foldersModel.getRootIndex()
        }
        function resetRootIndex() {
            rootIndex = getRootIndex()
        }
        Connections {
            target: foldersModel
            onRootFolderChanged: resetRootIndex()
        }
        // Mysterious hack to prevent rootIndex from being reset
        // asynchronously when using a QFileSystemModel:
        // http://lists.qt-project.org/pipermail/interest/2015-August/018369.html
        onRootIndexChanged: {
            resetRootIndex()
        }
        delegate: FileBrowserGridItem {
            width: gridItemSize
            height: gridItemSize
            onClicked: gridListView.openItem(filePath, fileIsDir)
            onPressAndHold: {
                if (fileIsDir && filesInDir > 0) {
                    gridListView.folderAction(filesInDir, filePath)
                }
            }
        }
    }
}
