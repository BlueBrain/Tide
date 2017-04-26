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

#define BOOST_TEST_MODULE WebBrowser

#include <boost/test/unit_test.hpp>

#include "localstreamer/WebkitHtmlSelectReplacer.h"
#include "types.h"

#include <QDebug>
#include <QDir>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>

#include "GlobalQtApp.h"

#define TEST_PAGE_URL "/select_test.htm"
#define HTTP_BODY_SELECTOR "body"
#define HTTP_SELECT_SELECTOR "select[id=language]"
#define HTTP_SELECTBOXIT_SELECTOR "span[id=languageSelectBoxIt]"
#define DISPLAY_STYLE_PROPERTY_NAME "display"
#define DISPLAY_STYLE_NONE "none"

BOOST_GLOBAL_FIXTURE(GlobalQtApp);

QString testPageURL()
{
    return "file://" + QDir::currentPath() + TEST_PAGE_URL;
}

class TestPage
{
public:
    TestPage()
    {
        webview.page()->setViewportSize(QSize(640, 480));
        QObject::connect(&webview, SIGNAL(loadFinished(bool)),
                         QApplication::instance(), SLOT(quit()));
    }

    void load()
    {
        webview.load(QUrl(testPageURL()));
        QApplication::instance()->exec();

        // Check that the page could be loaded
        const QString pageContent = getElement(HTTP_BODY_SELECTOR).toInnerXml();
        BOOST_REQUIRE(!pageContent.isEmpty());
    }

    QWebElement getElement(const QString& selectorQuery) const
    {
        return webview.page()->mainFrame()->findFirstElement(selectorQuery);
    }

    QString getSelectElementDisplayProperty() const
    {
        const QWebElement select = getElement(HTTP_SELECT_SELECTOR);
        BOOST_REQUIRE(!select.isNull());
        return select.styleProperty(DISPLAY_STYLE_PROPERTY_NAME,
                                    QWebElement::InlineStyle);
    }

    QWebView webview;
};

BOOST_AUTO_TEST_CASE(TestWhenNoReplacerThenSelectElementIsVisible)
{
    if (!hasGLXDisplay())
        return;

    TestPage testPage;
    testPage.load();

    const QString displayStyleProperty =
        testPage.getSelectElementDisplayProperty();
    BOOST_CHECK(displayStyleProperty.isEmpty());
}

BOOST_AUTO_TEST_CASE(TestWhenReplacerThenSelectHasEquivalentHtml)
{
    if (!hasGLXDisplay())
        return;

    TestPage testPage;
    WebkitHtmlSelectReplacer replacer(testPage.webview);
    testPage.load();

    BOOST_CHECK_EQUAL(testPage.getSelectElementDisplayProperty(),
                      DISPLAY_STYLE_NONE);

    QWebElement selectboxit = testPage.getElement(HTTP_SELECTBOXIT_SELECTOR);
    BOOST_CHECK(!selectboxit.isNull());
}
