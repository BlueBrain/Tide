/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael.Dumusc@epfl.ch                   */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "MultiChannelContent.h"

#include <deflect/Event.h>

class PixelStreamContent : public MultiChannelContent
{
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param uri The unique stream identifier.
     * @param size The initial size of the stream.
     * @param keyboard Show the keyboard action.
     */
    PixelStreamContent(const QString& uri, const QSize& size, bool keyboard);

    /** Get the content type **/
    CONTENT_TYPE getType() const override;

    /**
     * Content method overload, not used for PixelStreams.
     * @return always returns true
    **/
    bool readMetadata() override;

    /** Does this content already have registered EventReceiver(s) */
    bool hasEventReceivers() const;

    /** Register to receive events on this content. */
    void incrementEventReceiverCount();

    /** @return ON when hasEventReceivers() is true, otherwise OFF. */
    Interaction getInteractionPolicy() const final;

    /** Parse data received from the deflect::Stream. */
    virtual void parseData(QByteArray data) { Q_UNUSED(data); }
signals:
    /** Emitted when an Event occured. */
    void notify(deflect::Event event);

protected:
    // Default constructor required for boost::serialization
    // Derived classes can disable the keyboard action
    PixelStreamContent(const bool keyboard = true)
        : _hasKeyboardAction{keyboard}
    {
    }

private:
    void _createActions();

    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiChannelContent);
        ar & _eventReceiversCount;
        // clang-format on
    }

    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int /*version*/)
    {
        // serialize base class information (with NVP for xml archives)
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Content); // old xml format
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
        _createActions(); // actions are not saved to xml file
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    unsigned int _eventReceiversCount = 0;
    bool _hasKeyboardAction = true;
};

DECLARE_SERIALIZE_FOR_XML(PixelStreamContent)

BOOST_CLASS_EXPORT_KEY(PixelStreamContent)

#endif
