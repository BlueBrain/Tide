/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#define BOOST_TEST_MODULE LoggingUtilityTest

#include <boost/make_shared.hpp>
#include <boost/regex.hpp>
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "DisplayGroup.h"
#include "DummyContent.h"
#include "LoggingUtility.h"
#include "rest/RestLogger.h"

#include <QObject>

namespace
{
const QSize wallSize( 1000, 1000 );
const std::string regexJson{
R"(\{
    "event": \{
        "count": 2,
        "last_event": "contentWindowAdded",
        "last_event_date": "\d{4}-\d{2}-\d{2}\u\d{2}:\d{2}:\d{2}.\d{6}"
    \},
    "window": \{
        "accumulated_count": 2,
        "count": 2,
        "date_set": "\d{4}-\d{2}-\d{2}\u\d{2}:\d{2}:\d{2}.\d{6}"
    \}
\}
)"
};
const std::string defaultJson{
R"({
    "event": {
        "count": 0,
        "last_event": "",
        "last_event_date": ""
    },
    "window": {
        "accumulated_count": 0,
        "count": 0,
        "date_set": ""
    }
}
)"
};
}

BOOST_AUTO_TEST_CASE( testAccumulatedWindowCount )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    
    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window2 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();

    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowAdded,
                      logger.get(), &LoggingUtility::contentWindowAdded );

    BOOST_CHECK( logger.get()->getAccumulatedWindowCount() == 0);
    displayGroup->addContentWindow( window1 );
    BOOST_CHECK( logger.get()->getAccumulatedWindowCount() == 1);
    
    displayGroup->addContentWindow( window2 );
    BOOST_CHECK( logger.get()->getAccumulatedWindowCount() == 2);

}

BOOST_AUTO_TEST_CASE( testWindowCount )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();

    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowAdded,
                      logger.get(), &LoggingUtility::contentWindowAdded );
    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowRemoved,
                      logger.get(), &LoggingUtility::contentWindowRemoved );
    BOOST_CHECK( logger.get()->getWindowCount() == 0);
    displayGroup->addContentWindow( window1 );
    BOOST_CHECK( logger.get()->getWindowCount() == 1);
    displayGroup->removeContentWindow( window1 );
    BOOST_CHECK( logger.get()->getWindowCount() == 0);
}

BOOST_AUTO_TEST_CASE( testWindowCountDecrement )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();

    BOOST_CHECK( logger.get()->getWindowCount() == 0);
    displayGroup->addContentWindow( window1 );
    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowRemoved,
                      logger.get(), &LoggingUtility::contentWindowRemoved );
    displayGroup->removeContentWindow( window1 );
    BOOST_CHECK( logger.get()->getWindowCount() == 0);
}

BOOST_AUTO_TEST_CASE( testInteractionCounter )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window2 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();

    BOOST_CHECK( logger.get()->getInteractionCount() == 0);

    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowAdded,
                      logger.get(), &LoggingUtility::contentWindowAdded );
 
    displayGroup->addContentWindow( window1 );
    displayGroup->focus( window1->getID());
    BOOST_CHECK( logger.get()->getInteractionCount() == 2);

}

BOOST_AUTO_TEST_CASE( testLastInteraction )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    
    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();
    BOOST_CHECK( logger.get()->getLastInteraction() == "");

    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowAdded,
                      logger.get(), &LoggingUtility::contentWindowAdded );
 
    displayGroup->addContentWindow( window1 );
    displayGroup->focus( window1->getID());
    BOOST_CHECK( logger.get()->getLastInteraction() == "mode changed");

}

BOOST_AUTO_TEST_CASE( testJsonOutput )
{
    ContentPtr content( new DummyContent );
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window2 = boost::make_shared<ContentWindow>( content );
    std::unique_ptr<LoggingUtility> logger = make_unique<LoggingUtility>();

    QObject::connect( displayGroup.get(), &DisplayGroup::contentWindowAdded,
                      logger.get(), &LoggingUtility::contentWindowAdded );
 
    std::unique_ptr<RestLogger> logContent;
    logContent.reset( new RestLogger( *(logger.get()) ));
    BOOST_CHECK( logContent->toJSON() == defaultJson );

    displayGroup->addContentWindow( window1 );
    displayGroup->addContentWindow( window2 );

    std::string logText =  logContent->toJSON();

    boost::regex ip_regex(regexJson);
    boost::sregex_iterator it(logText.begin(), logText.end(), ip_regex);
    boost::sregex_iterator end;

    std::string outputJson;
    for (; it != end; ++it)
        outputJson.append( it->str());

    BOOST_CHECK( logContent->toJSON() == outputJson  );

}
