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

#ifndef WEBBROWSERCONTENT_H
#define WEBBROWSERCONTENT_H

#include "PixelStreamContent.h" // Base class
#include "WebbrowserHistory.h"  // Member

/**
 * The Webbrowser is a PixelStream extended with history navigation.
 */
class WebbrowserContent : public PixelStreamContent
{
    Q_OBJECT
    Q_PROPERTY(int page READ getPage NOTIFY pageChanged)
    Q_PROPERTY(int pageCount READ getPageCount NOTIFY pageCountChanged)

public:
    /**
     * Constructor.
     * @param uri The unique stream identifier.
     */
    explicit WebbrowserContent(const QString& uri);

    /** Get the content type **/
    CONTENT_TYPE getType() const final;

    /** Get the title for the web page. **/
    QString getTitle() const final;

    /** @return false, webbrowsers can adjust their aspect ratio. */
    bool hasFixedAspectRatio() const final;

    /** Get the index of the page navigation history. */
    int getPage() const;

    /** Get the number of pages in the navigation history. */
    int getPageCount() const;

    /** Get the url of the current webpage. */
    QString getUrl() const;

    /** Replace the navigation history with a single url. */
    void setUrl(const QString& url);

    /**
     * Parse data received from the deflect::Stream.
     *
     * @param data a data buffer created by serializeData()
     */
    void parseData(QByteArray data) final;

    /**
     * Serialize webbrowser data for sending through the deflect::Stream.
     *
     * @param history the navigation history
     * @param pageTitle the title of the current web page
     * @return a serialized data buffer that can be parsed by parseData()
     */
    static QByteArray serializeData(const WebbrowserHistory& history,
                                    const QString& pageTitle);

signals:
    /** @name QProperty notifiers */
    //@{
    void pageChanged();
    void pageCountChanged();
    //@}

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    WebbrowserContent();

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        // clang-format off
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PixelStreamContent);
        ar & _history;
        ar & _pageTitle;
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar, const unsigned int)
    {
        ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(PixelStreamContent);
        QString url;
        ar >> boost::serialization::make_nvp("url", url);
        setUrl(url);
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar, const unsigned int)
    {
        ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(PixelStreamContent);
        const auto url = getUrl();
        ar << boost::serialization::make_nvp("url", url);
    }

    /** Information received from the Webbrowser PixelStreamer. */
    WebbrowserHistory _history;
    QString _pageTitle;
};

DECLARE_SERIALIZE_FOR_XML(WebbrowserContent)

BOOST_CLASS_EXPORT_KEY(WebbrowserContent)

#endif
