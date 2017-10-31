/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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

#include "WebbrowserWidget.h"

#include "MasterConfiguration.h"

#include <QtWidgets>

#define SPIN_BOX_MIN_VALUE 0
#define SPIN_BOX_MAX_VALUE 10000

#define WEBBROWSER_DEFAULT_WIDTH 1280
#define WEBBROWSER_DEFAULT_HEIGHT 1024

WebbrowserWidget::WebbrowserWidget(const MasterConfiguration& config,
                                   QWidget* parent_)
    : QDialog(parent_)
    , _widthSpinBox(0)
    , _heightSpinBox(0)
{
    setWindowTitle(tr("Open Web Browser"));

    // URL input

    QLabel* urlLabel = new QLabel("Url: ", this);
    _urlLineEdit = new QLineEdit(config.getWebBrowserDefaultURL(), this);
    urlLabel->setBuddy(_urlLineEdit);

    // Standard buttons

    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Webbrowser dimensions

    QLabel* widthLabel = new QLabel("Width: ", this);

    _widthSpinBox = new QSpinBox(this);
    _widthSpinBox->setMinimum(SPIN_BOX_MIN_VALUE);
    _widthSpinBox->setMaximum(SPIN_BOX_MAX_VALUE);
    _widthSpinBox->setValue(WEBBROWSER_DEFAULT_WIDTH);

    QLabel* heightLabel = new QLabel("Height: ", this);

    _heightSpinBox = new QSpinBox(this);
    _heightSpinBox->setMinimum(SPIN_BOX_MIN_VALUE);
    _heightSpinBox->setMaximum(SPIN_BOX_MAX_VALUE);
    _heightSpinBox->setValue(WEBBROWSER_DEFAULT_HEIGHT);

    // Debug port

    QLabel* debugPortLabel = new QLabel("Debug port: ", this);

    _debugPortSpinBox = new QSpinBox(this);
    _debugPortSpinBox->setMinimum(0);
    _debugPortSpinBox->setMaximum(65535);

    // Layout

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnMinimumWidth(1, 250);
    setLayout(gridLayout);

    gridLayout->addWidget(urlLabel, 0, 0);
    gridLayout->addWidget(_urlLineEdit, 0, 1);

    gridLayout->addWidget(widthLabel, 1, 0);
    gridLayout->addWidget(_widthSpinBox, 1, 1);

    gridLayout->addWidget(heightLabel, 2, 0);
    gridLayout->addWidget(_heightSpinBox, 2, 1);

    gridLayout->addWidget(debugPortLabel, 3, 0);
    gridLayout->addWidget(_debugPortSpinBox, 3, 1);

    gridLayout->addWidget(buttonBox, 4, 1);
}

void WebbrowserWidget::accept()
{
    const QSize dimensions(_widthSpinBox->value(), _heightSpinBox->value());
    const auto debugPort = (ushort)_debugPortSpinBox->value();
    emit openWebBrowser(QPointF(), dimensions, _urlLineEdit->text(), debugPort);

    QDialog::accept();
}
