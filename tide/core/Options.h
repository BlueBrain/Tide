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
 * Rendering options which can be changed during runtime.
 *
 * Can be serialized and distributed to the Wall applications, and also set as
 * a Qml context object.
 */
class Options : public QObject, public boost::enable_shared_from_this<Options>
{
    Q_OBJECT
    Q_PROPERTY( bool alphaBlending READ isAlphaBlendingEnabled
                NOTIFY alphaBlendingEnabledChanged )
    Q_PROPERTY( bool showClock READ getShowClock NOTIFY showClockChanged )
    Q_PROPERTY( bool showContentTiles READ getShowContentTiles
                NOTIFY showContentTilesChanged )
    Q_PROPERTY( bool showControlArea READ getShowControlArea
                NOTIFY showControlAreaChanged )
    Q_PROPERTY( bool showStatistics READ getShowStatistics
                NOTIFY showStatisticsChanged )
    Q_PROPERTY( bool showTouchPoints READ getShowTouchPoints
                NOTIFY showTouchPointsChanged )
    Q_PROPERTY( bool showWindowBorders READ getShowWindowBorders
                NOTIFY showWindowBordersChanged )
    Q_PROPERTY( bool showZoomContext READ getShowZoomContext
                NOTIFY showZoomContextChanged )

public:
    /** Default constructor */
    Options();

    /** @name QProperty getters */
    //@{
    bool isAlphaBlendingEnabled() const;
    bool getAutoFocusPixelStreams() const;
    bool getShowClock() const;
    bool getShowContentTiles() const;
    bool getShowControlArea() const;
    bool getShowStatistics() const;
    bool getShowTestPattern() const;
    bool getShowTouchPoints() const;
    bool getShowWindowBorders() const;
    bool getShowZoomContext() const;
    //@}

    /** @name Background settings */
    //@{
    QColor getBackgroundColor() const;
    ContentPtr getBackgroundContent() const;
    QString getBackgroundUri() const;
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
    /** @name QProperty setters. @see updated() */
    //@{
    void enableAlphaBlending( bool set );
    void setAutoFocusPixelStreams( bool set );
    void setShowClock( bool set );
    void setShowContentTiles( bool set );
    void setShowControlArea( bool set );
    void setShowStatistics( bool set );
    void setShowTestPattern( bool set );
    void setShowTouchPoints( bool set );
    void setShowWindowBorders( bool set );
    void setShowZoomContext( bool set );
    //@}

    /** @name Background settings. @see updated(). */
    //@{
    /** Set the color of the background. */
    void setBackgroundColor( QColor color );

    /**
     * Set the background content.
     * @param content The content to set.
     *        A null pointer removes the current background.
     */
    void setBackgroundContent( ContentPtr content );

    /**
     * Set the background content from a uri.
     * @param uri The uri of the content to set. If the uri is invalid, the
     *        content is not modified. An empty uri removes the content.
     */
    void setBackgroundUri( const QString& uri );
    //@}

signals:
    /** @name QProperty notifiers */
    //@{
    void alphaBlendingEnabledChanged( bool set );
    void autoFocusPixelStreamsChanged( bool set );
    void showContentTilesChanged( bool set );
    void showClockChanged( bool set );
    void showControlAreaChanged( bool set );
    void showStatisticsChanged( bool set );
    void showTestPatternChanged( bool set );
    void showTouchPointsChanged( bool set );
    void showWindowBordersChanged( bool set );
    void showZoomContextChanged( bool set );
    //@}

    /** Emitted when any value is changed by one of the setters. */
    void updated( OptionsPtr );

private:
    Q_DISABLE_COPY( Options )

    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive & ar, const unsigned int )
    {
        ar & _alphaBlendingEnabled;
        ar & _autoFocusPixelStreams;
        ar & _showClock;
        ar & _showContentTiles;
        ar & _showControlArea;
        ar & _showStreamingStatistics;
        ar & _showTouchPoints;
        ar & _showTestPattern;
        ar & _showWindowBorders;
        ar & _showZoomContext;
        ar & _backgroundColor;
        ar & _backgroundContent;
    }

    bool _alphaBlendingEnabled;
    bool _autoFocusPixelStreams;
    bool _showClock;
    bool _showContentTiles;
    bool _showControlArea;
    bool _showStreamingStatistics;
    bool _showTestPattern;
    bool _showTouchPoints;
    bool _showWindowBorders;
    bool _showZoomContext;
    QColor _backgroundColor;
    ContentPtr _backgroundContent;
};

#endif
