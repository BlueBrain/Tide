/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "PDFContent.h"

#include "data/PDF.h"

BOOST_CLASS_EXPORT_IMPLEMENT(PDFContent)

IMPLEMENT_SERIALIZE_FOR_XML(PDFContent)

PDFContent::PDFContent(const QString& uri)
    : VectorialContent(uri)
    , _pageNumber(0)
    , _pageCount(0)
{
    _init();
}

PDFContent::PDFContent()
    : _pageNumber(0)
    , _pageCount(0)
{
    _init();
}

void PDFContent::_init()
{
    connect(this, SIGNAL(pageChanged()), this, SIGNAL(modified()));
}

CONTENT_TYPE PDFContent::getType() const
{
    return CONTENT_TYPE_PDF;
}

bool PDFContent::readMetadata()
{
    const PDF pdf(_uri);
    if (!pdf.isValid())
        return false;

    _size = pdf.getSize();
    _pageCount = pdf.getPageCount();
    _pageNumber = std::min(_pageNumber, _pageCount - 1);

    return true;
}

const QStringList& PDFContent::getSupportedExtensions()
{
    static QStringList extensions;
    if (extensions.empty())
        extensions << "pdf";
    return extensions;
}

void PDFContent::nextPage()
{
    if (_pageNumber < _pageCount - 1)
    {
        ++_pageNumber;
        emit pageChanged();
    }
}

void PDFContent::previousPage()
{
    if (_pageNumber > 0)
    {
        --_pageNumber;
        emit pageChanged();
    }
}

int PDFContent::getPage() const
{
    return _pageNumber;
}

int PDFContent::getPageCount() const
{
    return _pageCount;
}
