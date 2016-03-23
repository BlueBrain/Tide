/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#ifndef STATEPREVIEW_H
#define STATEPREVIEW_H

#include <QString>
#include <QSize>
#include <QImage>
#include "types.h"

class ContentWindow;

/**
 * A state preview is a thumbnail image saved alongside a state file.
 * It uses the same filename but a specific file extension
 * to differenciate it from other media in the folder.
 */
class StatePreview
{
public:
    /**
     * Constructor.
     * @param dcxFileName The state file associated with this preview image.
     *        Needed by loadFromFile() and saveToFile().
     */
    StatePreview( const QString& dcxFileName );

    /** Get the file extension used for state images. */
    static QString getFileExtension();

    /**
     * Load the thumbnail image from disk.
     */
    bool loadFromFile();

    /**
     * Retrieve the image loaded with loadFromFile() or generated with
     * generateImage().
     */
    QImage getImage() const;

    /**
     * Generate the preview image from a list of ContentWindows.
     * @param wallDimensions the total dimensions of the wall in pixels, used to
     *        position the contents.
     * @param contentWindows the contents to include in the preview.
     */
    void generateImage( const QSize& wallDimensions,
                        const ContentWindowPtrs& contentWindows );

    /**
     * Save the thumbnail created by generateImage() to a file.
     * The filename is the same as the state filename but uses a different
     * extension.
     * @see getFileExtension()
     */
    bool saveToFile() const;

protected:
    QString previewFilename() const;

    QString dcxFileName_;
    QImage previewImage_;
};

#endif // STATEPREVIEW_H
