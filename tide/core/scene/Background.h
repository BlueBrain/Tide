/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Raphael.Dumusc@epfl.ch                        */
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

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Content.h"
#include "serialization/includes.h"
#include "types.h"

#include <QColor>
#include <QObject>

/**
 * Background color and content.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class Background : public QObject,
                   public std::enable_shared_from_this<Background>
{
    Q_OBJECT
    Q_DISABLE_COPY(Background)

public:
    /** Create a shared Options object. */
    static BackgroundPtr create() { return BackgroundPtr{new Background()}; }
    /** Destructor. */
    ~Background();

    /** Get the color of the background. */
    QColor getColor() const;

    /** Get the text to display on the background. */
    QString getText() const;

    /** Get the uri of the background content. */
    QString getUri() const;

    /** Set the color of the background. */
    void setColor(QColor color);

    /** Set the text to display on the background. */
    void setText(const QString& text);

    /**
     * Set the background content from a uri.
     * @param uri The uri of the content to set. If the uri is invalid, the
     *        content is not modified. An empty uri removes the content.
     */
    void setUri(const QString& uri);

    /** @name Accessors for internal properties, used on the wall only. */
    //@{
    const Content* getContent() const;
    const QUuid& getContentUUID() const;
    //@}

signals:
    /** Emitted when any value is changed by one of the setters. */
    void updated(BackgroundPtr);

private:
    friend class boost::serialization::access;

    /** Default constructor */
    Background() = default;

    void _setContent(ContentPtr content);

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _color;
        ar & _text;
        ar & _content;
        if (_content)
            _content->setParent(this);
        ar & _contentID;
        // clang-format on
    }

    QColor _color;
    QString _text;
    ContentPtr _content;
    QUuid _contentID = QUuid::createUuid();
};

#endif
