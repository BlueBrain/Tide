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

#include "FolderModel.h"

#include <QDateTime>

#include <set>

namespace
{
template <typename T>
QStringList _toQStringList(const T& set, const FolderModel::SortOrder sortOrder)
{
    QStringList list;
    auto listInserter = std::back_inserter(list);
    auto numberToString = [](const auto& i) { return QString::number(i); };
    if (sortOrder == FolderModel::SortOrder::Descending)
        std::transform(set.rbegin(), set.rend(), listInserter, numberToString);
    else
        std::transform(set.begin(), set.end(), listInserter, numberToString);
    return list;
}
}

FolderModel::FolderModel()
{
    setReadOnly(true);
    setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    setNameFilterDisables(false);
    connect(this, &QFileSystemModel::directoryLoaded, this,
            &FolderModel::yearsChanged);
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return {{FileName, "fileName"},   {FilePath, "filePath"},
            {FileSize, "fileSize"},   {FileModified, "fileModified"},
            {FileIsDir, "fileIsDir"}, {FilesInDir, "filesInDir"}};
}

QVariant FolderModel::data(const QModelIndex& index, const int role) const
{
    switch (role)
    {
    case FileName:
        return _hideExtensions ? fileInfo(index).baseName()
                               : fileInfo(index).fileName();
    case FilePath:
        return QFileSystemModel::filePath(index);
    case FileSize:
        return fileInfo(index).size();
    case FileModified:
        return fileInfo(index).lastModified();
    case FileIsDir:
        return isDir(index);
    case FilesInDir:
    {
        const auto info = fileInfo(index);
        if (!info.isDir())
            return 0;

        auto dir = QDir{info.absoluteFilePath()};
        dir.setFilter(QDir::Files);
        dir.setNameFilters(nameFilters());
        return dir.count();
    }
    }
    return QVariant();
}

QModelIndex FolderModel::getRootIndex() const
{
    return getPathIndex(getRootFolder());
}

QModelIndex FolderModel::getPathIndex(const QString& path) const
{
    return index(path);
}

QString FolderModel::getParentFolder() const
{
    auto dir = rootDirectory();
    dir.cdUp();
    return dir.absolutePath();
}

QString FolderModel::getRootFolder() const
{
    return rootPath();
}

QStringList FolderModel::getNameFilters() const
{
    return nameFilters();
}

FolderModel::SortCategory FolderModel::getSortCategory() const
{
    return _sortCategory;
}

FolderModel::SortOrder FolderModel::getSortOrder() const
{
    return _sortOrder;
}

QStringList FolderModel::getYears() const
{
    std::set<int> years;
    const auto parent = getPathIndex(getRootFolder());
    const auto numRows = rowCount(parent);
    for (auto i = 0; i < numRows; ++i)
    {
        const auto idx = index(i, 0, parent);
        const auto dateTime = idx.data(FileModified).toDateTime();
        const auto year = dateTime.date().year();
        years.insert(year);
    }
    return _toQStringList(years, getSortOrder());
}

void FolderModel::toggleSortOrder()
{
    setSortOrder((_sortOrder == SortOrder::Ascending) ? SortOrder::Descending
                                                      : SortOrder::Ascending);
}

bool FolderModel::getHideExtensions() const
{
    return _hideExtensions;
}

void FolderModel::setRootFolder(QString rootfolder)
{
    if (getRootFolder() == rootfolder)
        return;

    setRootPath(rootfolder);
    emit rootFolderChanged(rootfolder);
}

void FolderModel::setNameFilters(QStringList nameFilters)
{
    if (getNameFilters() == nameFilters)
        return;

    QFileSystemModel::setNameFilters(nameFilters);
    emit nameFiltersChanged(nameFilters);
}

void FolderModel::setSortCategory(FolderModel::SortCategory sortCategory)
{
    if (_sortCategory == sortCategory)
        return;

    _sortCategory = sortCategory;
    _updateSorting();
    emit sortCategoryChanged(sortCategory);
}

void FolderModel::setSortOrder(FolderModel::SortOrder sortOrder)
{
    if (_sortOrder == sortOrder)
        return;

    _sortOrder = sortOrder;
    _updateSorting();
    emit sortOrderChanged(sortOrder);
}

void FolderModel::setHideExtensions(const bool hideExtensions)
{
    if (_hideExtensions == hideExtensions)
        return;

    _hideExtensions = hideExtensions;
    emit hideExtensionsChanged(_hideExtensions);
}

void FolderModel::_updateSorting()
{
    sort(_getQtSortCategory(), _getQtSortOrder());
}

int FolderModel::_getQtSortCategory() const
{
    switch (_sortCategory)
    {
    case SortCategory::Name:
        return 0;
    case SortCategory::Size:
        return 1;
    case SortCategory::Date:
        return 3;
    default:
        throw std::logic_error("Not implemented");
    }
}

Qt::SortOrder FolderModel::_getQtSortOrder() const
{
    switch (_sortOrder)
    {
    case SortOrder::Ascending:
        return Qt::AscendingOrder;
    case SortOrder::Descending:
        return Qt::DescendingOrder;
    default:
        throw std::logic_error("Not implemented");
    }
}
