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

#include "BackgroundWidget.h"

#include <QtWidgets>

#include "MasterConfiguration.h"
#include "ContentFactory.h"

BackgroundWidget::BackgroundWidget( MasterConfiguration& configuration,
                                    QWidget* parent_ )
    : QDialog( parent_ )
    , _configuration( configuration )
    , _backgroundFolder( configuration.getContentDir( ))
{
    setWindowTitle( tr( "Background settings" ));

    const int frameStyle = QFrame::Sunken | QFrame::Panel;

    // Get current variables

    _previousColor = _configuration.getBackgroundColor();
    _previousBackgroundURI = _configuration.getBackgroundUri();

    // Color chooser

    _colorLabel = new QLabel( _previousColor.name( ));
    _colorLabel->setFrameStyle( frameStyle );
    _colorLabel->setPalette( QPalette( _previousColor ));
    _colorLabel->setAutoFillBackground( true );

    QPushButton* colorButton = new QPushButton(
                                   tr( "Choose background color..." ));
    connect( colorButton, SIGNAL( clicked( )), this, SLOT( _chooseColor( )));


    // Background chooser

    _backgroundLabel = new QLabel( _previousBackgroundURI );
    _backgroundLabel->setFrameStyle( frameStyle );
    QPushButton* backgroundButton = new QPushButton(
                                        tr( "Choose background content..." ));
    connect( backgroundButton, SIGNAL( clicked( )),
             this, SLOT( _openBackgroundContent( )));

    QPushButton* backgroundClearButton = new QPushButton(
                                             tr( "Remove background" ));
    connect( backgroundClearButton, SIGNAL( clicked( )),
             this, SLOT( _removeBackground( )));


    // Standard buttons

    typedef QDialogButtonBox::StandardButton button;
    QDialogButtonBox* buttonBox = new QDialogButtonBox( button::Ok |
                                                        button::Cancel,
                                                        Qt::Horizontal, this );

    connect( buttonBox, SIGNAL( accepted( )), this, SLOT( accept( )));
    connect( buttonBox, SIGNAL( rejected( )), this, SLOT( reject( )));


    // Layout

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setColumnStretch( 1, 1 );
    gridLayout->setColumnMinimumWidth( 1, 250 );
    setLayout( gridLayout );

    gridLayout->addWidget( colorButton, 0, 0 );
    gridLayout->addWidget( _colorLabel, 0, 1 );
    gridLayout->addWidget( backgroundButton, 1, 0 );
    gridLayout->addWidget( _backgroundLabel, 1, 1 );
    gridLayout->addWidget( backgroundClearButton, 2, 0 );
    gridLayout->addWidget( buttonBox, 2, 1 );
}

void BackgroundWidget::accept()
{
    if( _configuration.save( ))
    {
        _previousColor = _configuration.getBackgroundColor();
        _previousBackgroundURI = _configuration.getBackgroundUri();

        QDialog::accept();
    }
    else
    {
        QMessageBox messageBox;
        messageBox.setText( "An error occured while saving the configuration "\
                            "xml file. Changes cannot be saved." );
        messageBox.exec();
    }
}

void BackgroundWidget::reject()
{
    // Revert to saved settings
    _colorLabel->setText( _previousColor.name( ));
    _colorLabel->setPalette( QPalette( _previousColor ));
    _backgroundLabel->setText( _previousBackgroundURI );

    _configuration.setBackgroundColor( _previousColor );
    _configuration.setBackgroundUri( _previousBackgroundURI );

    ContentPtr content;
    if( !_previousBackgroundURI.isEmpty( ))
        content = ContentFactory::getContent( _previousBackgroundURI );

    emit backgroundContentChanged( content );
    emit backgroundColorChanged( _previousColor );

    QDialog::reject();
}

void BackgroundWidget::_chooseColor()
{
    QColor color = QColorDialog::getColor( Qt::green, this );

    if( !color.isValid( ))
        return;

    _colorLabel->setText( color.name());
    _colorLabel->setPalette( QPalette( color ));

    _configuration.setBackgroundColor( color );
    emit backgroundColorChanged( color );
}

void BackgroundWidget::_openBackgroundContent()
{
    const QString filter = ContentFactory::getSupportedFilesFilterAsString();
    const QString filename = QFileDialog::getOpenFileName( this,
                                                           tr("Choose content"),
                                                           _backgroundFolder,
                                                           filter );
    if( filename.isEmpty( ))
        return;

    _backgroundFolder = QFileInfo( filename ).absoluteDir().path();

    ContentPtr content = ContentFactory::getContent( filename );
    if( content )
    {
        _backgroundLabel->setText( filename );
        _configuration.setBackgroundUri( filename );
        emit backgroundContentChanged( content );
    }
    else
    {
        QMessageBox messageBox;
        messageBox.setText( tr( "Error: Unsupported file." ));
        messageBox.exec();
    }
}

void BackgroundWidget::_removeBackground()
{
    _backgroundLabel->setText( "" );
    _configuration.setBackgroundUri( "" );
    emit backgroundContentChanged( ContentPtr( ));
}
