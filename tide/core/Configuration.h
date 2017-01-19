/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QColor>
#include <QRectF>

#include "types.h"

class QXmlQuery;

/**
 * The Configuration class manages all the settings needed by the Tide
 * application.
 *
 * Both WallConfiguration and MasterConfiguation extend this class to provide
 * additional parameters specific to a Wall or Master process.
 *
 * In a typical DisplayWall setup, the "screens" are configured to map 1 to 1
 * with the XServer screens (which can extend to one or more physical displays).
 *
 * For testing purposes, the "screens" can also be represented as windows on a
 * single display.
 *
 * @note For historic reasons, both the words "screen" and "tile" are used in
 * xml configuration files and have the same meaning.
 *
 * @warning: this class can only be used AFTER creating a QApplication.
 */
class Configuration
{
public:
    /**
     * Create a new configuration from an xml file
     * @param filename path to the xml configuration file
     * @throw std::runtime_error if the file could not be read
     */
    explicit Configuration( const QString& filename );

    /** Destructor. */
    virtual ~Configuration() {}

    /** Get the filename passed to the constructor. */
    const QString& getFilename() const;

    /** Get the total number of screens along the x axis. */
    int getTotalScreenCountX() const;

    /** Get the total number of screens along the y axis. */
    int getTotalScreenCountY() const;

    /**
     * Get the width of a screen.
     * @return width in pixel units
     * @note All the screens have the same size.
     */
    int getScreenWidth() const;

    /**
     * Get the height of a screen.
     * @return height in pixel units
     * @note All the screens have the same size.
     */
    int getScreenHeight() const;

    /**
     * Get the padding nedded to compensate for the physical displays' bezel.
     * @return horizontal padding between two screens in pixel units
     */
    int getMullionWidth() const;

    /**
     * Get the padding nedded to compensate for the physical displays' bezel.
     * @return vertical padding between two screens in pixel units
     */
    int getMullionHeight() const;

    /**
     * Get the total width of the DisplayWall, including the Mullion padding.
     * @return width in pixel units
     */
    int getTotalWidth() const;

    /**
     * Get the total height of the DisplayWall, including the Mullion padding.
     * @return height in pixel units
     */
    int getTotalHeight() const;

    /** Get the total size of the DisplayWall, including Mullion padding. */
    QSize getTotalSize() const;

    /** Get the aspect ratio of the DisplayWall, including Mullion padding. */
    double getAspectRatio() const;

    /** Get the coordinates and dimensions of a screen in pixel units. */
    QRect getScreenRect( const QPoint& tileIndex ) const;

    /** Display the windows in fullscreen mode. */
    bool getFullscreen() const;

protected:
    /** The path to the xml configuration file. */
    QString _filename;

    /** Evaluate the querry and set the result to value on success. */
    bool getDouble( const QXmlQuery& query, double& value ) const;
    bool getInt( const QXmlQuery& query, int& value ) const;
    bool getBool( const QXmlQuery& query, bool& value ) const;

private:
    int _totalScreenCountX;
    int _totalScreenCountY;
    int _screenWidth;
    int _screenHeight;
    int _mullionWidth;
    int _mullionHeight;
    bool _fullscreen;

    void _load();
};

#endif
