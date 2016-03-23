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

#ifndef PDFCONTENT_H
#define PDFCONTENT_H

#include "VectorialContent.h"
#include <boost/serialization/base_object.hpp>

class PDFContent : public VectorialContent
{
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param uri The uri of the pdf document.
     */
    explicit PDFContent( const QString& uri );

    /** Get the content type **/
    CONTENT_TYPE getType() const override;

    /**
     * Reaad PDF informations from the source URI.
     * @return true on success, false if the URI is invalid or an error occured.
    **/
    bool readMetadata() override;

    static const QStringList& getSupportedExtensions();

    /** Rank0 : go to next page **/
    void nextPage();

    /** Rank0 : go to previous page **/
    void previousPage();

    /** Get the current page number. */
    int getPage() const;

signals:
    /** Emitted when the page number is changed **/
    void pageChanged();

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    PDFContent();
    void _init();

    template<class Archive>
    void serialize( Archive & ar, const unsigned int )
    {
        // serialize base class information (with NVP for xml archives)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( Content );
        ar & boost::serialization::make_nvp( "pageNumber", _pageNumber );
    }

    int _pageNumber;
    int _pageCount;
};

#endif // PDFCONTENT_H
