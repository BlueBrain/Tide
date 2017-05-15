/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#include "State.h"

#include "log.h"
#include "scene/ContentFactory.h"

#include <QtXml/QtXml>
#include <QtXmlPatterns/QXmlQuery>

State::State()
    : _displayGroup(new DisplayGroup(QSizeF()))
    , _version(INVALID_FILE_VERSION)
{
}

State::State(DisplayGroupPtr displayGroup)
    : _displayGroup(displayGroup)
    , _version(INVALID_FILE_VERSION)
{
}

bool _isLegacyVersion(QXmlQuery& query)
{
    QString qstring;

    int version = -1;
    query.setQuery("string(/state/version)");

    if (query.evaluateTo(&qstring))
        version = qstring.toInt();

    if (version == LEGACY_FILE_VERSION)
        return true;

    put_flog(LOG_DEBUG, "not a legacy state file. version: %i, legacy: %i",
             version, LEGACY_FILE_VERSION);
    return false;
}

ContentPtr _loadContent(QXmlQuery& query, const int index)
{
    char string[1024];
    sprintf(string, "string(//state/ContentWindow[%i]/URI)", index);
    query.setQuery(string);

    QString qstring;
    if (!query.evaluateTo(&qstring))
        return ContentPtr();

    const QString uri = qstring.trimmed(); // remove any whitespace
    return ContentFactory::getContent(uri);
}

ContentWindowPtr _restoreContent(QXmlQuery& query, ContentPtr content,
                                 const int index)
{
    double x, y, w, h, centerX, centerY, zoom;
    x = y = w = h = centerX = centerY = zoom = -1.;

    char string[1024];
    sprintf(string, "string(//state/ContentWindow[%i]/x)", index);
    query.setQuery(string);

    QString qstring;
    if (query.evaluateTo(&qstring))
    {
        x = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/y)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        y = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/w)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        w = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/h)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        h = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/centerX)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        centerX = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/centerY)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        centerY = qstring.toDouble();
    }

    sprintf(string, "string(//state/ContentWindow[%i]/zoom)", index);
    query.setQuery(string);

    if (query.evaluateTo(&qstring))
    {
        zoom = qstring.toDouble();
    }

    ContentWindowPtr contentWindow(new ContentWindow(content));

    QRectF windowCoordinates(contentWindow->getCoordinates());
    if (x != -1. || y != -1.)
        windowCoordinates.moveTopLeft(QPointF(x, y));
    if (w != -1. || h != -1.)
        windowCoordinates.setSize(QSizeF(w, h));
    contentWindow->setCoordinates(windowCoordinates);

    QRectF zoomRect(contentWindow->getContent()->getZoomRect());
    if (zoom != -1.)
        zoomRect.setSize(QSizeF(1.0 / zoom, 1.0 / zoom));
    if (centerX != -1. || centerY != -1.)
        zoomRect.moveCenter(QPointF(centerX, centerY));
    contentWindow->getContent()->setZoomRect(zoomRect);

    return contentWindow;
}

bool State::legacyLoadXML(const QString& filename)
{
    QXmlQuery query;

    if (!query.setFocus(QUrl(filename)))
    {
        put_flog(LOG_DEBUG, "Not a valid legacy session: '%s'",
                 filename.toLocal8Bit().constData());
        return false;
    }

    if (!_isLegacyVersion(query))
        return false;

    int numContentWindows = 0;
    query.setQuery("string(count(//state/ContentWindow))");

    QString qstring;
    if (query.evaluateTo(&qstring))
        numContentWindows = qstring.toInt();

    ContentWindowPtrs contentWindows;
    contentWindows.reserve(numContentWindows);
    for (int i = 1; i <= numContentWindows; ++i)
    {
        ContentPtr content = _loadContent(query, i);
        if (!content)
            content = ContentFactory::getErrorContent();

        ContentWindowPtr contentWindow = _restoreContent(query, content, i);
        if (contentWindow)
            contentWindows.push_back(contentWindow);
    }

    _displayGroup->setContentWindows(contentWindows);
    // Preserve appearence of legacy sessions.
    _displayGroup->setCoordinates(UNIT_RECTF);
    _version = LEGACY_FILE_VERSION;

    return true;
}

StateVersion State::getVersion() const
{
    return _version;
}

DisplayGroupPtr State::getDisplayGroup()
{
    return _displayGroup;
}
