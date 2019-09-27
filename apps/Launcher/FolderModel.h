/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QFileSystemModel>
#include <QList>

/**
 * Expose a filesystem model to Qml.
 *
 * This is a replacement for Qt.labs.folderlistmodel for which the caseSensitive
 * property is not working.
 *
 * Note: QFileSystemModel requires a QApplication (QGuiApplication segfaults)!
 * https://bugreports.qt.io/browse/QTBUG-32768
 */
class FolderModel : public QFileSystemModel
{
    Q_OBJECT
    Q_PROPERTY(QString rootFolder READ getRootFolder WRITE setRootFolder NOTIFY
                   rootFolderChanged)
    Q_PROPERTY(QStringList nameFilters READ getNameFilters WRITE setNameFilters
                   NOTIFY nameFiltersChanged)
    Q_PROPERTY(SortCategory sortCategory READ getSortCategory WRITE
                   setSortCategory NOTIFY sortCategoryChanged)
    Q_PROPERTY(SortOrder sortOrder READ getSortOrder WRITE setSortOrder NOTIFY
                   sortOrderChanged)
    Q_PROPERTY(bool hideExtensions READ getHideExtensions WRITE
                   setHideExtensions NOTIFY hideExtensionsChanged)
    Q_PROPERTY(QStringList years READ getYears NOTIFY yearsChanged)

public:
    FolderModel();

    enum SortCategory
    {
        Name,
        Size,
        Date
    };
    Q_ENUMS(SortCategory)

    enum SortOrder
    {
        Ascending,
        Descending
    };
    Q_ENUMS(SortOrder)

    enum FolderModelRoles
    {
        FileName = Qt::UserRole + 5,
        FilePath,
        FileSize,
        FileModified,
        FileIsDir,
        FilesInDir
    };
    Q_ENUMS(FolderModelRoles)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, const int role) const override;

    Q_INVOKABLE QModelIndex getRootIndex() const;
    Q_INVOKABLE QModelIndex getPathIndex(const QString& path) const;
    Q_INVOKABLE QString getParentFolder() const;

    QString getRootFolder() const;
    QStringList getNameFilters() const;
    SortCategory getSortCategory() const;
    SortOrder getSortOrder() const;
    bool getHideExtensions() const;

    QStringList getYears() const;

    Q_INVOKABLE void toggleSortOrder();
    Q_INVOKABLE void hideFolders(bool hide);

public slots:
    void setRootFolder(QString rootfolder);
    void setNameFilters(QStringList nameFilters);
    void setSortCategory(SortCategory sortCategory);
    void setSortOrder(SortOrder sortOrder);
    void setHideExtensions(bool hideExtensions);

signals:
    void rootFolderChanged(QString rootfolder);
    void nameFiltersChanged(QStringList nameFilters);
    void sortCategoryChanged(SortCategory sortCategory);
    void sortOrderChanged(SortOrder sortOrder);
    void hideExtensionsChanged(bool hideExtensions);
    void yearsChanged();

private:
    void _updateSorting();
    int _getQtSortCategory() const;
    Qt::SortOrder _getQtSortOrder() const;

    SortCategory _sortCategory = FolderModel::SortCategory::Name;
    SortOrder _sortOrder = FolderModel::SortOrder::Ascending;
    bool _hideExtensions = false;
};

#endif
