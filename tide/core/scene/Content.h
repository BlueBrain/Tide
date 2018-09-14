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

#ifndef CONTENT_H
#define CONTENT_H

#include "ContentType.h"
#include "serialization/includes.h"
#include "types.h"

#include <deflect/SizeHints.h>

#include <QObject>
#include <QSize>

/**
 * An abstract Content displayed in a Window.
 *
 * This class does not actually hold any content data because it
 * is meant to be sent through MPI to Rank>0 processes.
 * The content data is held by WallContent objects on Wall processes.
 */
class Content : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Content)

    // These properties are read-only on master and wall
    Q_PROPERTY(QString uri READ getUri CONSTANT)
    Q_PROPERTY(QString filePath READ getFilePath CONSTANT)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(bool hasFixedAspectRatio READ hasFixedAspectRatio CONSTANT)
    Q_PROPERTY(KeyboardState* keyboard READ getKeyboardState CONSTANT)
    Q_PROPERTY(bool captureInteraction READ getCaptureInteraction NOTIFY
                   captureInteractionChanged)
    Q_PROPERTY(bool transparency READ hasTransparency CONSTANT)

    // These properties can be constant because they are only accessed on wall
    Q_PROPERTY(qreal aspectRatio READ getAspectRatio CONSTANT)
    Q_PROPERTY(QRectF zoomRect READ getZoomRect CONSTANT)
    Q_PROPERTY(bool zoomed READ isZoomed CONSTANT)

public:
    /** Constructor **/
    Content(const QString& uri);

    /** Make a clone of this Content using binary serialization. */
    ContentPtr clone() const;

    /** Get the content URI **/
    const QString& getUri() const;

    /** Get the content type **/
    virtual ContentType getType() const = 0;

    /** @return the full path to the content file. */
    virtual QString getFilePath() const;

    /** Get the title of the content */
    virtual QString getTitle() const;

    /** @return true if the content has transparency (alpha channel). */
    virtual bool hasTransparency() const;

    /**
     * Read content metadata from the data source.
     *
     * Used on Rank0 for file-based content types to refresh data from source
     * URI.
     * @return true if the informations could be read.
    **/
    virtual bool readMetadata() = 0;

    /** Get the dimensions. */
    QSize getDimensions() const;

    /** Get the width. */
    int width() const;

    /** Get the height. */
    int height() const;

    /** @return the min dimensions, used to constrain resize/scale. */
    QSize getMinDimensions() const;

    /** @return the preferred dimensions, used to for 1:1 size. */
    QSize getPreferredDimensions() const;

    /** @return the max dimensions, used to constrain resize/scale. */
    virtual QSize getMaxDimensions() const;

    /** Set the dimensions. */
    void setDimensions(const QSize& dimensions);

    /** Get the aspect ratio. */
    qreal getAspectRatio() const;

    /** @return true if the content has a fixed aspect ratio. */
    virtual bool hasFixedAspectRatio() const;

    /** Get the keyboard state from QML. */
    virtual KeyboardState* getKeyboardState();

    /** @return true if the content can be zoomed. */
    virtual bool canBeZoomed() const;

    /** Get the zoom rectangle in normalized coordinates, [0,0,1,1] default */
    const QRectF& getZoomRect() const;

    /** Set the zoom rectangle in normalized coordinates. */
    void setZoomRect(const QRectF& zoomRect);

    /** @return true if the content is zoomed. */
    bool isZoomed() const;

    /** Resets the zoom to [0,0,1,1]. */
    void resetZoom();

    /** Set optional size hints to constrain resize/scale and 1:1 size. */
    void setSizeHints(const deflect::SizeHints& sizeHints);

    /** @return true if the content captures interaction. */
    bool getCaptureInteraction() const;

    /** Tell the content to capture interaction (only for Policy::AUTO). */
    void setCaptureInteraction(bool enable);

    /** Set the maximum factor for zoom and resize; value times content size */
    static void setMaxScale(qreal value);

    /** @return the maxium scale factor for zoom and resize */
    static qreal getMaxScale();

signals:
    /** @name QProperty notifiers */
    //@{
    void titleChanged(QString title);
    void interactionPolicyChanged();
    void captureInteractionChanged();
    //@}

    /** Emitted by any Content subclass when its state has been modified */
    void modified();

protected:
    // Default constructor required for boost::serialization
    Content();

    /** The different types of interaction. */
    enum class Interaction
    {
        dynamic, // interaction is enabled dynamically by tap-and-hold (default)
        on,      // force interaction with content
        off      // never interact with content
    };

private:
    /** Get the interaction policy (default: AUTO). */
    virtual Interaction _getInteractionPolicy() const;

    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        // clang-format off
        ar & _uri;
        ar & _size.rwidth();
        ar & _size.rheight();
        ar & _zoomRect;
        ar & _captureInteraction;
        // clang-format on
    }

    /** Serialize for saving to an xml file */
    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int version)
    {
        // clang-format off
        ar & boost::serialization::make_nvp("uri", _uri);
        ar & boost::serialization::make_nvp("width", _size.rwidth());
        ar & boost::serialization::make_nvp("height", _size.rheight());

        if (version < 2)
        {
            bool blockAdvance = false;
            ar & boost::serialization::make_nvp("block_advance", blockAdvance);
        }
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

    void _init();

    QString _uri;
    QSize _size;
    QRectF _zoomRect{UNIT_RECTF};
    deflect::SizeHints _sizeHints;
    bool _captureInteraction{false};

    static qreal _maxScale;
};

BOOST_CLASS_VERSION(Content, 2)

DECLARE_SERIALIZE_FOR_XML(Content)

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Content)

#endif
