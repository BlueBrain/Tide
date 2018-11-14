/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#ifndef IMAGEPYRAMID_CONTENT_H
#define IMAGEPYRAMID_CONTENT_H

#include "Content.h"

/**
 * A multi-resolution tiled image.
 */
class ImagePyramidContent : public Content
{
public:
    /**
     * Constructor.
     * @param uri The uri of the image pyramid file.
     */
    explicit ImagePyramidContent(const QString& uri);

    /** Get the content type **/
    ContentType getType() const final;

    /** @copydoc Content::hasTransparency **/
    bool hasTransparency() const final;

    /**
     * Read texture metadata.
     * @return true on success, false if the URI is invalid or an error occured.
    **/
    bool readMetadata() final;

    /** @return true */
    bool canBeZoomed() const final;

    static const QStringList& getSupportedExtensions();

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    ImagePyramidContent() {}
    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Content);
        ar & _transparent;
        // clang-format on
    }

    /** Serialize for saving to an xml file. */
    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int)
    {
        // serialize base class information (with NVP for xml archives)
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Content);
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    bool _transparent = false;
};

DECLARE_SERIALIZE_FOR_XML(ImagePyramidContent)

BOOST_CLASS_EXPORT_KEY(ImagePyramidContent)

#endif
