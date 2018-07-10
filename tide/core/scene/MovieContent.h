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

#ifndef MOVIE_CONTENT_H
#define MOVIE_CONTENT_H

#include "Content.h"

enum ControlState
{
    STATE_PAUSED = 1 << 0,
    STATE_LOOP = 1 << 1
};

class MovieContent : public Content
{
    Q_OBJECT
    Q_DISABLE_COPY(MovieContent)

    Q_PROPERTY(qreal duration READ getDuration CONSTANT)
    Q_PROPERTY(qreal position READ getPosition NOTIFY positionChanged)
    Q_PROPERTY(bool skipping READ isSkipping NOTIFY skippingChanged)

public:
    /** Create a MovieContent from the given uri. */
    explicit MovieContent(const QString& uri);

    /** @copydoc Content::getType **/
    CONTENT_TYPE getType() const final;

    /**
     * Read movie informations from the source URI.
     * @return true on success, false if the URI is invalid or an error occured.
    **/
    bool readMetadata() final;

    /** @return the qml file which contains the movie controls. */
    QString getQmlControls() const final;

    /** @return OFF. */
    Interaction getInteractionPolicy() const final;

    /** @return the list of supported movie file extensions. */
    static const QStringList& getSupportedExtensions();

    /** @name Playback control. */
    //@{
    ControlState getControlState() const;
    qreal getDuration() const;
    qreal getPosition() const;
    void setPosition(qreal pos);
    bool isSkipping() const;
    void setSkipping(bool skipping);
    //@}

signals:
    /** @name QProperty notifiers */
    //@{
    void positionChanged(qreal pos);
    void skippingChanged(bool skipping);
    //@}

private slots:
    void _play();
    void _pause();

private:
    void _createActions();

    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    MovieContent() = default;

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Content);
        ar & _controlState;
        ar & _duration;
        ar & _position;
        ar & _skipping;
        // clang-format on
    }

    /** Serialize for saving to an xml file. */
    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int version)
    {
        // serialize base class information (with NVP for xml archives)
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Content);
        if (version >= 2)
            ar & boost::serialization::make_nvp("controlState", _controlState);
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
        // Actions are not saved to xml file. Their creation need to be done
        // after _controlState is restored.
        _createActions();
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    ControlState _controlState = STATE_LOOP;
    double _duration = 0.0;
    double _position = 0.0;
    bool _skipping = false;
};

BOOST_CLASS_VERSION(MovieContent, 2)

DECLARE_SERIALIZE_FOR_XML(MovieContent)

BOOST_CLASS_EXPORT_KEY(MovieContent)

#endif
