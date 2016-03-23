/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#ifndef DOCKTOOLBAR_H
#define DOCKTOOLBAR_H

#include <QImage>

/**
 * A simple button for the DockToolbar.
 */
struct ToolbarButton
{
    /** Text caption. */
    QString caption;
    /** Icon image, should be square and have at least the same height as the Toolbar to avoid upscaling. */
    QImage icon;
    /** User-defined command associated with this button. */
    QString command;

    /** Constructor */
    ToolbarButton(QString captionParam, QImage iconParam, QString commandParam)
        : caption(captionParam), icon(iconParam), command(commandParam) {}
};

/**
 * A Toolbar for the Dock.
 *
 * Renders a list of buttons layed out horizontally with an icon and text caption.
 */
class DockToolbar
{
public:
    /**
     * Constructor
     * @param size The size of the Toolbar in pixels.
     */
    DockToolbar(const QSize size);

    /** Destructor. */
    ~DockToolbar();

    /**
     * Add a button to the right of the Toolbar.
     * @param button The button to add. This class takes ownership of the object.
     */
    void addButton(ToolbarButton* button);

    /** Get the size in pixels. */
    QSize getSize() const;

    /** Get the image, regenerating it if required. */
    const QImage& getImage() const;

    /**
     * Get the button at the given position.
     * @param pos A position in pixels inside the toolbar.
     * @return A pointer to the button, or a nullptr if there is no button at the given position.
     */
    const ToolbarButton* getButtonAt(const QPoint& pos) const;

private:
    QRect area_;
    QList<ToolbarButton*> buttons_;
    mutable QImage image_;
    mutable bool needsUpdate_;

    void render(QImage& buffer) const;
    void drawButton(QPainter& painter, const ToolbarButton& button, const int index) const;
};

#endif // DOCKTOOLBAR_H
