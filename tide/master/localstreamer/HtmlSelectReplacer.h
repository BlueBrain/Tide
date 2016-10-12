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

#ifndef HTMLSELECTREPLACER_H
#define HTMLSELECTREPLACER_H

#include <QObject>
#include <QString>

/**
 * This class provides the necessary scripts (based on jQuery and SelectBoxIt)
 * to substitute all \<select\> elements on a webpage by equivalent HTML lists.
 */
class HtmlSelectReplacer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( HtmlSelectReplacer )

    Q_PROPERTY( QString jQuery READ getJQuery CONSTANT )
    Q_PROPERTY( QString jQueryUI READ getJQueryUI CONSTANT )
    Q_PROPERTY( QString selectboxit READ getSelectboxit CONSTANT )
    Q_PROPERTY( QString selectboxitCss READ getSelectboxitCss CONSTANT )
    Q_PROPERTY( QString selectboxitReplace READ getSelectboxitReplace CONSTANT )

public:
    HtmlSelectReplacer();

    const QString& getJQuery() const;
    const QString& getJQueryUI() const;
    const QString& getSelectboxit() const;
    const QString& getSelectboxitCss() const;
    const QString& getSelectboxitReplace() const;

private:
    QString _jQuery;
    QString _jQueryUI;
    QString _selectboxit;
    QString _selectboxitCss;
};

#endif
