/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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


#define BOOST_TEST_MODULE QmlControlPanelTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "types.h"
#include "QmlControlPanel.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp );


namespace
{
const QPointF expectedContentPanelPos( 100.0, 200.0 );
const QPointF expectedApplicationPanelPos( 200.0, 200.0 );
const QPointF expectedLoadSessionPanelPos( 300.0, 200.0 );
const QPointF expectedSaveSessionPanelPos( 400.0, 200.0 );
}

BOOST_AUTO_TEST_CASE( processActions )
{
    QPointF openContentPanelPos;
    QPointF openApplicationPanelPos;
    bool clearSessionCalled = false;
    QPointF openLoadSessionPanelPos;
    QPointF openSaveSessionPanelPos;

    QmlControlPanel panel;
    panel.connect( &panel, &QmlControlPanel::openContentPanel,
                   [&]( QPointF pos ) { openContentPanelPos = pos; }
    );
    panel.connect( &panel, &QmlControlPanel::openApplicationsPanel,
                   [&]( QPointF pos ) { openApplicationPanelPos = pos; }
    );
    panel.connect( &panel, &QmlControlPanel::clearSession,
                   [&]() { clearSessionCalled = true; }
    );
    panel.connect( &panel, &QmlControlPanel::openLoadSessionPanel,
                   [&]( QPointF pos ) { openLoadSessionPanelPos = pos; }
    );
    panel.connect( &panel, &QmlControlPanel::openSaveSessionPanel,
                   [&]( QPointF pos ) { openSaveSessionPanelPos = pos; }
    );

    panel.processAction( QmlControlPanel::OPEN_CONTENT,
                         expectedContentPanelPos );
    panel.processAction( QmlControlPanel::OPEN_APPLICATION,
                         expectedApplicationPanelPos );
    panel.processAction( QmlControlPanel::NEW_SESSION, QPointF( ));
    panel.processAction( QmlControlPanel::LOAD_SESSION,
                         expectedLoadSessionPanelPos );
    panel.processAction( QmlControlPanel::SAVE_SESSION,
                         expectedSaveSessionPanelPos );

    BOOST_CHECK_EQUAL( openContentPanelPos, expectedContentPanelPos );
    BOOST_CHECK_EQUAL( openApplicationPanelPos, expectedApplicationPanelPos );
    BOOST_CHECK( clearSessionCalled );
    BOOST_CHECK_EQUAL( openLoadSessionPanelPos, expectedLoadSessionPanelPos );
    BOOST_CHECK_EQUAL( openSaveSessionPanelPos, expectedSaveSessionPanelPos );
}
