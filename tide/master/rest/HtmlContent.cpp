/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#include "HtmlContent.h"

#include <QFile>
#include <QTextStream>

namespace
{
std::string _readFile( const QString& uri )
{
    QFile file( uri );
    file.open( QIODevice::ReadOnly | QIODevice::Text );
    QTextStream in( &file );
    return in.readAll().toStdString();
}
}

HtmlContent::HtmlContent( zeroeq::http::Server& server )
    : indexPage{ "/", _readFile( ":///html/index.html" ) }
    , closeIcon { "img/close.svg", _readFile( ":///html/img/close.svg" ) }
    , focusIcon { "img/focus.svg", _readFile( ":///html/img/focus.svg" ) }
    , jquery { "js/jquery-3.1.1.min.js",
               _readFile( ":///html/jquery-3.1.1.min.js" ) }
    , jqueryUiStyles { "css/jquery-ui.css",
                       _readFile( ":///html/jquery-ui.css" ) }
    , jqueryUi { "js/jquery-ui.min.js",
                 _readFile( ":///html/jquery-ui.min.js" ) }
    , maximizeIcon { "img/maximize.svg",
                     _readFile( ":///html/img/maximize.svg" ) }
    , sweetalert { "js/sweetalert.min.js",
                   _readFile( ":///html/sweetalert.min.js" ) }
    , sweetalertStyles { "css/sweetalert.min.css",
                         _readFile( ":///html/sweetalert.min.css" ) }
    , tideJs { "js/tide.js", _readFile( ":///html/tide.js" ) }
    , tideStyles{ "css/styles.css", _readFile( ":///html/styles.css" ) }
    , tideVarsJs { "js/tideVars.js", _readFile( ":///html/tideVars.js" ) }
    , underscore { "js/underscore-min.js",
                   _readFile( ":///html/underscore-min.js" ) }
{
    server.handleGET( indexPage );
    server.handleGET( closeIcon );
    server.handleGET( focusIcon );
    server.handleGET( jquery );
    server.handleGET( jqueryUiStyles );
    server.handleGET( jqueryUi );
    server.handleGET( maximizeIcon );
    server.handleGET( sweetalert );
    server.handleGET( sweetalertStyles );
    server.handleGET( tideJs );
    server.handleGET( tideStyles );
    server.handleGET( tideVarsJs );
    server.handleGET( underscore );
}

