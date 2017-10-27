/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE ScreenshotAssembler
#include <boost/test/unit_test.hpp>

#include "Configuration.h"
#include "ScreenshotAssembler.h"

#include "MinimalGlobalQtApp.h"
#include "imageCompare.h"

#define CONFIG_TEST_FILENAME "./configuration.xml"
#define REFERENCE_SCREENSHOT "./reference_screenshot.png"

// Needed for relative path to resources to work
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

BOOST_AUTO_TEST_CASE(test_assemble_screenshot)
{
    const Configuration config{CONFIG_TEST_FILENAME};
    ScreenshotAssembler assembler{config};

    QImage screenshot;
    assembler.connect(&assembler, &ScreenshotAssembler::screenshotComplete,
                      [&screenshot](const QImage image) {
                          screenshot = image;
                      });

    const QSize screenSize{config.getScreenWidth(), config.getScreenHeight()};
    const auto screenCount =
        config.getTotalScreenCountY() * config.getTotalScreenCountX();

    QImage screen{screenSize, QImage::Format_RGB32};

    for (auto y = 0; y < config.getTotalScreenCountY(); ++y)
    {
        for (auto x = 0; x < config.getTotalScreenCountX(); ++x)
        {
            screen.fill(QColor{x * 64, y * 64, 128});
            assembler.addImage(screen, {x, y});
            const auto index = x + y * config.getTotalScreenCountX();
            if (index < screenCount - 1)
                BOOST_CHECK(screenshot.isNull());
        }
    }

    BOOST_CHECK(!screenshot.isNull());
    BOOST_CHECK_EQUAL(screenshot.size(), config.getTotalSize());

    QImage reference;
    BOOST_REQUIRE(reference.load(REFERENCE_SCREENSHOT));
    BOOST_CHECK_LT(compareImages(screenshot, reference), 0.005);
}
