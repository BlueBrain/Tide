/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#ifndef CONTENTLOADER_H
#define CONTENTLOADER_H

#include "types.h"

#include <QString>
#include <QPointF>
#include <QSizeF>

/**
 * Helper class to open Content on a DisplayGroup.
 */
class ContentLoader
{
public:
    /**
     * Constructor.
     *
     * @param displayGroup The target DisplayGroup for displaying the content.
     */
    ContentLoader( DisplayGroupPtr displayGroup );

    /**
     * Load a Content from a file and create a window for it.
     *
     * @param filename The content file to open.
     * @param windowCenterPosition The point around which to center the window.
     *        If empty (default), the  window is automatically centered on the
     *        displayWall.
     * @param windowSize The size of the window. If empty, the size of the
     *        window is automatically adjusted to its content dimensions.
     * @return true if operation was successful, false otherwise.
     */
    bool load( const QString& filename,
               const QPointF& windowCenterPosition = QPointF(),
               const QSizeF& windowSize = QSizeF( ));

    /**
     * Check if a content is already open.
     *
     * @param filename The content file to search for.
     * @return true if a content with the same uri is already open.
     */
    bool isAlreadyOpen( const QString& filename ) const;

private:
    DisplayGroupPtr _displayGroup;
};

#endif
