/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "SessionLoaderLegacyXml.h"

#include "configuration/XmlParser.h"
#include "scene/ContentFactory.h"

namespace
{
const QString query{"string(//state/ContentWindow[%1]/%2)"};

SessionFileVersion _loadVersion(XmlParser& parser)
{
    int version = INVALID_FILE_VERSION;
    parser.get("string(/state/version)", version);
    return SessionFileVersion(version);
}

ContentPtr _loadContent(XmlParser& parser, const QString& index)
{
    ContentPtr content;

    QString uri;
    if (parser.get(query.arg(index, "URI"), uri))
        content = ContentFactory::getContent(uri);
    if (!content)
        content = ContentFactory::getErrorContent(uri);

    return content;
}

WindowPtr _loadWindow(XmlParser& parser, const QString& index)
{
    double x, y, w, h, centerX, centerY, zoom;
    x = y = w = h = centerX = centerY = zoom = -1.0;
    parser.get(query.arg(index, "x"), x);
    parser.get(query.arg(index, "y"), y);
    parser.get(query.arg(index, "w"), w);
    parser.get(query.arg(index, "h"), h);
    parser.get(query.arg(index, "centerX"), centerX);
    parser.get(query.arg(index, "centerY"), centerY);
    parser.get(query.arg(index, "zoom"), zoom);

    auto window = std::make_shared<Window>(_loadContent(parser, index));

    auto windowCoordinates = window->getCoordinates();
    if (x != -1. || y != -1.)
        windowCoordinates.moveTopLeft(QPointF(x, y));
    if (w != -1. || h != -1.)
        windowCoordinates.setSize(QSizeF(w, h));
    window->setCoordinates(windowCoordinates);

    auto zoomRect = window->getContent().getZoomRect();
    if (zoom != -1.)
        zoomRect.setSize(QSizeF(1.0 / zoom, 1.0 / zoom));
    if (centerX != -1. || centerY != -1.)
        zoomRect.moveCenter(QPointF(centerX, centerY));
    window->getContent().setZoomRect(zoomRect);

    return window;
}

WindowPtrs _loadWindows(XmlParser& parser)
{
    auto numWindows = 0u;
    parser.get("string(count(//state/ContentWindow))", numWindows);

    auto windows = WindowPtrs();
    windows.reserve(numWindows);
    for (auto i = 1u; i <= numWindows; ++i)
        windows.emplace_back(_loadWindow(parser, QString::number(i)));

    return windows;
}

WindowPtrs _loadWindows(const QString& filename)
{
    XmlParser parser{filename};

    const auto version = _loadVersion(parser);
    if (version != LEGACY_FILE_VERSION)
    {
        throw std::runtime_error(
            std::string("version: ") +
            std::to_string(static_cast<int>(version)) + " != legacy: " +
            std::to_string(static_cast<int>(LEGACY_FILE_VERSION)));
    }

    return _loadWindows(parser);
}
}

Session SessionLoaderLegacyXml::load(const QString& filename)
{
    auto group = DisplayGroup::create(QSizeF(1.0, 1.0));
    group->replaceWindows(_loadWindows(filename));
    return Session(Scene::create(std::move(group)), filename,
                   LEGACY_FILE_VERSION);
}
