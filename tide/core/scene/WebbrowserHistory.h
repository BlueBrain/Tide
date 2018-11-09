/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef WEBBROWSERHISTORY_H
#define WEBBROWSERHISTORY_H

#include "serialization/includes.h"

/**
 * A serializable navigation history.
 */
class WebbrowserHistory
{
public:
    WebbrowserHistory() = default;
    WebbrowserHistory(std::vector<QString>&& items, size_t currentItemIndex);

    const std::vector<QString>& items() const;
    size_t currentItemIndex() const;
    QString currentItem() const;

private:
    friend class boost::serialization::access;

    std::vector<QString> _items;
    size_t _currentItemIndex = 0;

    template <class Archive>
    void save(Archive& ar, const unsigned int) const
    {
        // clang-format off
        ar & _items;
        ar & _currentItemIndex;
        // clang-format on
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int)
    {
#if BOOST_VERSION == 105800
        // WAR bug for std::vector of strings affecting Boost 1.58 only
        // https://bugs.launchpad.net/ubuntu/+source/boost/+bug/1583805
        _items.clear();
#endif
        // clang-format off
        ar & _items;
        ar & _currentItemIndex;
        // clang-format on
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#endif
