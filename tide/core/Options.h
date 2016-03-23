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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"
#include "serializationHelpers.h"

#include <QObject>
#include <QColor>
#include <boost/serialization/access.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * Stores global display Options which can change during runtime.
 *
 * Can be serialized and distributed to the Wall applications, and also set as
 * a Qml context object.
 */
class Options : public QObject, public boost::enable_shared_from_this<Options>
{
    Q_OBJECT
    Q_PROPERTY( bool showClock READ getShowClock CONSTANT )
    Q_PROPERTY( bool showTouchPoints READ getShowTouchPoints CONSTANT )
    Q_PROPERTY( bool showStatistics READ getShowStatistics CONSTANT )
    Q_PROPERTY( bool showContentTiles READ getShowContentTiles CONSTANT )
    Q_PROPERTY( bool showWindowBorders READ getShowWindowBorders NOTIFY showWindowBordersChanged )
    Q_PROPERTY( bool showZoomContext READ getShowZoomContext CONSTANT )
    Q_PROPERTY( bool showControlArea READ getShowControlArea NOTIFY showControlAreaChanged )
    Q_PROPERTY( bool alphaBlending READ isAlphaBlendingEnabled CONSTANT )

public:
    /** Constructor */
    Options();

    /** @name Public getters */
    //@{
    bool getShowClock() const;
    bool getShowWindowBorders() const;
    bool getShowTouchPoints() const;
    bool getShowTestPattern() const;
    bool getShowZoomContext() const;
    bool getShowContentTiles() const;
    bool getShowStatistics() const;
    bool getShowControlArea() const;
    bool isAlphaBlendingEnabled() const;
    QColor getBackgroundColor() const;
    ContentPtr getBackgroundContent() const;
    //@}

    /**
     * Move this object and its member QObjects to the given QThread.
     *
     * This intentionally shadows the default QObject::moveToThread to include
     * member QObjects which are stored using shared_ptr and thus can't be made
     * direct children of this class.
     * @param thread the target thread.
     */
    void moveToThread( QThread* thread );

public slots:
    /** @name Public setters. @see updated() */
    //@{
    void setShowClock( bool set );
    void setShowWindowBorders( bool set );
    void setShowTouchPoints( bool set );
    void setShowTestPattern( bool set );
    void setShowZoomContext( bool set );
    void setShowContentTiles( bool set );
    void setShowStatistics( bool set );
    void setShowControlArea( bool set );
    void enableAlphaBlending( bool set );
    void setBackgroundColor( QColor color );

    /**
     * Set the background content.
     * @param content The content to set.
     *        A null pointer removes the current background.
     */
    void setBackgroundContent( ContentPtr content );
    //@}

signals:
    /** Emitted when a value is changed by one of the setters. */
    void updated( OptionsPtr );

    /** @name QProperty notifiers */
    //@{
    void showWindowBordersChanged();
    void showControlAreaChanged();
    //@}

private:
    Q_DISABLE_COPY( Options )

    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive & ar, const unsigned int )
    {
        ar & showClock_;
        ar & showWindowBorders_;
        ar & showTouchPoints_;
        ar & showTestPattern_;
        ar & showZoomContext_;
        ar & showContentTiles_;
        ar & showStreamingStatistics_;
        ar & showControlArea_;
        ar & alphaBlendingEnabled_;
        ar & backgroundColor_;
        ar & backgroundContent_;
    }

    bool showClock_;
    bool showWindowBorders_;
    bool showTouchPoints_;
    bool showTestPattern_;
    bool showZoomContext_;
    bool showContentTiles_;
    bool showStreamingStatistics_;
    bool showControlArea_;
    bool alphaBlendingEnabled_;
    QColor backgroundColor_;
    ContentPtr backgroundContent_;
};

#endif
