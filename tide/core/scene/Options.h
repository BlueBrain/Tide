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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "serialization/includes.h"
#include "types.h"

#include <QObject>

/**
 * Rendering options which can be changed during runtime.
 *
 * Can be serialized and distributed to the Wall applications, and also set as
 * a Qml context object.
 */
class Options : public QObject, public std::enable_shared_from_this<Options>
{
    Q_OBJECT
    Q_DISABLE_COPY(Options)

    Q_PROPERTY(bool alphaBlending READ isAlphaBlendingEnabled NOTIFY
                   alphaBlendingEnabledChanged)
    Q_PROPERTY(bool showClock READ getShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool showContentTiles READ getShowContentTiles NOTIFY
                   showContentTilesChanged)
    Q_PROPERTY(bool showControlArea READ getShowControlArea NOTIFY
                   showControlAreaChanged)
    Q_PROPERTY(
        bool showFilePaths READ getShowFilePaths NOTIFY showFilePathsChanged)
    Q_PROPERTY(
        bool showStatistics READ getShowStatistics NOTIFY showStatisticsChanged)
    Q_PROPERTY(bool showTouchPoints READ getShowTouchPoints NOTIFY
                   showTouchPointsChanged)
    Q_PROPERTY(bool showWindowBorders READ getShowWindowBorders NOTIFY
                   showWindowBordersChanged)
    Q_PROPERTY(bool showWindowTitles READ getShowWindowTitles NOTIFY
                   showWindowTitlesChanged)
    Q_PROPERTY(bool showZoomContext READ getShowZoomContext NOTIFY
                   showZoomContextChanged)

public:
    /** Create a shared Options object. */
    static OptionsPtr create() { return OptionsPtr{new Options()}; }
    /** @name QProperty getters */
    //@{
    bool isAlphaBlendingEnabled() const;
    bool getAutoFocusPixelStreams() const;
    bool getShowClock() const;
    bool getShowContentTiles() const;
    bool getShowControlArea() const;
    bool getShowFilePaths() const;
    bool getShowStatistics() const;
    bool getShowTestPattern() const;
    bool getShowTouchPoints() const;
    bool getShowWindowBorders() const;
    bool getShowWindowTitles() const;
    bool getShowZoomContext() const;
    //@}

public slots:
    /** @name QProperty setters. @see updated() */
    //@{
    void enableAlphaBlending(bool set);
    void setAutoFocusPixelStreams(bool set);
    void setShowClock(bool set);
    void setShowContentTiles(bool set);
    void setShowControlArea(bool set);
    void setShowFilePaths(bool set);
    void setShowStatistics(bool set);
    void setShowTestPattern(bool set);
    void setShowTouchPoints(bool set);
    void setShowWindowBorders(bool set);
    void setShowWindowTitles(bool set);
    void setShowZoomContext(bool set);
    //@}

signals:
    /** @name QProperty notifiers */
    //@{
    void alphaBlendingEnabledChanged(bool set);
    void autoFocusPixelStreamsChanged(bool set);
    void showContentTilesChanged(bool set);
    void showClockChanged(bool set);
    void showControlAreaChanged(bool set);
    void showFilePathsChanged(bool set);
    void showStatisticsChanged(bool set);
    void showTestPatternChanged(bool set);
    void showTouchPointsChanged(bool set);
    void showWindowBordersChanged(bool set);
    void showWindowTitlesChanged(bool set);
    void showZoomContextChanged(bool set);
    //@}

    /** Emitted when any value is changed by one of the setters. */
    void updated(OptionsPtr);

private:
    friend class boost::serialization::access;

    Options() = default;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _alphaBlendingEnabled;
        ar & _autoFocusPixelStreams;
        ar & _showClock;
        ar & _showContentTiles;
        ar & _showControlArea;
        ar & _showFilePaths;
        ar & _showStreamingStatistics;
        ar & _showTouchPoints;
        ar & _showTestPattern;
        ar & _showWindowBorders;
        ar & _showWindowTitles;
        ar & _showZoomContext;
        // clang-format on
    }

    bool _alphaBlendingEnabled = false;
    bool _autoFocusPixelStreams = true;
    bool _showClock = false;
    bool _showContentTiles = false;
    bool _showControlArea = true;
    bool _showFilePaths = false;
    bool _showStreamingStatistics = false;
    bool _showTestPattern = false;
    bool _showTouchPoints = true;
    bool _showWindowBorders = true;
    bool _showWindowTitles = true;
    bool _showZoomContext = true;
};

#endif
