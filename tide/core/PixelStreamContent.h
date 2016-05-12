/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#ifndef PIXEL_STREAM_CONTENT_H
#define PIXEL_STREAM_CONTENT_H

#include "Content.h"
#include <boost/serialization/base_object.hpp>

class PixelStreamContent : public Content
{
    Q_OBJECT
    Q_PROPERTY( int page READ getPage CONSTANT )
    Q_PROPERTY( int pageCount READ getPageCount CONSTANT )

public:
    /**
     * Constructor.
     * @param uri The unique stream identifier.
     */
    PixelStreamContent( const QString& uri, bool showPreviousNextButtons );

    /** Get the content type **/
    CONTENT_TYPE getType() const override;

    /**
     * Content method overload, not used for PixelStreams.
     * @return always returns true
    **/
    bool readMetadata() override;

    /** @return true if the streamer can handle aspect ratio changes. */
    bool hasFixedAspectRatio() const override;

    /** Get the current page number. */
    int getPage() const;

    /** Get the total number of pages. */
    int getPageCount() const;

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    PixelStreamContent() {}

    template<class Archive>
    void serialize( Archive & ar, const unsigned int )
    {
        // serialize base class information (with NVP for xml archives)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( Content );
        ar & boost::serialization::make_nvp( "showPreviousNextButtons",
                                             _showPreviousNextButtons );
    }

    bool _showPreviousNextButtons = false;
};

#endif
