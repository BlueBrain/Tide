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
    , configuration_( configuration )
    , backgroundFolder_( configuration.getDockStartDir( ))
{
    setWindowTitle( tr( "Background settings" ));

    const int frameStyle = QFrame::Sunken | QFrame::Panel;

    // Get current variables

    previousColor_ = configuration_.getBackgroundColor();
    previousBackgroundURI_ = configuration_.getBackgroundUri();

    // Color chooser

    colorLabel_ = new QLabel( previousColor_.name( ));
    colorLabel_->setFrameStyle( frameStyle );
    colorLabel_->setPalette( QPalette( previousColor_ ));
    colorLabel_->setAutoFillBackground( true );

    QPushButton* colorButton = new QPushButton(
                                   tr( "Choose background color..." ));
    connect( colorButton, SIGNAL( clicked( )), this, SLOT( chooseColor( )));


    // Background chooser

    backgroundLabel_ = new QLabel( previousBackgroundURI_ );
    backgroundLabel_->setFrameStyle( frameStyle );
    QPushButton* backgroundButton = new QPushButton(
                                        tr( "Choose background content..." ));
    connect( backgroundButton, SIGNAL( clicked( )),
             this, SLOT( openBackgroundContent( )));

    QPushButton* backgroundClearButton = new QPushButton(
                                             tr( "Remove background" ));
    connect( backgroundClearButton, SIGNAL( clicked( )),
             this, SLOT( removeBackground( )));


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
    gridLayout->addWidget( colorLabel_, 0, 1 );
    gridLayout->addWidget( backgroundButton, 1, 0 );
    gridLayout->addWidget( backgroundLabel_, 1, 1 );
    gridLayout->addWidget( backgroundClearButton, 2, 0 );
    gridLayout->addWidget( buttonBox, 2, 1 );
}

void BackgroundWidget::accept()
{
    if( configuration_.save( ))
    {
        previousColor_ = configuration_.getBackgroundColor();
        previousBackgroundURI_ = configuration_.getBackgroundUri();

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
    colorLabel_->setText( previousColor_.name( ));
    colorLabel_->setPalette( QPalette( previousColor_ ));
    backgroundLabel_->setText( previousBackgroundURI_ );

    configuration_.setBackgroundColor( previousColor_ );
    configuration_.setBackgroundUri( previousBackgroundURI_ );

    ContentPtr content;
    if( !previousBackgroundURI_.isEmpty( ))
        content = ContentFactory::getContent( previousBackgroundURI_ );

    emit backgroundContentChanged( content );
    emit backgroundColorChanged( previousColor_ );

    QDialog::reject();
}

void BackgroundWidget::chooseColor()
{
    QColor color = QColorDialog::getColor( Qt::green, this );

    if( !color.isValid( ))
        return;

    colorLabel_->setText( color.name());
    colorLabel_->setPalette( QPalette( color ));

    configuration_.setBackgroundColor( color );
    emit backgroundColorChanged( color );
}

void BackgroundWidget::openBackgroundContent()
{
    const QString filter = ContentFactory::getSupportedFilesFilterAsString();
    const QString filename = QFileDialog::getOpenFileName( this,
                                                           tr("Choose content"),
                                                           backgroundFolder_,
                                                           filter );
    if( filename.isEmpty( ))
        return;

    backgroundFolder_ = QFileInfo( filename ).absoluteDir().path();

    ContentPtr content = ContentFactory::getContent( filename );
    if( content )
    {
        backgroundLabel_->setText( filename );
        configuration_.setBackgroundUri( filename );
        emit backgroundContentChanged( content );
    }
    else
    {
        QMessageBox messageBox;
        messageBox.setText( tr( "Error: Unsupported file." ));
        messageBox.exec();
    }
}

void BackgroundWidget::removeBackground()
{
    backgroundLabel_->setText( "" );
    configuration_.setBackgroundUri( "" );
    emit backgroundContentChanged( ContentPtr( ));
}
