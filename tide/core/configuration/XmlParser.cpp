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

#include "XmlParser.h"

#include <QtXmlPatterns>

namespace
{
const QRegExp TRIM_REGEX("[\\n\\t\\r]");

deflect::View _toView(const QString& viewString)
{
    if (viewString == "left")
        return deflect::View::left_eye;
    else if (viewString == "right")
        return deflect::View::right_eye;
    else
        return deflect::View::mono;
}
}

XmlParser::XmlParser(const QString& filename)
{
    if (!query.setFocus(QUrl{filename}))
    {
        throw std::runtime_error("Invalid configuration file: '" +
                                 filename.toStdString() + "'");
    }
}

bool XmlParser::get(const QString& queryURI, double& value)
{
    bool ok = false;
    QString queryResult;
    double tmp = 0.0;

    query.setQuery(queryURI);
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toDouble(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool XmlParser::get(const QString& queryURI, int& value)
{
    bool ok = false;
    QString queryResult;
    int tmp = 0;

    query.setQuery(queryURI);
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toInt(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool XmlParser::get(const QString& queryURI, uint& value)
{
    bool ok = false;
    QString queryResult;
    uint tmp = 0;

    query.setQuery(queryURI);
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toUInt(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool XmlParser::get(const QString& queryURI, ushort& value)
{
    bool ok = false;
    QString queryResult;
    ushort tmp = 0;

    query.setQuery(queryURI);
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toUShort(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool XmlParser::get(const QString& queryURI, QString& value)
{
    QString queryResult;

    query.setQuery(queryURI);
    if (!query.evaluateTo(&queryResult))
        return false;

    value = queryResult.remove(QRegExp(TRIM_REGEX));
    return true;
}

bool XmlParser::get(const QString& queryURI, bool& value)
{
    QString result;
    if (!get(queryURI, result))
        return false;

    if (result == "true")
    {
        value = true;
        return true;
    }
    if (result == "false")
    {
        value = false;
        return true;
    }
    return false;
}

bool XmlParser::get(const QString& queryURI, QColor& color)
{
    QString result;
    if (get(queryURI, result))
    {
        const QColor tmpColor{result};
        if (tmpColor.isValid())
        {
            color = tmpColor;
            return true;
        }
    }
    return false;
}

void XmlParser::get(const QString& queryURI, SwapSync& swapsync)
{
    QString result;
    if (get(queryURI, result) && result == "hardware")
        swapsync = SwapSync::hardware;
}

void XmlParser::get(const QString& queryURI, deflect::View& view)
{
    QString result;
    if (get(queryURI, result) && !result.isEmpty())
        view = _toView(result);
}
