/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#ifndef SESSION_SAVER_H
#define SESSION_SAVER_H

#include <QString>
#include <QtConcurrent>

#include "types.h"

/**
 * Helper class to save a scene to a file.
 */
class SessionSaver
{
public:
    /**
     * Constructor
     *
     * @param scene The scene to be saved.
     * @param tmpDir The folder where temporary documents (uploaded via web
     *        interface) are stored, used for relocating them to the uploadDir.
     * @param uploadDir folder to move temporary documents to.
     */
    SessionSaver(ScenePtr scene, const QString& tmpDir = QString(),
                 const QString& uploadDir = QString());

    /**
     * Save the scene to a file.
     *
     * @param filename The .dcx file to save the scene. The extension will be
     *        automatically added if it is missing.
     * @param generatePreview Also generate a .dcxpreview thumbnail image.
     */
    QFuture<bool> save(QString filename, bool generatePreview = true);

    /**
     * Find an available filename by appending "_[n]" if needed to the filename.
     *
     * @param filename the desired filename.
     * @param dstDir the target directory.
     * @return the full path to the next filename that is available.
     */
    static QString findAvailableFilePath(const QString& filename,
                                         const QString& dstDir);

private:
    ScenePtr _scene;
    QString _tmpDir;
    QString _uploadDir;
};

#endif
