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

#ifndef WEBBROWSER_CONTENT_H
#define WEBBROWSER_CONTENT_H

#include "PixelStreamContent.h"
#include <boost/serialization/base_object.hpp>

/**
 * The Webbrowser is a PixelStream extended with history navigation.
 */
class WebbrowserContent : public PixelStreamContent
{
    Q_OBJECT
    Q_PROPERTY( int page READ getPage CONSTANT )
    Q_PROPERTY( int pageCount READ getPageCount CONSTANT )

public:
    /**
     * Constructor.
     * @param uri The unique stream identifier.
     */
    explicit WebbrowserContent( const QString& uri );

    /** Get the content type **/
    CONTENT_TYPE getType() const override;

    /** @return false, webbrowsers can adjust their aspect ratio. */
    bool hasFixedAspectRatio() const final;

    /** Get the index of the page navigation history (currently fixed). */
    int getPage() const;

    /** Get the number of pages in the navigation history (currently fixed). */
    int getPageCount() const;

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    WebbrowserContent() {}

    template<class Archive>
    void serialize( Archive & ar, const unsigned int )
    {
        // serialize base class information (with NVP for xml archives)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( PixelStreamContent );
    }
};

#endif
