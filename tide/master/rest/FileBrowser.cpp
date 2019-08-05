/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#include "FileBrowser.h"

#include "json/json.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QJsonArray>
#include <QUrl>
#include <iostream>
namespace
{
QJsonObject _toJsonObject(const QFileInfo& entry)
{
    return QJsonObject{{"name", entry.fileName()}, {"dir", entry.isDir()}};
}

QJsonArray _toJsonArray(const QFileInfoList& list)
{
    QJsonArray array;
    for (const auto& entry : list)
        array.append(_toJsonObject(entry));
    return array;
}

bool isSupported(QString fileName, QStringList filters, bool isDir)
{
    if (isDir)
        return true;
    for (auto filter : filters)
    {
        if (fileName.endsWith(filter.remove("*")))
        {
            return true;
        }
    }
    return false;
}

} // namespace

FileBrowser::FileBrowser(const QString& baseDir, const QStringList& filters)
    : _baseDir{baseDir}
    , _filters{filters}
{
}

std::future<rockets::http::Response> FileBrowser::list(
    const rockets::http::Request& request)
{
    using namespace rockets::http;
    auto path = QString::fromStdString(request.path);
    QUrl url;
    url.setPath(path, QUrl::StrictMode);
    path = url.path();

    const QString fullpath = _baseDir + "/" + path;
    const QDir absolutePath(fullpath);
    if (!absolutePath.canonicalPath().startsWith(_baseDir))
        return make_ready_response(Code::BAD_REQUEST);

    if (!absolutePath.exists())
        return make_ready_response(Code::NO_CONTENT);

    const auto body = json::dump(_toJsonArray(_contents(fullpath)));
    return make_ready_response(Code::OK, body, "application/json");
}

std::future<rockets::http::Response> FileBrowser::find(
    const rockets::http::Request& request)
{
    using namespace rockets::http;
    auto path = QString::fromStdString(request.path);
    QUrl url;
    url.setPath(path, QUrl::StrictMode);
    path = url.path();
    auto queryParam = request.query;
    const QString fullpath = _baseDir + "/" + path;
    const QDir absolutePath(fullpath);

    if (!absolutePath.canonicalPath().startsWith(_baseDir))
        return make_ready_response(Code::BAD_REQUEST);

    if (!absolutePath.exists())
        return make_ready_response(Code::NO_CONTENT);

    if (queryParam.find("file") == queryParam.end())
    {
        return make_ready_response(Code::BAD_REQUEST);
    }

    auto fileName = QString::fromStdString(queryParam.at("file"));

    //    if (fileName.length() < 3)
    //        return make_ready_response(Code::BAD_REQUEST);

    QString fileNameRegex;
    fileNameRegex.append("*");
    fileNameRegex.append(fileName);
    fileNameRegex.append("*");

    QJsonArray list;
    QStringList combinedFilers;

    QDirIterator it(fullpath, QStringList() << fileNameRegex, QDir::AllEntries,
                    QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QFileInfo f(it.next());
        if (!isSupported(f.fileName(), _filters, f.isDir()))
        {
            //            qDebug() << f.fileName() << f.isDir();
            continue;
        }
        QDir baseDir(_baseDir);
        QJsonObject obj;
        obj.insert("name", f.fileName());
        obj.insert("path", baseDir.relativeFilePath(f.absoluteFilePath()));
        obj.insert("size", f.size());
        obj.insert("isDir", f.isDir());
        obj.insert("lastModified", f.lastModified().toString());
        list.append(obj);
    }
    const auto body = json::dump(list);
    return make_ready_response(Code::OK, body, "application/json");
}

QFileInfoList FileBrowser::_contents(const QDir& directory) const
{
    const auto filters = QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot;
    const auto sortFlags = QDir::DirsFirst | QDir::IgnoreCase;
    return directory.entryInfoList(_filters, filters, sortFlags);
}
