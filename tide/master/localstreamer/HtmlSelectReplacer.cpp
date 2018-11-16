/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "HtmlSelectReplacer.h"

#include <QFile>
#include <QRegExp>

namespace
{
const QString JQUERY_FILE = {":/web/js/selectboxit/jquery.min.js"};
const QString JQUERY_UI_FILE = {":/web/js/selectboxit/jquery-ui.min.js"};
const QString SELECTBOXIT_JS_FILE = {
    ":/web/js/selectboxit/jquery.selectBoxIt.min.js"};
const QString SELECTBOXIT_CSS_FILE = {":/web/js/selectboxit/selectboxit.css"};
const QString SELECTBOXIT_REPLACE = {
    "var selectBox = $(\"select\").selectBoxIt();"};
}

QString _read(const QString& jsFile)
{
    QFile file(jsFile);
    file.open(QIODevice::ReadOnly);
    const auto js = file.readAll();
    file.close();
    return js;
}

QString _readCss()
{
    QFile file{SELECTBOXIT_CSS_FILE};
    file.open(QIODevice::ReadOnly);
    auto cssStyle = QString{file.readAll()};
    file.close();
    cssStyle.remove(QRegExp{"[\\n\\t\\r]"});
    return QString{
        "loadCSS = function(href) {"
        "  var cssStyle = $(\"<style> %1 </style>\");"
        "  $(\"head\").append(cssStyle);"
        "};"
        "loadCSS();"}
        .arg(cssStyle);
}

HtmlSelectReplacer::HtmlSelectReplacer()
    : _jQuery(_read(JQUERY_FILE))
    , _jQueryUI(_read(JQUERY_UI_FILE))
    , _selectboxit(_read(SELECTBOXIT_JS_FILE))
    , _selectboxitCss(_readCss())
{
}

const QString& HtmlSelectReplacer::getJQuery() const
{
    return _jQuery;
}

const QString& HtmlSelectReplacer::getJQueryUI() const
{
    return _jQueryUI;
}

const QString& HtmlSelectReplacer::getSelectboxit() const
{
    return _selectboxit;
}

const QString& HtmlSelectReplacer::getSelectboxitCss() const
{
    return _selectboxitCss;
}

const QString& HtmlSelectReplacer::getSelectboxitReplace() const
{
    return SELECTBOXIT_REPLACE;
}
